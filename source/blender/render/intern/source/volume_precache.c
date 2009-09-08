/**
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): Matt Ebb.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "MEM_guardedalloc.h"

#include "BLI_blenlib.h"
#include "BLI_arithb.h"
#include "BLI_threads.h"
#include "BLI_voxel.h"

#include "PIL_time.h"

#include "RE_shader_ext.h"
#include "RE_raytrace.h"

#include "DNA_material_types.h"

#include "render_types.h"
#include "renderdatabase.h"
#include "volumetric.h"
#include "volume_precache.h"

#if defined( _MSC_VER ) && !defined( __cplusplus )
# define inline __inline
#endif // defined( _MSC_VER ) && !defined( __cplusplus )

#include "BKE_global.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* defined in pipeline.c, is hardcopy of active dynamic allocated Render */
/* only to be used here in this file, it's for speed */
extern struct Render R;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* *** utility code to set up an individual raytree for objectinstance, for checking inside/outside *** */

/* Recursive test for intersections, from a point inside the mesh, to outside
 * Number of intersections (depth) determine if a point is inside or outside the mesh */
int intersect_outside_volume(RayTree *tree, Isect *isect, float *offset, int limit, int depth)
{
	if (limit == 0) return depth;
	
	if (RE_ray_tree_intersect(tree, isect)) {
		float hitco[3];
		
		hitco[0] = isect->start[0] + isect->labda*isect->vec[0];
		hitco[1] = isect->start[1] + isect->labda*isect->vec[1];
		hitco[2] = isect->start[2] + isect->labda*isect->vec[2];
		VecAddf(isect->start, hitco, offset);

		return intersect_outside_volume(tree, isect, offset, limit-1, depth+1);
	} else {
		return depth;
	}
}

/* Uses ray tracing to check if a point is inside or outside an ObjectInstanceRen */
int point_inside_obi(RayTree *tree, ObjectInstanceRen *obi, float *co)
{
	float maxsize = RE_ray_tree_max_size(tree);
	Isect isect;
	float vec[3] = {0.0f,0.0f,1.0f};
	int final_depth=0, depth=0, limit=20;
	
	/* set up the isect */
	memset(&isect, 0, sizeof(isect));
	VECCOPY(isect.start, co);
	isect.end[0] = co[0] + vec[0] * maxsize;
	isect.end[1] = co[1] + vec[1] * maxsize;
	isect.end[2] = co[2] + vec[2] * maxsize;
	
	/* and give it a little offset to prevent self-intersections */
	VecMulf(vec, 1e-5);
	VecAddf(isect.start, isect.start, vec);
	
	isect.mode= RE_RAY_MIRROR;
	isect.face_last= NULL;
	isect.lay= -1;
	
	final_depth = intersect_outside_volume(tree, &isect, vec, limit, depth);
	
	/* even number of intersections: point is outside
	 * odd number: point is inside */
	if (final_depth % 2 == 0) return 0;
	else return 1;
}

static int inside_check_func(Isect *is, int ob, RayFace *face)
{
	return 1;
}
static void vlr_face_coords(RayFace *face, float **v1, float **v2, float **v3, float **v4)
{
	VlakRen *vlr= (VlakRen*)face;

	*v1 = (vlr->v1)? vlr->v1->co: NULL;
	*v2 = (vlr->v2)? vlr->v2->co: NULL;
	*v3 = (vlr->v3)? vlr->v3->co: NULL;
	*v4 = (vlr->v4)? vlr->v4->co: NULL;
}

RayTree *create_raytree_obi(ObjectInstanceRen *obi, float *bbmin, float *bbmax)
{
	int v;
	VlakRen *vlr= NULL;
	
	/* create empty raytree */
	RayTree *tree = RE_ray_tree_create(64, obi->obr->totvlak, bbmin, bbmax,
		vlr_face_coords, inside_check_func, NULL, NULL);
	
	/* fill it with faces */
	for(v=0; v<obi->obr->totvlak; v++) {
		if((v & 255)==0)
			vlr= obi->obr->vlaknodes[v>>8].vlak;
		else
			vlr++;
	
		RE_ray_tree_add_face(tree, 0, vlr);
	}
	
	RE_ray_tree_done(tree);
	
	return tree;
}

/* *** light cache filtering *** */

