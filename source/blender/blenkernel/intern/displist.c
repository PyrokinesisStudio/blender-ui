/*  displist.c
 * 
 * 
 * $Id$
 *
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. The Blender
 * Foundation also sells licenses for use in proprietary software under
 * the Blender License.  See http://www.blender.org/BL/ for information
 * about this.
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
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "MEM_guardedalloc.h"

#include "IMB_imbuf_types.h"

#include "DNA_texture_types.h"
#include "DNA_meta_types.h"
#include "DNA_curve_types.h"
#include "DNA_effect_types.h"
#include "DNA_listBase.h"
#include "DNA_lamp_types.h"
#include "DNA_object_types.h"
#include "DNA_object_force.h"
#include "DNA_mesh_types.h"
#include "DNA_meshdata_types.h"
#include "DNA_modifier_types.h"
#include "DNA_scene_types.h"
#include "DNA_image_types.h"
#include "DNA_material_types.h"
#include "DNA_view3d_types.h"
#include "DNA_lattice_types.h"
#include "DNA_key_types.h"

#include "BLI_blenlib.h"
#include "BLI_arithb.h"
#include "BLI_editVert.h"
#include "BLI_edgehash.h"

#include "BKE_bad_level_calls.h"
#include "BKE_utildefines.h"
#include "BKE_global.h"
#include "BKE_displist.h"
#include "BKE_deform.h"
#include "BKE_DerivedMesh.h"
#include "BKE_object.h"
#include "BKE_world.h"
#include "BKE_mesh.h"
#include "BKE_effect.h"
#include "BKE_mball.h"
#include "BKE_material.h"
#include "BKE_curve.h"
#include "BKE_key.h"
#include "BKE_anim.h"
#include "BKE_screen.h"
#include "BKE_texture.h"
#include "BKE_library.h"
#include "BKE_font.h"
#include "BKE_lattice.h"
#include "BKE_scene.h"
#include "BKE_subsurf.h"
#include "BKE_modifier.h"

#include "nla.h" /* For __NLA: Please do not remove yet */


static void boundbox_displist(Object *ob);

void displistmesh_free(DispListMesh *dlm) 
{
	// also check on mvert and mface, can be NULL after decimator (ton)
	if (!dlm->dontFreeVerts && dlm->mvert) MEM_freeN(dlm->mvert);
	if (!dlm->dontFreeNors && dlm->nors) MEM_freeN(dlm->nors);
	if (!dlm->dontFreeOther) {
		if (dlm->medge) MEM_freeN(dlm->medge);
		if (dlm->mface) MEM_freeN(dlm->mface);
		if (dlm->mcol) MEM_freeN(dlm->mcol);
		if (dlm->tface) MEM_freeN(dlm->tface);
	}
	MEM_freeN(dlm);
}

DispListMesh *displistmesh_copy(DispListMesh *odlm) 
{
	DispListMesh *ndlm= MEM_dupallocN(odlm);
	ndlm->mvert= MEM_dupallocN(odlm->mvert);
	if (odlm->medge) ndlm->medge= MEM_dupallocN(odlm->medge);
	ndlm->mface= MEM_dupallocN(odlm->mface);
	if (odlm->nors) ndlm->nors = MEM_dupallocN(odlm->nors);
	if (odlm->mcol) ndlm->mcol= MEM_dupallocN(odlm->mcol);
	if (odlm->tface) ndlm->tface= MEM_dupallocN(odlm->tface);

	return ndlm;
}

DispListMesh *displistmesh_copyShared(DispListMesh *odlm) 
{
	DispListMesh *ndlm= MEM_dupallocN(odlm);
	ndlm->dontFreeNors = ndlm->dontFreeOther = ndlm->dontFreeVerts = 1;
	
	return ndlm;
}

void displistmesh_to_mesh(DispListMesh *dlm, Mesh *me) 
{
		/* We assume, rather courageously, that any
		 * shared data came from the mesh itself and so
		 * we can ignore the dlm->dontFreeOther flag.
		 */

	if (me->mvert && dlm->mvert!=me->mvert) MEM_freeN(me->mvert);
	if (me->mface && dlm->mface!=me->mface) MEM_freeN(me->mface);
	if (me->tface && dlm->tface!=me->tface) MEM_freeN(me->tface);
	if (me->mcol && dlm->mcol!=me->mcol) MEM_freeN(me->mcol);
	if (me->medge && dlm->medge!=me->medge) MEM_freeN(me->medge);

	me->tface = NULL;
	me->mcol = NULL;
	me->medge = NULL;

	if (dlm->totvert!=me->totvert) {
		if (me->msticky) MEM_freeN(me->msticky);
		me->msticky = NULL;

		if (me->dvert) free_dverts(me->dvert, me->totvert);
		me->dvert = NULL;

		if(me->key) me->key->id.us--;
		me->key = NULL;
	}

	me->totface= dlm->totface;
	me->totvert= dlm->totvert;
	me->totedge= 0;

	me->mvert= dlm->mvert;
	me->mface= dlm->mface;
	if (dlm->tface)
		me->tface= dlm->tface;
	if (dlm->mcol)
		me->mcol= dlm->mcol;

	if(dlm->medge) {
		me->totedge= dlm->totedge;
		me->medge= dlm->medge;
	}

	if (dlm->nors && !dlm->dontFreeNors) MEM_freeN(dlm->nors);

	MEM_freeN(dlm);
}