static float get_avg_surrounds(float *cache, int *res, int xx, int yy, int zz)
{
	int x, y, z, x_, y_, z_;
	int added=0;
	float tot=0.0f;
	
	for (z=-1; z <= 1; z++) {
		z_ = zz+z;
		if (z_ >= 0 && z_ <= res[2]-1) {
		
			for (y=-1; y <= 1; y++) {
				y_ = yy+y;
				if (y_ >= 0 && y_ <= res[1]-1) {
				
					for (x=-1; x <= 1; x++) {
						x_ = xx+x;
						if (x_ >= 0 && x_ <= res[0]-1) {
						
							if (cache[ V_I(x_, y_, z_, res) ] > 0.0f) {
								tot += cache[ V_I(x_, y_, z_, res) ];
								added++;
							}
							
						}
					}
				}
			}
		}
	}
	
	tot /= added;
	
	return ((added>0)?tot:0.0f);
}

/* function to filter the edges of the light cache, where there was no volume originally.
 * For each voxel which was originally external to the mesh, it finds the average values of
 * the surrounding internal voxels and sets the original external voxel to that average amount.
 * Works almost a bit like a 'dilate' filter */
static void lightcache_filter(VolumePrecache *vp)
{
	int x, y, z;

	for (z=0; z < vp->res[2]; z++) {
		for (y=0; y < vp->res[1]; y++) {
			for (x=0; x < vp->res[0]; x++) {
				/* trigger for outside mesh */
				if (vp->data_r[ V_I(x, y, z, vp->res) ] < -0.5f)
					vp->data_r[ V_I(x, y, z, vp->res) ] = get_avg_surrounds(vp->data_r, vp->res, x, y, z);
				if (vp->data_g[ V_I(x, y, z, vp->res) ] < -0.5f)
					vp->data_g[ V_I(x, y, z, vp->res) ] = get_avg_surrounds(vp->data_g, vp->res, x, y, z);
				if (vp->data_b[ V_I(x, y, z, vp->res) ] < -0.5f)
					vp->data_b[ V_I(x, y, z, vp->res) ] = get_avg_surrounds(vp->data_b, vp->res, x, y, z);
			}
		}
	}
}

static inline int ms_I(int x, int y, int z, int *n) //has a pad of 1 voxel surrounding the core for boundary simulation
{ 
	return z*(n[1]+2)*(n[0]+2) + y*(n[0]+2) + x;
}


/* *** multiple scattering approximation *** */

/* get the total amount of light energy in the light cache. used to normalise after multiple scattering */
static float total_ss_energy(VolumePrecache *vp)
{
	int x, y, z;
	int *res = vp->res;
	float energy=0.f;
	
	for (z=0; z < res[2]; z++) {
		for (y=0; y < res[1]; y++) {
			for (x=0; x < res[0]; x++) {
				if (vp->data_r[ V_I(x, y, z, res) ] > 0.f) energy += vp->data_r[ V_I(x, y, z, res) ];
				if (vp->data_g[ V_I(x, y, z, res) ] > 0.f) energy += vp->data_g[ V_I(x, y, z, res) ];
				if (vp->data_b[ V_I(x, y, z, res) ] > 0.f) energy += vp->data_b[ V_I(x, y, z, res) ];
			}
		}
	}
	
	return energy;
}

static float total_ms_energy(float *sr, float *sg, float *sb, int *res)
{
	int x, y, z, i;
	float energy=0.f;
	
	for (z=1;z<=res[2];z++) {
		for (y=1;y<=res[1];y++) {
			for (x=1;x<=res[0];x++) {
			
				i = ms_I(x,y,z,res);
				if (sr[i] > 0.f) energy += sr[i];
				if (sg[i] > 0.f) energy += sg[i];
				if (sb[i] > 0.f) energy += sb[i];
			}
		}
	}
	
	return energy;
}