void free_disp_elem(DispList *dl)
{
	if(dl) {
		if(dl->verts) MEM_freeN(dl->verts);
		if(dl->nors) MEM_freeN(dl->nors);
		if(dl->index) MEM_freeN(dl->index);
		if(dl->col1) MEM_freeN(dl->col1);
		if(dl->col2) MEM_freeN(dl->col2);
		if(dl->bevelSplitFlag) MEM_freeN(dl->bevelSplitFlag);
		MEM_freeN(dl);
	}
}

void freedisplist(ListBase *lb)
{
	DispList *dl;

	dl= lb->first;
	while(dl) {
		BLI_remlink(lb, dl);
		free_disp_elem(dl);
		dl= lb->first;
	}
}

DispList *find_displist_create(ListBase *lb, int type)
{
	DispList *dl;
	
	dl= lb->first;
	while(dl) {
		if(dl->type==type) return dl;
		dl= dl->next;
	}

	dl= MEM_callocN(sizeof(DispList), "find_disp");
	dl->type= type;
	BLI_addtail(lb, dl);

	return dl;
}

DispList *find_displist(ListBase *lb, int type)
{
	DispList *dl;
	
	dl= lb->first;
	while(dl) {
		if(dl->type==type) return dl;
		dl= dl->next;
	}

	return 0;
}

int displist_has_faces(ListBase *lb)
{
	DispList *dl;
	
	dl= lb->first;
	while(dl) {
		if ELEM3(dl->type, DL_INDEX3, DL_INDEX4, DL_SURF)
			return 1;
		dl= dl->next;
	}
	return 0;
}

void copy_displist(ListBase *lbn, ListBase *lb)
{
	DispList *dln, *dl;
	
	lbn->first= lbn->last= 0;
	
	dl= lb->first;
	while(dl) {
		
		dln= MEM_dupallocN(dl);
		BLI_addtail(lbn, dln);
		dln->verts= MEM_dupallocN(dl->verts);
		dln->nors= MEM_dupallocN(dl->nors);
		dln->index= MEM_dupallocN(dl->index);
		dln->col1= MEM_dupallocN(dl->col1);
		dln->col2= MEM_dupallocN(dl->col2);
		
		dl= dl->next;
	}
}

void initfastshade(void)
{
}


void freefastshade()
{
}


static void fastshade(float *co, float *nor, float *orco, Material *ma, char *col1, char *col2, char *vertcol)
{
}

void addnormalsDispList(Object *ob, ListBase *lb)
{
	DispList *dl = NULL;
	float *vdata, *ndata, nor[3];
	float *v1, *v2, *v3, *v4;
	float *n1, *n2, *n3, *n4;
	int a, b, p1, p2, p3, p4;


	dl= lb->first;
	
	while(dl) {
		if(dl->type==DL_INDEX3) {
			if(dl->nors==0) {
				dl->nors= MEM_callocN(sizeof(float)*3, "dlnors");
				if(dl->verts[2]<0.0) dl->nors[2]= -1.0;
				else dl->nors[2]= 1.0;
			}
		}
		else if(dl->type==DL_SURF) {
			if(dl->nors==0) {
				dl->nors= MEM_callocN(sizeof(float)*3*dl->nr*dl->parts, "dlnors");
				
				vdata= dl->verts;
				ndata= dl->nors;
				
				for(a=0; a<dl->parts; a++) {
	
					DL_SURFINDEX(dl->flag & DL_CYCL_U, dl->flag & DL_CYCL_V, dl->nr, dl->parts);
	
					v1= vdata+ 3*p1; 
					n1= ndata+ 3*p1;
					v2= vdata+ 3*p2; 
					n2= ndata+ 3*p2;
					v3= vdata+ 3*p3; 
					n3= ndata+ 3*p3;
					v4= vdata+ 3*p4; 
					n4= ndata+ 3*p4;
					
					for(; b<dl->nr; b++) {
	
						CalcNormFloat4(v1, v3, v4, v2, nor);
	
						VecAddf(n1, n1, nor);
						VecAddf(n2, n2, nor);
						VecAddf(n3, n3, nor);
						VecAddf(n4, n4, nor);
	
						v2= v1; v1+= 3;
						v4= v3; v3+= 3;
						n2= n1; n1+= 3;
						n4= n3; n3+= 3;
					}
				}
				a= dl->parts*dl->nr;
				v1= ndata;
				while(a--) {
					Normalise(v1);
					v1+= 3;
				}
			}
		}
		dl= dl->next;
	}
}

static void init_fastshade_for_ob(Object *ob, int *need_orco_r, float mat[4][4], float imat[3][3])
{
}