static void ms_diffuse(int b, float* x0, float* x, float diff, int *n)
{
	int i, j, k, l;
	const float dt = VOL_MS_TIMESTEP;
	const float a = dt*diff*n[0]*n[1]*n[2];
	
	for (l=0; l<20; l++)
	{
		for (k=1; k<=n[2]; k++)
		{
			for (j=1; j<=n[1]; j++)
			{
				for (i=1; i<=n[0]; i++)
				{
					x[ms_I(i,j,k,n)] = (x0[ms_I(i,j,k,n)] + a*(
						 x[ms_I(i-1,j,k,n)]+x[ms_I(i+1,j,k,n)]+
						 x[ms_I(i,j-1,k,n)]+x[ms_I(i,j+1,k,n)]+
						 x[ms_I(i,j,k-1,n)]+x[ms_I(i,j,k+1,n)]))/(1+6*a);
				}
			}
		}
	}
}

void multiple_scattering_diffusion(Render *re, VolumePrecache *vp, Material *ma)
{
	const float diff = ma->vol.ms_diff * 0.001f; 	/* compensate for scaling for a nicer UI range */
	const float simframes = ma->vol.ms_steps;
	const int shade_type = ma->vol.shade_type;
	float fac = ma->vol.ms_intensity;
	
	int x, y, z, m;
	int *n = vp->res;
	const int size = (n[0]+2)*(n[1]+2)*(n[2]+2);
	double time, lasttime= PIL_check_seconds_timer();
	float total;
	float c=1.0f;
	int i;
	float origf;	/* factor for blending in original light cache */
	float energy_ss, energy_ms;

	float *sr0=(float *)MEM_callocN(size*sizeof(float), "temporary multiple scattering buffer");
	float *sr=(float *)MEM_callocN(size*sizeof(float), "temporary multiple scattering buffer");
	float *sg0=(float *)MEM_callocN(size*sizeof(float), "temporary multiple scattering buffer");
	float *sg=(float *)MEM_callocN(size*sizeof(float), "temporary multiple scattering buffer");
	float *sb0=(float *)MEM_callocN(size*sizeof(float), "temporary multiple scattering buffer");
	float *sb=(float *)MEM_callocN(size*sizeof(float), "temporary multiple scattering buffer");

	total = (float)(n[0]*n[1]*n[2]*simframes);
	
	energy_ss = total_ss_energy(vp);
	
	/* Scattering as diffusion pass */
	for (m=0; m<simframes; m++)
	{
		/* add sources */
		for (z=1; z<=n[2]; z++)
		{
			for (y=1; y<=n[1]; y++)
			{
				for (x=1; x<=n[0]; x++)
				{
					i = V_I((x-1), (y-1), (z-1), n);
					time= PIL_check_seconds_timer();
					c++;
										
					if (vp->data_r[i] > 0.f)
						sr[ms_I(x,y,z,n)] += vp->data_r[i];
					if (vp->data_g[i] > 0.f)
						sg[ms_I(x,y,z,n)] += vp->data_g[i];
					if (vp->data_b[i] > 0.f)
						sb[ms_I(x,y,z,n)] += vp->data_b[i];
					
					/* Displays progress every second */
					if(time-lasttime>1.0f) {
						char str[64];
						sprintf(str, "Simulating multiple scattering: %d%%", (int)
								(100.0f * (c / total)));
						re->i.infostr= str;
						re->stats_draw(re->sdh, &re->i);
						re->i.infostr= NULL;
						lasttime= time;
					}
				}
			}
		}
		SWAP(float *, sr, sr0);
		SWAP(float *, sg, sg0);
		SWAP(float *, sb, sb0);

		/* main diffusion simulation */
		ms_diffuse(0, sr0, sr, diff, n);
		ms_diffuse(0, sg0, sg, diff, n);
		ms_diffuse(0, sb0, sb, diff, n);
		
		if (re->test_break(re->tbh)) break;
	}
	
	/* normalisation factor to conserve energy */
	energy_ms = total_ms_energy(sr, sg, sb, n);
	fac *= (energy_ss / energy_ms);
	
	/* blend multiple scattering back in the light cache */
	if (shade_type == MA_VOL_SHADE_SINGLEPLUSMULTIPLE) {
		/* conserve energy - half single, half multiple */
		origf = 0.5f;
		fac *= 0.5f;
	} else {
		origf = 0.0f;
	}

	for (z=1;z<=n[2];z++)
	{
		for (y=1;y<=n[1];y++)
		{
			for (x=1;x<=n[0];x++)
			{
				int index=(x-1)*n[1]*n[2] + (y-1)*n[2] + z-1;
				vp->data_r[index] = origf * vp->data_r[index] + fac * sr[ms_I(x,y,z,n)];
				vp->data_g[index] = origf * vp->data_g[index] + fac * sg[ms_I(x,y,z,n)];
				vp->data_b[index] = origf * vp->data_b[index] + fac * sb[ms_I(x,y,z,n)];
			}
		}
	}

	MEM_freeN(sr0);
	MEM_freeN(sr);
	MEM_freeN(sg0);
	MEM_freeN(sg);
	MEM_freeN(sb0);
	MEM_freeN(sb);
}



#if 0 // debug stuff
static void *vol_precache_part_test(void *data)
{
	VolPrecachePart *pa = data;

	printf("part number: %d \n", pa->num);
	printf("done: %d \n", pa->done);
	printf("x min: %d   x max: %d \n", pa->minx, pa->maxx);
	printf("y min: %d   y max: %d \n", pa->miny, pa->maxy);
	printf("z min: %d   z max: %d \n", pa->minz, pa->maxz);

	return NULL;
}
#endif

/* Iterate over the 3d voxel grid, and fill the voxels with scattering information
 *
 * It's stored in memory as 3 big float grids next to each other, one for each RGB channel.
 * I'm guessing the memory alignment may work out better this way for the purposes
 * of doing linear interpolation, but I haven't actually tested this theory! :)
 */
static void *vol_precache_part(void *data)
{
	VolPrecachePart *pa =  (VolPrecachePart *)data;
	ObjectInstanceRen *obi = pa->obi;
	RayTree *tree = pa->tree;
	ShadeInput *shi = pa->shi;
	float density, scatter_col[3] = {0.f, 0.f, 0.f};
	float co[3];
	int x, y, z;
	const int res[3]= {pa->res[0], pa->res[1], pa->res[2]};
	const float stepsize = vol_get_stepsize(shi, STEPSIZE_VIEW);

	for (z= pa->minz; z < pa->maxz; z++) {
		co[2] = pa->bbmin[2] + (pa->voxel[2] * (z + 0.5f));
		
		for (y= pa->miny; y < pa->maxy; y++) {
			co[1] = pa->bbmin[1] + (pa->voxel[1] * (y + 0.5f));
			
			for (x=pa->minx; x < pa->maxx; x++) {
				co[0] = pa->bbmin[0] + (pa->voxel[0] * (x + 0.5f));
				
				// don't bother if the point is not inside the volume mesh
				if (!point_inside_obi(tree, obi, co)) {
					obi->volume_precache->data_r[ V_I(x, y, z, res) ] = -1.0f;
					obi->volume_precache->data_g[ V_I(x, y, z, res) ] = -1.0f;
					obi->volume_precache->data_b[ V_I(x, y, z, res) ] = -1.0f;
					continue;
				}
				
				VecCopyf(shi->view, co);
				Normalize(shi->view);
				density = vol_get_density(shi, co);
				vol_get_scattering(shi, scatter_col, co, stepsize, density);
			
				obi->volume_precache->data_r[ V_I(x, y, z, res) ] = scatter_col[0];
				obi->volume_precache->data_g[ V_I(x, y, z, res) ] = scatter_col[1];
				obi->volume_precache->data_b[ V_I(x, y, z, res) ] = scatter_col[2];
			}
		}
	}
	
	pa->done = 1;
	
	return 0;
}


static void precache_setup_shadeinput(Render *re, ObjectInstanceRen *obi, Material *ma, ShadeInput *shi)
{
	memset(shi, 0, sizeof(ShadeInput)); 
	shi->depth= 1;
	shi->mask= 1;
	shi->mat = ma;
	shi->vlr = NULL;
	memcpy(&shi->r, &shi->mat->r, 23*sizeof(float));	// note, keep this synced with render_types.h
	shi->har= shi->mat->har;
	shi->obi= obi;
	shi->obr= obi->obr;
	shi->lay = re->scene->lay;
}