void mesh_create_shadedColors(Object *ob, int onlyForMesh, unsigned int **col1_r, unsigned int **col2_r)
{
	Mesh *me= ob->data;
	int dmNeedsFree;
	DerivedMesh *dm;
	DispListMesh *dlm;
	unsigned int *col1, *col2;
	float *orco, *vnors, imat[3][3], mat[4][4], vec[3];
	int a, i, need_orco;

	init_fastshade_for_ob(ob, &need_orco, mat, imat);

	if (need_orco) {
		orco = mesh_create_orco(ob);
	} else {
		orco = NULL;
	}

	if (onlyForMesh) {
		dm = mesh_get_derived_deform(ob, &dmNeedsFree);
	} else {
		dm = mesh_get_derived_final(ob, &dmNeedsFree);
	}
	dlm= dm->convertToDispListMesh(dm, 1);

	col1 = MEM_mallocN(sizeof(*col1)*dlm->totface*4, "col1");
	if (col2_r && (me->flag & ME_TWOSIDED)) {
		col2 = MEM_mallocN(sizeof(*col2)*dlm->totface*4, "col1");
	} else {
		col2 = NULL;
	}
	
	*col1_r = col1;
	if (col2_r) *col2_r = col2;

		/* vertexnormals */
	vnors= MEM_mallocN(dlm->totvert*3*sizeof(float), "vnors disp");
	for (a=0; a<dlm->totvert; a++) {
		MVert *mv = &dlm->mvert[a];
		float *vn= &vnors[a*3];
		float xn= mv->no[0]; 
		float yn= mv->no[1]; 
		float zn= mv->no[2];
		
			/* transpose ! */
		vn[0]= imat[0][0]*xn+imat[0][1]*yn+imat[0][2]*zn;
		vn[1]= imat[1][0]*xn+imat[1][1]*yn+imat[1][2]*zn;
		vn[2]= imat[2][0]*xn+imat[2][1]*yn+imat[2][2]*zn;
		Normalise(vn);
	}		

	for (i=0; i<dlm->totface; i++) {
		MFace *mf= &dlm->mface[i];
		int j, vidx[4], nverts= mf->v4?4:3;
		unsigned char *col1base= (unsigned char*) &col1[i*4];
		unsigned char *col2base= (unsigned char*) (col2?&col2[i*4]:NULL);
		unsigned char *mcolbase;
		Material *ma= give_current_material(ob, mf->mat_nr+1);
		float nor[3], n1[3];
		
		if(ma==0) ma= &defmaterial;
		
		if (dlm->tface) {
			mcolbase = (unsigned char*) dlm->tface[i].col;
		} else if (dlm->mcol) {
			mcolbase = (unsigned char*) &dlm->mcol[i*4];
		} else {
			mcolbase = NULL;
		}

		vidx[0]= mf->v1;
		vidx[1]= mf->v2;
		vidx[2]= mf->v3;
		vidx[3]= mf->v4;

		if (dlm->nors) {
			VECCOPY(nor, &dlm->nors[i*3]);
		} else {
			if (mf->v4)
				CalcNormFloat4(dlm->mvert[mf->v1].co, dlm->mvert[mf->v2].co, dlm->mvert[mf->v3].co, dlm->mvert[mf->v4].co, nor);
			else
				CalcNormFloat(dlm->mvert[mf->v1].co, dlm->mvert[mf->v2].co, dlm->mvert[mf->v3].co, nor);
		}

		n1[0]= imat[0][0]*nor[0]+imat[0][1]*nor[1]+imat[0][2]*nor[2];
		n1[1]= imat[1][0]*nor[0]+imat[1][1]*nor[1]+imat[1][2]*nor[2];
		n1[2]= imat[2][0]*nor[0]+imat[2][1]*nor[1]+imat[2][2]*nor[2];
		Normalise(n1);

		for (j=0; j<nverts; j++) {
			MVert *mv= &dlm->mvert[vidx[j]];
			unsigned char *col1= &col1base[j*4];
			unsigned char *col2= col2base?&col2base[j*4]:NULL;
			unsigned char *mcol= mcolbase?&mcolbase[j*4]:NULL;
			float *vn = (mf->flag & ME_SMOOTH)?&vnors[3*vidx[j]]:n1;

			VECCOPY(vec, mv->co);
			Mat4MulVecfl(mat, vec);
			fastshade(vec, vn, orco?&orco[vidx[j]*3]:mv->co, ma, col1, col2, mcol);
		}
	} 
	MEM_freeN(vnors);
	displistmesh_free(dlm);

	if (orco) {
		MEM_freeN(orco);
	}

	if (dmNeedsFree) dm->release(dm);

}

/* has base pointer, to check for layer */
void shadeDispList(Base *base)
{
	Object *ob= base->object;
	DispList *dl, *dlob;
	Material *ma = NULL;
	Curve *cu;
	float imat[3][3], mat[4][4], vec[3];
	float *fp, *nor, n1[3];
	unsigned int *col1;
	int a;

	dl = find_displist(&ob->disp, DL_VERTCOL);
	if (dl) {
		BLI_remlink(&ob->disp, dl);
		free_disp_elem(dl);
	}

	if(ob->type==OB_MESH) {
		dl= MEM_callocN(sizeof(DispList), "displistshade");
		BLI_addtail(&ob->disp, dl);
		dl->type= DL_VERTCOL;

		mesh_create_shadedColors(ob, 0, &dl->col1, &dl->col2);

		return;
	}

	init_fastshade_for_ob(ob, NULL, mat, imat);
	
	if ELEM3(ob->type, OB_CURVE, OB_SURF, OB_FONT) {
	
		/* now we need the normals */
		cu= ob->data;
		dl= cu->disp.first;
		
		while(dl) {
			dlob= MEM_callocN(sizeof(DispList), "displistshade");
			BLI_addtail(&ob->disp, dlob);
			dlob->type= DL_VERTCOL;
			dlob->parts= dl->parts;
			dlob->nr= dl->nr;
			
			if(dl->type==DL_INDEX3) {
				col1= dlob->col1= MEM_mallocN(sizeof(int)*dl->nr, "col1");
			}
			else {
				col1= dlob->col1= MEM_mallocN(sizeof(int)*dl->parts*dl->nr, "col1");
			}
			
		
			ma= give_current_material(ob, dl->col+1);
			if(ma==0) ma= &defmaterial;

			if(dl->type==DL_INDEX3) {
				if(dl->nors) {
					/* there's just one normal */
					n1[0]= imat[0][0]*dl->nors[0]+imat[0][1]*dl->nors[1]+imat[0][2]*dl->nors[2];
					n1[1]= imat[1][0]*dl->nors[0]+imat[1][1]*dl->nors[1]+imat[1][2]*dl->nors[2];
					n1[2]= imat[2][0]*dl->nors[0]+imat[2][1]*dl->nors[1]+imat[2][2]*dl->nors[2];
					Normalise(n1);
					
					fp= dl->verts;
					
					a= dl->nr;		
					while(a--) {
						VECCOPY(vec, fp);
						Mat4MulVecfl(mat, vec);
						
						fastshade(vec, n1, fp, ma, (char *)col1, 0, 0);
						
						fp+= 3; col1++;
					}
				}
			}
			else if(dl->type==DL_SURF) {
				if(dl->nors) {
					a= dl->nr*dl->parts;
					fp= dl->verts;
					nor= dl->nors;
					
					while(a--) {
						VECCOPY(vec, fp);
						Mat4MulVecfl(mat, vec);
						
						n1[0]= imat[0][0]*nor[0]+imat[0][1]*nor[1]+imat[0][2]*nor[2];
						n1[1]= imat[1][0]*nor[0]+imat[1][1]*nor[1]+imat[1][2]*nor[2];
						n1[2]= imat[2][0]*nor[0]+imat[2][1]*nor[1]+imat[2][2]*nor[2];
						Normalise(n1);
			
						fastshade(vec, n1, fp, ma, (char *)col1, 0, 0);
						
						fp+= 3; nor+= 3; col1++;
					}
				}
			}
			dl= dl->next;
		}
	}
	else if(ob->type==OB_MBALL) {
		/* there are normals already */
		dl= ob->disp.first;
		
		while(dl) {
			
			if(dl->type==DL_INDEX4) {
				if(dl->nors) {
					
					if(dl->col1) MEM_freeN(dl->col1);
					col1= dl->col1= MEM_mallocN(sizeof(int)*dl->nr, "col1");
			
					ma= give_current_material(ob, dl->col+1);
					if(ma==0) ma= &defmaterial;
	
					fp= dl->verts;
					nor= dl->nors;
					
					a= dl->nr;		
					while(a--) {
						VECCOPY(vec, fp);
						Mat4MulVecfl(mat, vec);
						
						/* transpose ! */
						n1[0]= imat[0][0]*nor[0]+imat[0][1]*nor[1]+imat[0][2]*nor[2];
						n1[1]= imat[1][0]*nor[0]+imat[1][1]*nor[1]+imat[1][2]*nor[2];
						n1[2]= imat[2][0]*nor[0]+imat[2][1]*nor[1]+imat[2][2]*nor[2];
						Normalise(n1);
					
						fastshade(vec, n1, fp, ma, (char *)col1, 0, 0);
						
						fp+= 3; col1++; nor+= 3;
					}
				}
			}
			dl= dl->next;
		}
	}
}

void reshadeall_displist(void)
{
	Base *base;
	Object *ob;
	
	freefastshade();
	
	for(base= G.scene->base.first; base; base= base->next) {
		ob= base->object;
		freedisplist(&ob->disp);
		if(base->lay & G.scene->lay) {
			/* Metaballs have standard displist at the Object */
			if(ob->type==OB_MBALL) shadeDispList(base);
		}
	}
}

void count_displist(ListBase *lb, int *totvert, int *totface)
{
	DispList *dl;
	
	dl= lb->first;
	while(dl) {
		
		switch(dl->type) {
		case DL_SURF:
			*totvert+= dl->nr*dl->parts;
			*totface+= (dl->nr-1)*(dl->parts-1);
			break;
		case DL_INDEX3:
		case DL_INDEX4:
			*totvert+= dl->nr;
			*totface+= dl->parts;
			break;
		case DL_POLY:
		case DL_SEGM:
			*totvert+= dl->nr*dl->parts;
		}
		
		dl= dl->next;
	}
}