static void precache_init_parts(Render *re, RayTree *tree, ShadeInput *shi, ObjectInstanceRen *obi, int totthread, int *parts)
{
	VolumePrecache *vp = obi->volume_precache;
	int i=0, x, y, z;
	float voxel[3];
	int sizex, sizey, sizez;
	float *bbmin=obi->obr->boundbox[0], *bbmax=obi->obr->boundbox[1];
	int *res;
	int minx, maxx;
	int miny, maxy;
	int minz, maxz;
	
	if (!vp) return;

	BLI_freelistN(&re->volume_precache_parts);
	
	/* currently we just subdivide the box, number of threads per side */
	parts[0] = parts[1] = parts[2] = totthread;
	res = vp->res;
	
	VecSubf(voxel, bbmax, bbmin);
	
	voxel[0] /= res[0];
	voxel[1] /= res[1];
	voxel[2] /= res[2];

	for (x=0; x < parts[0]; x++) {
		sizex = ceil(res[0] / (float)parts[0]);
		minx = x * sizex;
		maxx = minx + sizex;
		maxx = (maxx>res[0])?res[0]:maxx;
		
		for (y=0; y < parts[1]; y++) {
			sizey = ceil(res[1] / (float)parts[1]);
			miny = y * sizey;
			maxy = miny + sizey;
			maxy = (maxy>res[1])?res[1]:maxy;
			
			for (z=0; z < parts[2]; z++) {
				VolPrecachePart *pa= MEM_callocN(sizeof(VolPrecachePart), "new precache part");
				
				sizez = ceil(res[2] / (float)parts[2]);
				minz = z * sizez;
				maxz = minz + sizez;
				maxz = (maxz>res[2])?res[2]:maxz;
						
				pa->done = 0;
				pa->working = 0;
				
				pa->num = i;
				pa->tree = tree;
				pa->shi = shi;
				pa->obi = obi;
				VECCOPY(pa->bbmin, bbmin);
				VECCOPY(pa->voxel, voxel);
				VECCOPY(pa->res, res);
				
				pa->minx = minx; pa->maxx = maxx;
				pa->miny = miny; pa->maxy = maxy;
				pa->minz = minz; pa->maxz = maxz;
				
				
				BLI_addtail(&re->volume_precache_parts, pa);
				
				i++;
			}
		}
	}
}

static VolPrecachePart *precache_get_new_part(Render *re)
{
	VolPrecachePart *pa, *nextpa=NULL;
	
	for (pa = re->volume_precache_parts.first; pa; pa=pa->next)
	{
		if (pa->done==0 && pa->working==0) {
			nextpa = pa;
			break;
		}
	}

	return nextpa;
}

static int precache_resolution(VolumePrecache *vp, float *bbmin, float *bbmax, int res)
{
	float dim[3], div;
	
	VecSubf(dim, bbmax, bbmin);
	
	div = MAX3(dim[0], dim[1], dim[2]);
	dim[0] /= div;
	dim[1] /= div;
	dim[2] /= div;
	
	vp->res[0] = dim[0] * (float)res;
	vp->res[1] = dim[1] * (float)res;
	vp->res[2] = dim[2] * (float)res;
	
	if ((vp->res[0] < 1) || (vp->res[1] < 1) || (vp->res[2] < 1))
		return 0;
	
	return 1;
}

/* Precache a volume into a 3D voxel grid.
 * The voxel grid is stored in the ObjectInstanceRen, 
 * in camera space, aligned with the ObjectRen's bounding box.
 * Resolution is defined by the user.
 */