static void curve_to_displist(Curve *cu, ListBase *nubase, ListBase *dispbase)
{
	Nurb *nu;
	DispList *dl;
	BezTriple *bezt, *prevbezt;
	BPoint *bp;
	float *data, *v1, *v2;
	int a, len;
	
	nu= nubase->first;
	while(nu) {
		if(nu->hide==0) {
			if((nu->type & 7)==CU_BEZIER) {
				
				/* count */
				len= 0;
				a= nu->pntsu-1;
				if(nu->flagu & 1) a++;

				prevbezt= nu->bezt;
				bezt= prevbezt+1;
				while(a--) {
					if(a==0 && (nu->flagu & 1)) bezt= nu->bezt;
					
					if(prevbezt->h2==HD_VECT && bezt->h1==HD_VECT) len++;
					else len+= nu->resolu;
					
					if(a==0 && (nu->flagu & 1)==0) len++;
					
					prevbezt= bezt;
					bezt++;
				}
				
				dl= MEM_callocN(sizeof(DispList), "makeDispListbez");
				/* len+1 because of 'forward_diff_bezier' function */
				dl->verts= MEM_callocN( (len+1)*3*sizeof(float), "dlverts");
				BLI_addtail(dispbase, dl);
				dl->parts= 1;
				dl->nr= len;
				dl->col= nu->mat_nr;
				dl->charidx= nu->charidx;

				data= dl->verts;

				if(nu->flagu & 1) {
					dl->type= DL_POLY;
					a= nu->pntsu;
				}
				else {
					dl->type= DL_SEGM;
					a= nu->pntsu-1;
				}
				
				prevbezt= nu->bezt;
				bezt= prevbezt+1;
				
				while(a--) {
					if(a==0 && dl->type== DL_POLY) bezt= nu->bezt;
					
					if(prevbezt->h2==HD_VECT && bezt->h1==HD_VECT) {
						VECCOPY(data, prevbezt->vec[1]);
						data+= 3;
					}
					else {
						v1= prevbezt->vec[1];
						v2= bezt->vec[0];
						forward_diff_bezier(v1[0], v1[3], v2[0], v2[3], data, nu->resolu, 3);
						forward_diff_bezier(v1[1], v1[4], v2[1], v2[4], data+1, nu->resolu, 3);
						forward_diff_bezier(v1[2], v1[5], v2[2], v2[5], data+2, nu->resolu, 3);
						data+= 3*nu->resolu;
					}
					
					if(a==0 && dl->type==DL_SEGM) {
						VECCOPY(data, bezt->vec[1]);
					}
					
					prevbezt= bezt;
					bezt++;
				}
			}
			else if((nu->type & 7)==CU_NURBS) {
				len= nu->pntsu*nu->resolu;
				dl= MEM_callocN(sizeof(DispList), "makeDispListsurf");
				dl->verts= MEM_callocN(len*3*sizeof(float), "dlverts");
				BLI_addtail(dispbase, dl);
				dl->parts= 1;
				dl->nr= len;
				dl->col= nu->mat_nr;

				data= dl->verts;
				if(nu->flagu & 1) dl->type= DL_POLY;
				else dl->type= DL_SEGM;
				makeNurbcurve(nu, data, 3);
			}
			else if((nu->type & 7)==CU_POLY) {
				len= nu->pntsu;
				dl= MEM_callocN(sizeof(DispList), "makeDispListpoly");
				dl->verts= MEM_callocN(len*3*sizeof(float), "dlverts");
				BLI_addtail(dispbase, dl);
				dl->parts= 1;
				dl->nr= len;
				dl->col= nu->mat_nr;
				dl->charidx = nu->charidx;

				data= dl->verts;
				if(nu->flagu & 1) dl->type= DL_POLY;
				else dl->type= DL_SEGM;
				
				a= len;
				bp= nu->bp;
				while(a--) {
					VECCOPY(data, bp->vec);
					bp++;
					data+= 3;
				}
			}
		}
		nu= nu->next;
	}
}


void filldisplist(ListBase *dispbase, ListBase *to)
{
	EditVert *eve, *v1, *vlast;
	EditFace *efa;
	DispList *dlnew=0, *dl;
	float *f1;
	int colnr=0, charidx=0, cont=1, tot, a, *index;
	long totvert;
	
	if(dispbase==0) return;
	if(dispbase->first==0) return;

	while(cont) {
		cont= 0;
		totvert=0;
		
		dl= dispbase->first;
		while(dl) {
	
			if(dl->type==DL_POLY) {
				if(charidx<dl->charidx) cont= 1;
				else if(charidx==dl->charidx) {
			
					colnr= dl->col;
					charidx= dl->charidx;
		
					/* make editverts and edges */
					f1= dl->verts;
					a= dl->nr;
					eve= v1= 0;
					
					while(a--) {
						vlast= eve;
						
						eve= BLI_addfillvert(f1);
						totvert++;
						
						if(vlast==0) v1= eve;
						else {
							BLI_addfilledge(vlast, eve);
						}
						f1+=3;
					}
				
					if(eve!=0 && v1!=0) {
						BLI_addfilledge(eve, v1);
					}
				}
			}
			dl= dl->next;
		}
		
		if(totvert && BLI_edgefill(0, (G.obedit && G.obedit->actcol)?(G.obedit->actcol-1):0)) {

			/* count faces  */
			tot= 0;
			efa= fillfacebase.first;
			while(efa) {
				tot++;
				efa= efa->next;
			}

			if(tot) {
				dlnew= MEM_callocN(sizeof(DispList), "filldisplist");
				dlnew->type= DL_INDEX3;
				dlnew->col= colnr;
				dlnew->nr= totvert;
				dlnew->parts= tot;

				dlnew->index= MEM_mallocN(tot*3*sizeof(int), "dlindex");
				dlnew->verts= MEM_mallocN(totvert*3*sizeof(float), "dlverts");
				
				/* vert data */
				f1= dlnew->verts;
				totvert= 0;
				eve= fillvertbase.first;
				while(eve) {
					VECCOPY(f1, eve->co);
					f1+= 3;
	
					/* index number */
					eve->tmp.l = totvert;
					totvert++;
					
					eve= eve->next;
				}
				
				/* index data */
				efa= fillfacebase.first;
				index= dlnew->index;
				while(efa) {
					index[0]= (long)efa->v1->tmp.l;
					index[1]= (long)efa->v2->tmp.l;
					index[2]= (long)efa->v3->tmp.l;
					
					index+= 3;
					efa= efa->next;
				}
			}

			BLI_addhead(to, dlnew);
			
		}
		BLI_end_edgefill();

		charidx++;
	}
	
	/* do not free polys, needed for wireframe display */
	
}

static void bevels_to_filledpoly(Curve *cu, ListBase *dispbase)
{
	ListBase front, back;
	DispList *dl, *dlnew;
	float *fp, *fp1;
	int a, dpoly;
	
	front.first= front.last= back.first= back.last= 0;
	
	dl= dispbase->first;
	while(dl) {
		if(dl->type==DL_SURF) {
			if( (dl->flag & DL_CYCL_V) && (dl->flag & DL_CYCL_U)==0 ) {
				if( (cu->flag & CU_BACK) && (dl->flag & DL_BACK_CURVE) ) {
					dlnew= MEM_callocN(sizeof(DispList), "filldisp");
					BLI_addtail(&front, dlnew);
					dlnew->verts= fp1= MEM_mallocN(sizeof(float)*3*dl->parts, "filldisp1");
					dlnew->nr= dl->parts;
					dlnew->parts= 1;
					dlnew->type= DL_POLY;
					dlnew->col= dl->col;
					
					fp= dl->verts;
					dpoly= 3*dl->nr;
					
					a= dl->parts;
					while(a--) {
						VECCOPY(fp1, fp);
						fp1+= 3;
						fp+= dpoly;
					}
				}
				if( (cu->flag & CU_FRONT) && (dl->flag & DL_FRONT_CURVE) ) {
					dlnew= MEM_callocN(sizeof(DispList), "filldisp");
					BLI_addtail(&back, dlnew);
					dlnew->verts= fp1= MEM_mallocN(sizeof(float)*3*dl->parts, "filldisp1");
					dlnew->nr= dl->parts;
					dlnew->parts= 1;
					dlnew->type= DL_POLY;
					dlnew->col= dl->col;
					
					fp= dl->verts+3*(dl->nr-1);
					dpoly= 3*dl->nr;
					
					a= dl->parts;
					while(a--) {
						VECCOPY(fp1, fp);
						fp1+= 3;
						fp+= dpoly;
					}
				}
			}
		}
		dl= dl->next;
	}

	filldisplist(&front, dispbase);
	filldisplist(&back, dispbase);
	
	freedisplist(&front);
	freedisplist(&back);

	filldisplist(dispbase, dispbase);
	
}

void curve_to_filledpoly(Curve *cu, ListBase *nurb, ListBase *dispbase)
{
	if(cu->flag & CU_3D) return;

	if(dispbase->first && ((DispList*) dispbase->first)->type==DL_SURF) {
		bevels_to_filledpoly(cu, dispbase);
	}
	else {
		filldisplist(dispbase, dispbase);
	}
}


/* taper rules:
  - only 1 curve
  - first point left, last point right
  - based on subdivided points in original curve, not on points in taper curve (still)
*/
static float calc_taper(Object *taperobj, int cur, int tot)
{
	Curve *cu;
	DispList *dl;
	
	if(taperobj==NULL) return 1.0;
	
	cu= taperobj->data;
	dl= cu->disp.first;
	if(dl==NULL) {
		makeDispListCurveTypes(taperobj, 0);
		dl= cu->disp.first;
	}
	if(dl) {
		float fac= ((float)cur)/(float)(tot-1);
		float minx, dx, *fp;
		int a;
		
		/* horizontal size */
		minx= dl->verts[0];
		dx= dl->verts[3*(dl->nr-1)] - minx;
		if(dx>0.0) {
		
			fp= dl->verts;
			for(a=0; a<dl->nr; a++, fp+=3) {
				if( (fp[0]-minx)/dx >= fac) {
					/* interpolate with prev */
					if(a>0) {
						float fac1= (fp[-3]-minx)/dx;
						float fac2= (fp[0]-minx)/dx;
						if(fac1!=fac2)
							return fp[1]*(fac1-fac)/(fac1-fac2) + fp[-2]*(fac-fac2)/(fac1-fac2);
					}
					return fp[1];
				}
			}
			return fp[-2];	// last y coord
		}
	}
	
	return 1.0;
}

void makeDispListMBall(Object *ob)
{
	if(!ob || ob->type!=OB_MBALL) return;

	freedisplist(&(ob->disp));
	
	if(ob->type==OB_MBALL) {
		if(ob==find_basis_mball(ob)) {
			metaball_polygonize(ob);
			tex_space_mball(ob);

			object_deform_mball(ob);
		}
	}
	
	boundbox_displist(ob);
}

static ModifierData *curve_get_tesselate_point(Object *ob, int forRender, int editmode)
{
	ModifierData *md = modifiers_getVirtualModifierList(ob);
	ModifierData *preTesselatePoint;

	preTesselatePoint = NULL;
	for (; md; md=md->next) {
		ModifierTypeInfo *mti = modifierType_getInfo(md->type);

		if (!(md->mode&(1<<forRender))) continue;
		if (editmode && !(md->mode&eModifierMode_Editmode)) continue;
		if (mti->isDisabled && mti->isDisabled(md)) continue;

		if (md->type==eModifierType_Hook || md->type==eModifierType_Softbody) {
			preTesselatePoint  = md;
		}
	}

	return preTesselatePoint;
}

void curve_calc_modifiers_pre(Object *ob, ListBase *nurb, int forRender, float (**originalVerts_r)[3], float (**deformedVerts_r)[3], int *numVerts_r)
{
	int editmode = (!forRender && ob==G.obedit);
	ModifierData *md = modifiers_getVirtualModifierList(ob);
	ModifierData *preTesselatePoint = curve_get_tesselate_point(ob, forRender, editmode);
	int numVerts = 0;
	float (*originalVerts)[3] = NULL;
	float (*deformedVerts)[3] = NULL;

	if(ob!=G.obedit && do_ob_key(ob)) {
		deformedVerts = curve_getVertexCos(ob->data, nurb, &numVerts);
		originalVerts = MEM_dupallocN(deformedVerts);
	}
	
	if (preTesselatePoint) {
		for (; md; md=md->next) {
			ModifierTypeInfo *mti = modifierType_getInfo(md->type);

			if (!(md->mode&(1<<forRender))) continue;
			if (editmode && !(md->mode&eModifierMode_Editmode)) continue;
			if (mti->isDisabled && mti->isDisabled(md)) continue;
			if (mti->type!=eModifierTypeType_OnlyDeform) continue;

			if (!deformedVerts) {
				deformedVerts = curve_getVertexCos(ob->data, nurb, &numVerts);
				originalVerts = MEM_dupallocN(deformedVerts);
			}
			
			mti->deformVerts(md, ob, NULL, deformedVerts, numVerts);

			if (md==preTesselatePoint)
				break;
		}
	}

	if (deformedVerts) {
		curve_applyVertexCos(ob->data, nurb, deformedVerts);
	}

	*originalVerts_r = originalVerts;
	*deformedVerts_r = deformedVerts;
	*numVerts_r = numVerts;
}