void vol_precache_objectinstance_threads(Render *re, ObjectInstanceRen *obi, Material *ma)
{
	VolumePrecache *vp;
	VolPrecachePart *nextpa, *pa;
	RayTree *tree;
	ShadeInput shi;
	ListBase threads;
	float *bbmin=obi->obr->boundbox[0], *bbmax=obi->obr->boundbox[1];
	int parts[3], totparts;
	
	int caching=1, counter=0;
	int totthread = re->r.threads;
	
	double time, lasttime= PIL_check_seconds_timer();
	
	R = *re;

	/* create a raytree with just the faces of the instanced ObjectRen, 
	 * used for checking if the cached point is inside or outside. */
	tree = create_raytree_obi(obi, bbmin, bbmax);
	if (!tree) return;

	vp = MEM_callocN(sizeof(VolumePrecache), "volume light cache");
	
	if (!precache_resolution(vp, bbmin, bbmax, ma->vol.precache_resolution)) {
		MEM_freeN(vp);
		vp = NULL;
		return;
	}

	vp->data_r = MEM_callocN(sizeof(float)*vp->res[0]*vp->res[1]*vp->res[2], "volume light cache data red channel");
	vp->data_g = MEM_callocN(sizeof(float)*vp->res[0]*vp->res[1]*vp->res[2], "volume light cache data green channel");
	vp->data_b = MEM_callocN(sizeof(float)*vp->res[0]*vp->res[1]*vp->res[2], "volume light cache data blue channel");
	obi->volume_precache = vp;

	/* Need a shadeinput to calculate scattering */
	precache_setup_shadeinput(re, obi, ma, &shi);
	
	precache_init_parts(re, tree, &shi, obi, totthread, parts);
	totparts = parts[0] * parts[1] * parts[2];
	
	BLI_init_threads(&threads, vol_precache_part, totthread);
	
	while(caching) {

		if(BLI_available_threads(&threads) && !(re->test_break(re->tbh))) {
			nextpa = precache_get_new_part(re);
			if (nextpa) {
				nextpa->working = 1;
				BLI_insert_thread(&threads, nextpa);
			}
		}
		else PIL_sleep_ms(50);

		caching=0;
		counter=0;
		for(pa= re->volume_precache_parts.first; pa; pa= pa->next) {
			
			if(pa->done) {
				counter++;
				BLI_remove_thread(&threads, pa);
			} else
				caching = 1;
		}
		
		if (re->test_break(re->tbh) && BLI_available_threads(&threads)==totthread)
			caching=0;
		
		time= PIL_check_seconds_timer();
		if(time-lasttime>1.0f) {
			char str[64];
			sprintf(str, "Precaching volume: %d%%", (int)(100.0f * ((float)counter / (float)totparts)));
			re->i.infostr= str;
			re->stats_draw(re->sdh, &re->i);
			re->i.infostr= NULL;
			lasttime= time;
		}
	}
	
	BLI_end_threads(&threads);
	BLI_freelistN(&re->volume_precache_parts);
	
	if(tree) {
		RE_ray_tree_free(tree);
		tree= NULL;
	}
	
	lightcache_filter(obi->volume_precache);
	
	if (ELEM(ma->vol.shade_type, MA_VOL_SHADE_MULTIPLE, MA_VOL_SHADE_SINGLEPLUSMULTIPLE))
	{
		multiple_scattering_diffusion(re, vp, ma);
	}
}

static int using_lightcache(Material *ma)
{
	return (((ma->vol.shadeflag & MA_VOL_PRECACHESHADING) && (ma->vol.shade_type == MA_VOL_SHADE_SINGLE))
		|| (ELEM(ma->vol.shade_type, MA_VOL_SHADE_MULTIPLE, MA_VOL_SHADE_SINGLEPLUSMULTIPLE)));
}

/* loop through all objects (and their associated materials)
 * marked for pre-caching in convertblender.c, and pre-cache them */
void volume_precache(Render *re)
{
	ObjectInstanceRen *obi;
	VolumeOb *vo;

	for(vo= re->volumes.first; vo; vo= vo->next) {
		if (using_lightcache(vo->ma)) {
			for(obi= re->instancetable.first; obi; obi= obi->next) {
				if (obi->obr == vo->obr) {
					vol_precache_objectinstance_threads(re, obi, vo->ma);
				}
			}
		}
	}
	
	re->i.infostr= NULL;
	re->stats_draw(re->sdh, &re->i);
}

void free_volume_precache(Render *re)
{
	ObjectInstanceRen *obi;
	
	for(obi= re->instancetable.first; obi; obi= obi->next) {
		if (obi->volume_precache != NULL) {
			MEM_freeN(obi->volume_precache->data_r);
			MEM_freeN(obi->volume_precache->data_g);
			MEM_freeN(obi->volume_precache->data_b);
			MEM_freeN(obi->volume_precache);
			obi->volume_precache = NULL;
		}
	}
	
	BLI_freelistN(&re->volumes);
}

int point_inside_volume_objectinstance(ObjectInstanceRen *obi, float *co)
{
	RayTree *tree;
	int inside=0;
	
	tree = create_raytree_obi(obi, obi->obr->boundbox[0], obi->obr->boundbox[1]);
	if (!tree) return 0;
	
	inside = point_inside_obi(tree, obi, co);
	
	RE_ray_tree_free(tree);
	tree= NULL;
	
	return inside;
}