void curve_calc_modifiers_post(Object *ob, ListBase *nurb, ListBase *dispbase, int forRender, float (*originalVerts)[3], float (*deformedVerts)[3])
{
	int editmode = (!forRender && ob==G.obedit);
	ModifierData *md = modifiers_getVirtualModifierList(ob);
	ModifierData *preTesselatePoint = curve_get_tesselate_point(ob, forRender, editmode);
	DispList *dl;

	if (preTesselatePoint) {
		md = preTesselatePoint->next;
	}

	for (; md; md=md->next) {
		ModifierTypeInfo *mti = modifierType_getInfo(md->type);

		if (!(md->mode&(1<<forRender))) continue;
		if (editmode && !(md->mode&eModifierMode_Editmode)) continue;
		if (mti->isDisabled && mti->isDisabled(md)) continue;
		if (mti->type!=eModifierTypeType_OnlyDeform) continue;

		for (dl=dispbase->first; dl; dl=dl->next) {
			mti->deformVerts(md, ob, NULL, (float(*)[3]) dl->verts, (dl->type==DL_INDEX3)?dl->nr:dl->parts*dl->nr);
		}
	}

	if (deformedVerts) {
		curve_applyVertexCos(ob->data, nurb, originalVerts);
		MEM_freeN(originalVerts);
		MEM_freeN(deformedVerts);
	}
}

void makeDispListSurf(Object *ob, ListBase *dispbase, int forRender)
{
	ListBase *nubase;
	Nurb *nu;
	Curve *cu = ob->data;
	DispList *dl;
	float *data;
	int len;
	int numVerts;
	float (*originalVerts)[3];
	float (*deformedVerts)[3];
		
	if(!forRender && ob==G.obedit) {
		nubase= &editNurb;
	}
	else {
		nubase= &cu->nurb;
	}

	curve_calc_modifiers_pre(ob, nubase, forRender, &originalVerts, &deformedVerts, &numVerts);

	for (nu=nubase->first; nu; nu=nu->next) {
		if(forRender || nu->hide==0) {
			if(nu->pntsv==1) {
				len= nu->pntsu*nu->resolu;
				
				dl= MEM_callocN(sizeof(DispList), "makeDispListsurf");
				dl->verts= MEM_callocN(len*3*sizeof(float), "dlverts");
				
				BLI_addtail(dispbase, dl);
				dl->parts= 1;
				dl->nr= len;
				dl->col= nu->mat_nr;
				dl->rt= nu->flag;
				
				data= dl->verts;
				if(nu->flagu & 1) dl->type= DL_POLY;
				else dl->type= DL_SEGM;
				
				makeNurbcurve(nu, data, 3);
			}
			else {
				len= nu->resolu*nu->resolv;
				
				dl= MEM_callocN(sizeof(DispList), "makeDispListsurf");
				dl->verts= MEM_callocN(len*3*sizeof(float), "dlverts");
				BLI_addtail(dispbase, dl);

				dl->col= nu->mat_nr;
				dl->rt= nu->flag;
				
				data= dl->verts;
				dl->type= DL_SURF;

				dl->parts= nu->resolu;	/* in reverse, because makeNurbfaces works that way */
				dl->nr= nu->resolv;
				if(nu->flagv & CU_CYCLIC) dl->flag|= DL_CYCL_U;	/* reverse too! */
				if(nu->flagu & CU_CYCLIC) dl->flag|= DL_CYCL_V;

				makeNurbfaces(nu, data, 0);
			}
		}
	}

	if (!forRender) {
		tex_space_curve(cu);
	}

	curve_calc_modifiers_post(ob, nubase, dispbase, forRender, originalVerts, deformedVerts);
}

void makeDispListCurveTypes(Object *ob, int forOrco)
{
	Curve *cu = ob->data;
	ListBase *dispbase;
	
	/* we do allow duplis... this is only displist on curve level */
	if(!ELEM3(ob->type, OB_SURF, OB_CURVE, OB_FONT)) return;

	freedisplist(&(ob->disp));
	dispbase= &(cu->disp);
	freedisplist(dispbase);
	
	if(ob->type==OB_SURF) {
		makeDispListSurf(ob, dispbase, 0);
	}
	else if ELEM(ob->type, OB_CURVE, OB_FONT) {
		ListBase dlbev;
		float (*originalVerts)[3];
		float (*deformedVerts)[3];
		int obedit= (G.obedit && G.obedit->data==ob->data && G.obedit->type==OB_CURVE);
		ListBase *nubase = obedit?&editNurb:&cu->nurb;
		int numVerts;

		BLI_freelistN(&(cu->bev));
		
		if(cu->path) free_path(cu->path);
		cu->path= NULL;
		
		if(ob->type==OB_FONT) text_to_curve(ob, 0);
		
		if(!forOrco) curve_calc_modifiers_pre(ob, nubase, 0, &originalVerts, &deformedVerts, &numVerts);

		makeBevelList(ob);

			/* If curve has no bevel will return nothing */
		makebevelcurve(ob, &dlbev);

		if (!dlbev.first) {
			curve_to_displist(cu, nubase, dispbase);
		} else {
			float widfac= cu->width-1.0;
			BevList *bl= cu->bev.first;
			Nurb *nu= nubase->first;

			for (; bl && nu; bl=bl->next,nu=nu->next) {
				DispList *dlb;
				
				for (dlb=dlbev.first; dlb; dlb=dlb->next) {
					DispList *dl;
					float *fp1, *data;
					BevPoint *bevp;
					int a,b;

						/* for each part of the bevel use a separate displblock */
					dl= MEM_callocN(sizeof(DispList), "makeDispListbev1");
					dl->verts= data= MEM_callocN(3*sizeof(float)*dlb->nr*bl->nr, "dlverts");
					BLI_addtail(dispbase, dl);

					dl->type= DL_SURF;
					
					dl->flag= dlb->flag & (DL_FRONT_CURVE|DL_BACK_CURVE);
					if(dlb->type==DL_POLY) dl->flag |= DL_CYCL_U;
					if(bl->poly>=0) dl->flag |= DL_CYCL_V;
					
					dl->parts= bl->nr;
					dl->nr= dlb->nr;
					dl->col= nu->mat_nr;
					dl->rt= nu->flag;
					dl->bevelSplitFlag= MEM_callocN(sizeof(*dl->col2)*((bl->nr+0x1F)>>5), "col2");
					bevp= (BevPoint *)(bl+1);

						/* for each point of poly make a bevel piece */
					bevp= (BevPoint *)(bl+1);
					for(a=0; a<bl->nr; a++,bevp++) {
						float fac = calc_taper(cu->taperobj, a, bl->nr);
						
						if (bevp->f1) {
							dl->bevelSplitFlag[a>>5] |= 1<<(a&0x1F);
						}

							/* rotate bevel piece and write in data */
						fp1= dlb->verts;
						for (b=0; b<dlb->nr; b++,fp1+=3,data+=3) {
							if(cu->flag & CU_3D) {
								float vec[3];

								vec[0]= fp1[1]+widfac;
								vec[1]= fp1[2];
								vec[2]= 0.0;
								
								Mat3MulVecfl(bevp->mat, vec);
								
								data[0]= bevp->x+ fac*vec[0];
								data[1]= bevp->y+ fac*vec[1];
								data[2]= bevp->z+ fac*vec[2];
							}
							else {
								data[0]= bevp->x+ fac*(widfac+fp1[1])*bevp->sina;
								data[1]= bevp->y+ fac*(widfac+fp1[1])*bevp->cosa;
								data[2]= bevp->z+ fac*fp1[2];
							}
						}
					}
				}
			}

			freedisplist(&dlbev);
		}

		curve_to_filledpoly(cu, nubase, dispbase);

		if(cu->flag & CU_PATH) calc_curvepath(ob);

		if(!forOrco) curve_calc_modifiers_post(ob, nubase, &cu->disp, 0, originalVerts, deformedVerts);
		tex_space_curve(cu);
	}
	
	boundbox_displist(ob);
}

void imagestodisplist(void)
{
	/* removed */
}

static void boundbox_displist(Object *ob)
{
	BoundBox *bb=0;
	float min[3], max[3];
	DispList *dl;
	float *vert;
	int a, tot=0;
	
	INIT_MINMAX(min, max);

	if(ELEM3(ob->type, OB_CURVE, OB_SURF, OB_FONT)) {
		Curve *cu= ob->data;

		if(cu->bb==0) cu->bb= MEM_callocN(sizeof(BoundBox), "boundbox");	
		bb= cu->bb;
		
		dl= cu->disp.first;

		while (dl) {
			if(dl->type==DL_INDEX3) tot= dl->nr;
			else tot= dl->nr*dl->parts;
			
			vert= dl->verts;
			for(a=0; a<tot; a++, vert+=3) {
				DO_MINMAX(vert, min, max);
			}

			dl= dl->next;
		}
	}
	
	if(bb) {
		boundbox_set_from_min_max(bb, min, max);
	}
}

void displistmesh_add_edges(DispListMesh *dlm)
{
	EdgeHash *eh = BLI_edgehash_new();
	EdgeHashIterator *ehi;
	int i;

	for (i=0; i<dlm->totface; i++) {
		MFace *mf = &dlm->mface[i];

		if (!BLI_edgehash_haskey(eh, mf->v1, mf->v2))
			BLI_edgehash_insert(eh, mf->v1, mf->v2, NULL);
		if (!BLI_edgehash_haskey(eh, mf->v2, mf->v3))
			BLI_edgehash_insert(eh, mf->v2, mf->v3, NULL);
		
		if (mf->v4) {
			if (!BLI_edgehash_haskey(eh, mf->v3, mf->v4))
				BLI_edgehash_insert(eh, mf->v3, mf->v4, NULL);
			if (!BLI_edgehash_haskey(eh, mf->v4, mf->v1))
				BLI_edgehash_insert(eh, mf->v4, mf->v1, NULL);
		} else {
			if (!BLI_edgehash_haskey(eh, mf->v3, mf->v1))
				BLI_edgehash_insert(eh, mf->v3, mf->v1, NULL);
		}
	}

	dlm->totedge = BLI_edgehash_size(eh);
	dlm->medge = MEM_callocN(dlm->totedge*sizeof(*dlm->medge), "medge");

	ehi = BLI_edgehashIterator_new(eh);
	for (i=0; !BLI_edgehashIterator_isDone(ehi); BLI_edgehashIterator_step(ehi)) {
		MEdge *med = &dlm->medge[i++];

		BLI_edgehashIterator_getKey(ehi, &med->v1, &med->v2);

		med->flag = ME_EDGEDRAW|ME_EDGERENDER;
	}
	BLI_edgehashIterator_free(ehi);

	BLI_edgehash_free(eh, NULL);
}
