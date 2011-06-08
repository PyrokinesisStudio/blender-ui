/*
* $Id$
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
* along with this program; if not, write to the Free Software  Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
* The Original Code is Copyright (C) 2005 by the Blender Foundation.
* All rights reserved.
*
* Contributor(s): Daniel Dunbar
*                 Ton Roosendaal,
*                 Ben Batt,
*                 Brecht Van Lommel,
*                 Campbell Barton
*
* ***** END GPL LICENSE BLOCK *****
*
*/

/** \file blender/modifiers/intern/MOD_explode.c
 *  \ingroup modifiers
 */


#include "DNA_meshdata_types.h"
#include "DNA_scene_types.h"

#include "BLI_kdtree.h"
#include "BLI_rand.h"
#include "BLI_math.h"
#include "BLI_edgehash.h"
#include "BLI_utildefines.h"

#include "BKE_cdderivedmesh.h"
#include "BKE_deform.h"
#include "BKE_lattice.h"
#include "BKE_mesh.h"
#include "BKE_modifier.h"
#include "BKE_object.h"
#include "BKE_particle.h"
#include "BKE_scene.h"


#include "MEM_guardedalloc.h"

#include "MOD_util.h"

static void initData(ModifierData *md)
{
	ExplodeModifierData *emd= (ExplodeModifierData*) md;

	emd->facepa= NULL;
	emd->flag |= eExplodeFlag_Unborn+eExplodeFlag_Alive+eExplodeFlag_Dead;
}
static void freeData(ModifierData *md)
{
	ExplodeModifierData *emd= (ExplodeModifierData*) md;
	
	if(emd->facepa) MEM_freeN(emd->facepa);
}
static void copyData(ModifierData *md, ModifierData *target)
{
	ExplodeModifierData *emd= (ExplodeModifierData*) md;
	ExplodeModifierData *temd= (ExplodeModifierData*) target;

	temd->facepa = NULL;
	temd->flag = emd->flag;
	temd->protect = emd->protect;
	temd->vgroup = emd->vgroup;
}
static int dependsOnTime(ModifierData *UNUSED(md)) 
{
	return 1;
}
static CustomDataMask requiredDataMask(Object *UNUSED(ob), ModifierData *md)
{
	ExplodeModifierData *emd= (ExplodeModifierData*) md;
	CustomDataMask dataMask = 0;

	if(emd->vgroup)
		dataMask |= CD_MASK_MDEFORMVERT;

	return dataMask;
}

static void createFacepa(ExplodeModifierData *emd,
						ParticleSystemModifierData *psmd,
						DerivedMesh *dm)
{
	ParticleSystem *psys=psmd->psys;
	MFace *fa=NULL, *mface=NULL;
	MVert *mvert = NULL;
	ParticleData *pa;
	KDTree *tree;
	float center[3], co[3];
	int *facepa=NULL,*vertpa=NULL,totvert=0,totface=0,totpart=0;
	int i,p,v1,v2,v3,v4=0;

	mvert = dm->getVertArray(dm);
	mface = dm->getFaceArray(dm);
	totface= dm->getNumFaces(dm);
	totvert= dm->getNumVerts(dm);
	totpart= psmd->psys->totpart;

	BLI_srandom(psys->seed);

	if(emd->facepa)
		MEM_freeN(emd->facepa);

	facepa = emd->facepa = MEM_callocN(sizeof(int)*totface, "explode_facepa");

	vertpa = MEM_callocN(sizeof(int)*totvert, "explode_vertpa");

	/* initialize all faces & verts to no particle */
	for(i=0; i<totface; i++)
		facepa[i]=totpart;

	for (i=0; i<totvert; i++)
		vertpa[i]=totpart;

	/* set protected verts */
	if(emd->vgroup){
		MDeformVert *dvert = dm->getVertDataArray(dm, CD_MDEFORMVERT);
		if(dvert){
			const int defgrp_index= emd->vgroup-1;
			for(i=0; i<totvert; i++, dvert++){
				float val = BLI_frand();
				val = (1.0f-emd->protect)*val + emd->protect*0.5f;
				if(val < defvert_find_weight(dvert, defgrp_index))
					vertpa[i] = -1;
			}
		}
	}

	/* make tree of emitter locations */
	tree=BLI_kdtree_new(totpart);
	for(p=0,pa=psys->particles; p<totpart; p++,pa++){
		psys_particle_on_emitter(psmd,psys->part->from,pa->num,pa->num_dmcache,pa->fuv,pa->foffset,co,NULL,NULL,NULL,NULL,NULL);
		BLI_kdtree_insert(tree, p, co, NULL);
	}
	BLI_kdtree_balance(tree);

	/* set face-particle-indexes to nearest particle to face center */
	for(i=0,fa=mface; i<totface; i++,fa++){
		add_v3_v3v3(center,mvert[fa->v1].co,mvert[fa->v2].co);
		add_v3_v3(center, mvert[fa->v3].co);
		if(fa->v4){
			add_v3_v3(center, mvert[fa->v4].co);
			mul_v3_fl(center,0.25);
		}
		else
			mul_v3_fl(center,0.3333f);

		p= BLI_kdtree_find_nearest(tree,center,NULL,NULL);

		v1=vertpa[fa->v1];
		v2=vertpa[fa->v2];
		v3=vertpa[fa->v3];
		if(fa->v4)
			v4=vertpa[fa->v4];

		if(v1>=0 && v2>=0 && v3>=0 && (fa->v4==0 || v4>=0))
			facepa[i]=p;

		if(v1>=0) vertpa[fa->v1]=p;
		if(v2>=0) vertpa[fa->v2]=p;
		if(v3>=0) vertpa[fa->v3]=p;
		if(fa->v4 && v4>=0) vertpa[fa->v4]=p;
	}

	if(vertpa) MEM_freeN(vertpa);
	BLI_kdtree_free(tree);
}

static int edgecut_get(EdgeHash *edgehash, int v1, int v2)
{
	return GET_INT_FROM_POINTER(BLI_edgehash_lookup(edgehash, v1, v2));
}

 
static const short add_faces[24] = {
	0,
	0, 0, 2, 0, 1, 2, 2, 0, 2, 1,
	2, 2, 2, 2, 3, 0, 0, 0, 1, 0,
	1, 1, 2
 };

static MFace *get_dface(DerivedMesh *dm, DerivedMesh *split, int cur, int i, MFace *mf)
{
	MFace *df = CDDM_get_face(split, cur);
	DM_copy_face_data(dm, split, i, cur, 1);
	*df = *mf;
	return df;
}

#define SET_VERTS(a, b, c, d) \
			v[0]=mf->v##a; uv[0]=a-1; \
			v[1]=mf->v##b; uv[1]=b-1; \
			v[2]=mf->v##c; uv[2]=c-1; \
			v[3]=mf->v##d; uv[3]=d-1;

#define GET_ES(v1, v2) edgecut_get(eh, v1, v2);
#define INT_UV(uvf, c0, c1) interp_v2_v2v2(uvf, mf->uv[c0], mf->uv[c1], 0.5f);

static void remap_faces_3_6_9_12(DerivedMesh *dm, DerivedMesh *split, MFace *mf, int *facepa, int *vertpa, int i, EdgeHash *eh, int cur, int v1, int v2, int v3, int v4)
{
	MFace *df1 = get_dface(dm, split, cur, i, mf);
	MFace *df2 = get_dface(dm, split, cur+1, i, mf);
	MFace *df3 = get_dface(dm, split, cur+2, i, mf);

	facepa[cur] = vertpa[v1];
	df1->v1 = v1;
	df1->v2 = GET_ES(v1, v2)
	df1->v3 = GET_ES(v2, v3)
	df1->v4 = v3;
	df1->flag |= ME_FACE_SEL;

	facepa[cur+1] = vertpa[v2];
	df2->v1 = GET_ES(v1, v2)
	df2->v2 = v2;
	df2->v3 = GET_ES(v2, v3)
	df2->v4 = 0;
	df2->flag &= ~ME_FACE_SEL;

	facepa[cur+2] = vertpa[v1];
	df3->v1 = v1;
	df3->v2 = v3;
	df3->v3 = v4;
	df3->v4 = 0;
	df3->flag &= ~ME_FACE_SEL;
}

static void remap_uvs_3_6_9_12(DerivedMesh *dm, DerivedMesh *split, int numlayer, int i, int cur, int c0, int c1, int c2, int c3)
{
	MTFace *mf, *df1, *df2, *df3;
	int l;

	for(l=0; l<numlayer; l++) {
		mf = CustomData_get_layer_n(&split->faceData, CD_MTFACE, l);
		df1 = mf+cur;
		df2 = df1 + 1;
		df3 = df1 + 2;
		mf = CustomData_get_layer_n(&dm->faceData, CD_MTFACE, l);
		mf += i;

		copy_v2_v2(df1->uv[0], mf->uv[c0]);
		INT_UV(df1->uv[1], c0, c1)
		INT_UV(df1->uv[2], c1, c2)
		copy_v2_v2(df1->uv[3], mf->uv[c2]);

		INT_UV(df2->uv[0], c0, c1)
		copy_v2_v2(df2->uv[1], mf->uv[c1]);
		INT_UV(df2->uv[2], c1, c2)

		copy_v2_v2(df3->uv[0], mf->uv[c0]);
		copy_v2_v2(df3->uv[1], mf->uv[c2]);
		copy_v2_v2(df3->uv[2], mf->uv[c3]);
	}
}

static void remap_faces_5_10(DerivedMesh *dm, DerivedMesh *split, MFace *mf, int *facepa, int *vertpa, int i, EdgeHash *eh, int cur, int v1, int v2, int v3, int v4)
{
	MFace *df1 = get_dface(dm, split, cur, i, mf);
	MFace *df2 = get_dface(dm, split, cur+1, i, mf);

	facepa[cur] = vertpa[v1];
	df1->v1 = v1;
	df1->v2 = v2;
	df1->v3 = GET_ES(v2, v3)
	df1->v4 = GET_ES(v1, v4)
	df1->flag |= ME_FACE_SEL;

	facepa[cur+1] = vertpa[v3];
	df2->v1 = GET_ES(v1, v4)
	df2->v2 = GET_ES(v2, v3)
	df2->v3 = v3;
	df2->v4 = v4;
	df2->flag |= ME_FACE_SEL;
}

static void remap_uvs_5_10(DerivedMesh *dm, DerivedMesh *split, int numlayer, int i, int cur, int c0, int c1, int c2, int c3)
{
	MTFace *mf, *df1, *df2;
	int l;

	for(l=0; l<numlayer; l++) {
		mf = CustomData_get_layer_n(&split->faceData, CD_MTFACE, l);
		df1 = mf+cur;
		df2 = df1 + 1;
		mf = CustomData_get_layer_n(&dm->faceData, CD_MTFACE, l);
		mf += i;

		copy_v2_v2(df1->uv[0], mf->uv[c0]);
		copy_v2_v2(df1->uv[1], mf->uv[c1]);
		INT_UV(df1->uv[2], c1, c2)
		INT_UV(df1->uv[3], c0, c3)

		INT_UV(df2->uv[0], c0, c3)
		INT_UV(df2->uv[1], c1, c2)
		copy_v2_v2(df2->uv[2], mf->uv[c2]);
		copy_v2_v2(df2->uv[3], mf->uv[c3]);

	}
}

static void remap_faces_15(DerivedMesh *dm, DerivedMesh *split, MFace *mf, int *facepa, int *vertpa, int i, EdgeHash *eh, int cur, int v1, int v2, int v3, int v4)
{
	MFace *df1 = get_dface(dm, split, cur, i, mf);
	MFace *df2 = get_dface(dm, split, cur+1, i, mf);
	MFace *df3 = get_dface(dm, split, cur+2, i, mf);
	MFace *df4 = get_dface(dm, split, cur+3, i, mf);

	facepa[cur] = vertpa[v1];
	df1->v1 = v1;
	df1->v2 = GET_ES(v1, v2)
	df1->v3 = GET_ES(v1, v3)
	df1->v4 = GET_ES(v1, v4)
	df1->flag |= ME_FACE_SEL;

	facepa[cur+1] = vertpa[v2];
	df2->v1 = GET_ES(v1, v2)
	df2->v2 = v2;
	df2->v3 = GET_ES(v2, v3)
	df2->v4 = GET_ES(v1, v3)
	df2->flag |= ME_FACE_SEL;

	facepa[cur+2] = vertpa[v3];
	df3->v1 = GET_ES(v1, v3)
	df3->v2 = GET_ES(v2, v3)
	df3->v3 = v3;
	df3->v4 = GET_ES(v3, v4)
	df3->flag |= ME_FACE_SEL;

	facepa[cur+3] = vertpa[v4];
	df4->v1 = GET_ES(v1, v4)
	df4->v2 = GET_ES(v1, v3)
	df4->v3 = GET_ES(v3, v4)
	df4->v4 = v4;
	df4->flag |= ME_FACE_SEL;
}

static void remap_uvs_15(DerivedMesh *dm, DerivedMesh *split, int numlayer, int i, int cur, int c0, int c1, int c2, int c3)
{
	MTFace *mf, *df1, *df2, *df3, *df4;
	int l;

	for(l=0; l<numlayer; l++) {
		mf = CustomData_get_layer_n(&split->faceData, CD_MTFACE, l);
		df1 = mf+cur;
		df2 = df1 + 1;
		df3 = df1 + 2;
		df4 = df1 + 3;
		mf = CustomData_get_layer_n(&dm->faceData, CD_MTFACE, l);
		mf += i;

		copy_v2_v2(df1->uv[0], mf->uv[c0]);
		INT_UV(df1->uv[1], c0, c1)
		INT_UV(df1->uv[2], c0, c2)
		INT_UV(df1->uv[3], c0, c3)

		INT_UV(df2->uv[0], c0, c1)
		copy_v2_v2(df2->uv[1], mf->uv[c1]);
		INT_UV(df2->uv[2], c1, c2)
		INT_UV(df2->uv[3], c0, c2)

		INT_UV(df3->uv[0], c0, c2)
		INT_UV(df3->uv[1], c1, c2)
		copy_v2_v2(df3->uv[2], mf->uv[c2]);
		INT_UV(df3->uv[3], c2, c3)

		INT_UV(df4->uv[0], c0, c3)
		INT_UV(df4->uv[1], c0, c2)
		INT_UV(df4->uv[2], c2, c3)
		copy_v2_v2(df4->uv[3], mf->uv[c3]);
	}
}

static void remap_faces_7_11_13_14(DerivedMesh *dm, DerivedMesh *split, MFace *mf, int *facepa, int *vertpa, int i, EdgeHash *eh, int cur, int v1, int v2, int v3, int v4)
{
	MFace *df1 = get_dface(dm, split, cur, i, mf);
	MFace *df2 = get_dface(dm, split, cur+1, i, mf);
	MFace *df3 = get_dface(dm, split, cur+2, i, mf);

	facepa[cur] = vertpa[v1];
	df1->v1 = v1;
	df1->v2 = GET_ES(v1, v2)
	df1->v3 = GET_ES(v2, v3)
	df1->v4 = GET_ES(v1, v4)
	df1->flag |= ME_FACE_SEL;

	facepa[cur+1] = vertpa[v2];
	df2->v1 = GET_ES(v1, v2)
	df2->v2 = v2;
	df2->v3 = GET_ES(v2, v3)
	df2->v4 = 0;
	df2->flag &= ~ME_FACE_SEL;

	facepa[cur+2] = vertpa[v4];
	df3->v1 = GET_ES(v1, v4)
	df3->v2 = GET_ES(v2, v3)
	df3->v3 = v3;
	df3->v4 = v4;
	df3->flag |= ME_FACE_SEL;
}

static void remap_uvs_7_11_13_14(DerivedMesh *dm, DerivedMesh *split, int numlayer, int i, int cur, int c0, int c1, int c2, int c3)
{
	MTFace *mf, *df1, *df2, *df3;
	int l;

	for(l=0; l<numlayer; l++) {
		mf = CustomData_get_layer_n(&split->faceData, CD_MTFACE, l);
		df1 = mf+cur;
		df2 = df1 + 1;
		df3 = df1 + 2;
		mf = CustomData_get_layer_n(&dm->faceData, CD_MTFACE, l);
		mf += i;

		copy_v2_v2(df1->uv[0], mf->uv[c0]);
		INT_UV(df1->uv[1], c0, c1)
		INT_UV(df1->uv[2], c1, c2)
		INT_UV(df1->uv[3], c0, c3)

		INT_UV(df2->uv[0], c0, c1)
		copy_v2_v2(df2->uv[1], mf->uv[c1]);
		INT_UV(df2->uv[2], c1, c2)

		INT_UV(df3->uv[0], c0, c3)
		INT_UV(df3->uv[1], c1, c2)
		copy_v2_v2(df3->uv[2], mf->uv[c2]);
		copy_v2_v2(df3->uv[3], mf->uv[c3]);
	}
}

static void remap_faces_19_21_22(DerivedMesh *dm, DerivedMesh *split, MFace *mf, int *facepa, int *vertpa, int i, EdgeHash *eh, int cur, int v1, int v2, int v3)
{
	MFace *df1 = get_dface(dm, split, cur, i, mf);
	MFace *df2 = get_dface(dm, split, cur+1, i, mf);

	facepa[cur] = vertpa[v1];
	df1->v1 = v1;
	df1->v2 = GET_ES(v1, v2)
	df1->v3 = GET_ES(v1, v3)
	df1->v4 = 0;
	df1->flag &= ~ME_FACE_SEL;

	facepa[cur+1] = vertpa[v2];
	df2->v1 = GET_ES(v1, v2)
	df2->v2 = v2;
	df2->v3 = v3;
	df2->v4 = GET_ES(v1, v3)
	df2->flag |= ME_FACE_SEL;
}

static void remap_uvs_19_21_22(DerivedMesh *dm, DerivedMesh *split, int numlayer, int i, int cur, int c0, int c1, int c2)
{
	MTFace *mf, *df1, *df2;
	int l;

	for(l=0; l<numlayer; l++) {
		mf = CustomData_get_layer_n(&split->faceData, CD_MTFACE, l);
		df1 = mf+cur;
		df2 = df1 + 1;
		mf = CustomData_get_layer_n(&dm->faceData, CD_MTFACE, l);
		mf += i;

		copy_v2_v2(df1->uv[0], mf->uv[c0]);
		INT_UV(df1->uv[1], c0, c1)
		INT_UV(df1->uv[2], c0, c2)

		INT_UV(df2->uv[0], c0, c1)
		copy_v2_v2(df2->uv[1], mf->uv[c1]);
		copy_v2_v2(df2->uv[2], mf->uv[c2]);
		INT_UV(df2->uv[3], c0, c2)
	}
}

static void remap_faces_23(DerivedMesh *dm, DerivedMesh *split, MFace *mf, int *facepa, int *vertpa, int i, EdgeHash *eh, int cur, int v1, int v2, int v3)
{
	MFace *df1 = get_dface(dm, split, cur, i, mf);
	MFace *df2 = get_dface(dm, split, cur+1, i, mf);
	MFace *df3 = get_dface(dm, split, cur+2, i, mf);

	facepa[cur] = vertpa[v1];
	df1->v1 = v1;
	df1->v2 = GET_ES(v1, v2)
	df1->v3 = GET_ES(v2, v3)
	df1->v4 = GET_ES(v1, v3)
	df1->flag |= ME_FACE_SEL;

	facepa[cur+1] = vertpa[v2];
	df2->v1 = GET_ES(v1, v2)
	df2->v2 = v2;
	df2->v3 = GET_ES(v2, v3)
	df2->v4 = 0;
	df2->flag &= ~ME_FACE_SEL;

	facepa[cur+2] = vertpa[v3];
	df3->v1 = GET_ES(v1, v3)
	df3->v2 = GET_ES(v2, v3)
	df3->v3 = v3;
	df3->v4 = 0;
	df3->flag &= ~ME_FACE_SEL;
}

static void remap_uvs_23(DerivedMesh *dm, DerivedMesh *split, int numlayer, int i, int cur, int c0, int c1, int c2)
{
	MTFace *mf, *df1, *df2;
	int l;

	for(l=0; l<numlayer; l++) {
		mf = CustomData_get_layer_n(&split->faceData, CD_MTFACE, l);
		df1 = mf+cur;
		df2 = df1 + 1;
		mf = CustomData_get_layer_n(&dm->faceData, CD_MTFACE, l);
		mf += i;

		copy_v2_v2(df1->uv[0], mf->uv[c0]);
		INT_UV(df1->uv[1], c0, c1)
		INT_UV(df1->uv[2], c1, c2)
		INT_UV(df1->uv[3], c0, c2)

		INT_UV(df2->uv[0], c0, c1)
		copy_v2_v2(df2->uv[1], mf->uv[c1]);
		INT_UV(df2->uv[2], c1, c2)

		INT_UV(df2->uv[0], c0, c2)
		INT_UV(df2->uv[1], c1, c2)
		copy_v2_v2(df2->uv[2], mf->uv[c2]);
	}
}

static DerivedMesh * cutEdges(ExplodeModifierData *emd, DerivedMesh *dm){
	DerivedMesh *splitdm;
	MFace *mf=NULL,*df1=NULL;
	MFace *mface=dm->getFaceArray(dm);
	MVert *dupve, *mv;
	EdgeHash *edgehash;
	EdgeHashIterator *ehi;
	int totvert=dm->getNumVerts(dm);
	int totface=dm->getNumFaces(dm);

	int *facesplit = MEM_callocN(sizeof(int)*totface,"explode_facesplit");
	int *vertpa = MEM_callocN(sizeof(int)*totvert,"explode_vertpa2");
	int *facepa = emd->facepa;
	int *fs, totesplit=0,totfsplit=0,curdupface=0;
	int i,j,v1,v2,v3,v4,esplit, v[4], uv[4];
	int numlayer;

	edgehash= BLI_edgehash_new();

	/* recreate vertpa from facepa calculation */
	for (i=0,mf=mface; i<totface; i++,mf++) {
		vertpa[mf->v1]=facepa[i];
		vertpa[mf->v2]=facepa[i];
		vertpa[mf->v3]=facepa[i];
		if(mf->v4)
			vertpa[mf->v4]=facepa[i];
	}

	/* mark edges for splitting and how to split faces */
	for (i=0,mf=mface,fs=facesplit; i<totface; i++,mf++,fs++) {
		v1=vertpa[mf->v1];
		v2=vertpa[mf->v2];
		v3=vertpa[mf->v3];

		if(v1!=v2){
			BLI_edgehash_insert(edgehash, mf->v1, mf->v2, NULL);
			(*fs) |= 1;
		}

		if(v2!=v3){
			BLI_edgehash_insert(edgehash, mf->v2, mf->v3, NULL);
			(*fs) |= 2;
		}

		if(mf->v4){
			v4=vertpa[mf->v4];

			if(v3!=v4){
				BLI_edgehash_insert(edgehash, mf->v3, mf->v4, NULL);
				(*fs) |= 4;
			}

			if(v1!=v4){
				BLI_edgehash_insert(edgehash, mf->v1, mf->v4, NULL);
				(*fs) |= 8;
			}

			/* mark center vertex as a fake edge split */
			if(*fs == 15)
				BLI_edgehash_insert(edgehash, mf->v1, mf->v3, NULL);
		}
		else {
			(*fs) |= 16; /* mark face as tri */

			if(v1!=v3){
				BLI_edgehash_insert(edgehash, mf->v1, mf->v3, NULL);
				(*fs) |= 4;
			}
		}
	}

	/* count splits & create indexes for new verts */
	ehi= BLI_edgehashIterator_new(edgehash);
	totesplit=totvert;
	for(; !BLI_edgehashIterator_isDone(ehi); BLI_edgehashIterator_step(ehi)) {
		BLI_edgehashIterator_setValue(ehi, SET_INT_IN_POINTER(totesplit));
		totesplit++;
	}
	BLI_edgehashIterator_free(ehi);

	/* count new faces due to splitting */
	for(i=0,fs=facesplit; i<totface; i++,fs++)
		totfsplit += add_faces[*fs];
	
	splitdm= CDDM_from_template(dm, totesplit, 0, totface+totfsplit);
	numlayer = CustomData_number_of_layers(&splitdm->faceData, CD_MTFACE);

	/* copy new faces & verts (is it really this painful with custom data??) */
	for(i=0; i<totvert; i++){
		MVert source;
		MVert *dest;
		dm->getVert(dm, i, &source);
		dest = CDDM_get_vert(splitdm, i);

		DM_copy_vert_data(dm, splitdm, i, i, 1);
		*dest = source;
	}

	/* override original facepa (original pointer is saved in caller function) */
	facepa= MEM_callocN(sizeof(int)*(totface+totfsplit),"explode_facepa");
	//memcpy(facepa,emd->facepa,totface*sizeof(int));
	emd->facepa=facepa;

	/* create new verts */
	ehi= BLI_edgehashIterator_new(edgehash);
	for(; !BLI_edgehashIterator_isDone(ehi); BLI_edgehashIterator_step(ehi)) {
		BLI_edgehashIterator_getKey(ehi, &i, &j);
		esplit= GET_INT_FROM_POINTER(BLI_edgehashIterator_getValue(ehi));
		mv=CDDM_get_vert(splitdm,j);
		dupve=CDDM_get_vert(splitdm,esplit);

		DM_copy_vert_data(splitdm,splitdm,j,esplit,1);

		*dupve=*mv;

		mv=CDDM_get_vert(splitdm,i);

		add_v3_v3(dupve->co, mv->co);
		mul_v3_fl(dupve->co, 0.5f);
	}
	BLI_edgehashIterator_free(ehi);

	/* create new faces */
	curdupface=0;//=totface;
	//curdupin=totesplit;
	for(i=0,fs=facesplit; i<totface; i++,fs++){
		mf = dm->getFaceData(dm, i, CD_MFACE);

		switch(*fs) {
		case 3:
		case 10:
		case 11:
		case 15:
			SET_VERTS(1, 2, 3, 4)
			break;
		case 5:
		case 6:
		case 7:
			SET_VERTS(2, 3, 4, 1)
			break;
		case 9:
		case 13:
			SET_VERTS(4, 1, 2, 3)
			break;
		case 12:
		case 14:
			SET_VERTS(3, 4, 1, 2)
			break;
		case 21:
		case 23:
			SET_VERTS(1, 2, 3, 4)
			break;
		case 19:
			SET_VERTS(2, 3, 1, 4)
			break;
		case 22:
			SET_VERTS(3, 1, 2, 4)
			break;
		}

		switch(*fs) {
		case 3:
		case 6:
		case 9:
		case 12:
			remap_faces_3_6_9_12(dm, splitdm, mf, facepa, vertpa, i, edgehash, curdupface, v[0], v[1], v[2], v[3]);
			if(numlayer)
				remap_uvs_3_6_9_12(dm, splitdm, numlayer, i, curdupface, uv[0], uv[1], uv[2], uv[3]);
			break;
		case 5:
		case 10:
			remap_faces_5_10(dm, splitdm, mf, facepa, vertpa, i, edgehash, curdupface, v[0], v[1], v[2], v[3]);
			if(numlayer)
				remap_uvs_5_10(dm, splitdm, numlayer, i, curdupface, uv[0], uv[1], uv[2], uv[3]);
			break;
		case 15:
			remap_faces_15(dm, splitdm, mf, facepa, vertpa, i, edgehash, curdupface, v[0], v[1], v[2], v[3]);
			if(numlayer)
				remap_uvs_15(dm, splitdm, numlayer, i, curdupface, uv[0], uv[1], uv[2], uv[3]);
			break;
		case 7:
		case 11:
		case 13:
		case 14:
			remap_faces_7_11_13_14(dm, splitdm, mf, facepa, vertpa, i, edgehash, curdupface, v[0], v[1], v[2], v[3]);
			if(numlayer)
				remap_uvs_7_11_13_14(dm, splitdm, numlayer, i, curdupface, uv[0], uv[1], uv[2], uv[3]);
			break;
		case 19:
		case 21:
		case 22:
			remap_faces_19_21_22(dm, splitdm, mf, facepa, vertpa, i, edgehash, curdupface, v[0], v[1], v[2]);
			if(numlayer)
				remap_uvs_19_21_22(dm, splitdm, numlayer, i, curdupface, uv[0], uv[1], uv[2]);
			break;
		case 23:
			remap_faces_23(dm, splitdm, mf, facepa, vertpa, i, edgehash, curdupface, v[0], v[1], v[2]);
			if(numlayer)
				remap_uvs_23(dm, splitdm, numlayer, i, curdupface, uv[0], uv[1], uv[2]);
			break;
		case 0:
		case 16:
			df1 = get_dface(dm, splitdm, curdupface, i, mf);
			facepa[curdupface] = vertpa[mf->v1];

			if(df1->v4)
				df1->flag |= ME_FACE_SEL;
			else
				df1->flag &= ~ME_FACE_SEL;
			break;
		}

		curdupface += add_faces[*fs]+1;
	}

	for(i=0; i<curdupface; i++) {
		mf = CDDM_get_face(splitdm, i);
		test_index_face(mf, &splitdm->faceData, i, (mf->flag & ME_FACE_SEL ? 4 : 3));
	}

	BLI_edgehash_free(edgehash, NULL);
	MEM_freeN(facesplit);
	MEM_freeN(vertpa);

	return splitdm;

}
static DerivedMesh * explodeMesh(ExplodeModifierData *emd, 
		ParticleSystemModifierData *psmd, Scene *scene, Object *ob, 
  DerivedMesh *to_explode)
{
	DerivedMesh *explode, *dm=to_explode;
	MFace *mf= NULL, *mface;
	ParticleSettings *part=psmd->psys->part;
	ParticleSimulationData sim= {NULL};
	ParticleData *pa=NULL, *pars=psmd->psys->particles;
	ParticleKey state;
	EdgeHash *vertpahash;
	EdgeHashIterator *ehi;
	float *vertco= NULL, imat[4][4];
	float loc0[3], nor[3];
	float cfra;
	/* float timestep; */
	int *facepa=emd->facepa;
	int totdup=0,totvert=0,totface=0,totpart=0;
	int i, j, v, mindex=0;
	MTFace *mtface = NULL, *mtf;

	totface= dm->getNumFaces(dm);
	totvert= dm->getNumVerts(dm);
	mface= dm->getFaceArray(dm);
	totpart= psmd->psys->totpart;

	sim.scene= scene;
	sim.ob= ob;
	sim.psys= psmd->psys;
	sim.psmd= psmd;

	/* timestep= psys_get_timestep(&sim); */

	//if(part->flag & PART_GLOB_TIME)
		cfra= BKE_curframe(scene);
	//else
	//	cfra=bsystem_time(scene, ob,(float)scene->r.cfra,0.0);

	/* hash table for vertice <-> particle relations */
	vertpahash= BLI_edgehash_new();

	for (i=0; i<totface; i++) {
		/* do mindex + totvert to ensure the vertex index to be the first
		 * with BLI_edgehashIterator_getKey */
		if(facepa[i]==totpart || cfra <= (pars+facepa[i])->time)
			mindex = totvert+totpart;
		else 
			mindex = totvert+facepa[i];

		mf= &mface[i];

		/* set face vertices to exist in particle group */
		BLI_edgehash_insert(vertpahash, mf->v1, mindex, NULL);
		BLI_edgehash_insert(vertpahash, mf->v2, mindex, NULL);
		BLI_edgehash_insert(vertpahash, mf->v3, mindex, NULL);
		if(mf->v4)
			BLI_edgehash_insert(vertpahash, mf->v4, mindex, NULL);
	}

	/* make new vertice indexes & count total vertices after duplication */
	ehi= BLI_edgehashIterator_new(vertpahash);
	for(; !BLI_edgehashIterator_isDone(ehi); BLI_edgehashIterator_step(ehi)) {
		BLI_edgehashIterator_setValue(ehi, SET_INT_IN_POINTER(totdup));
		totdup++;
	}
	BLI_edgehashIterator_free(ehi);

	/* the final duplicated vertices */
	explode= CDDM_from_template(dm, totdup, 0,totface);
	mtface = CustomData_get_layer_named(&explode->faceData, CD_MTFACE, emd->uvname);
	/*dupvert= CDDM_get_verts(explode);*/

	/* getting back to object space */
	invert_m4_m4(imat,ob->obmat);

	psmd->psys->lattice = psys_get_lattice(&sim);

	/* duplicate & displace vertices */
	ehi= BLI_edgehashIterator_new(vertpahash);
	for(; !BLI_edgehashIterator_isDone(ehi); BLI_edgehashIterator_step(ehi)) {
		MVert source;
		MVert *dest;

		/* get particle + vertex from hash */
		BLI_edgehashIterator_getKey(ehi, &j, &i);
		i -= totvert;
		v= GET_INT_FROM_POINTER(BLI_edgehashIterator_getValue(ehi));

		dm->getVert(dm, j, &source);
		dest = CDDM_get_vert(explode,v);

		DM_copy_vert_data(dm,explode,j,v,1);
		*dest = source;

		if(i!=totpart) {
			/* get particle */
			pa= pars+i;

			/* get particle state */
			psys_particle_on_emitter(psmd,part->from,pa->num,pa->num_dmcache,pa->fuv,pa->foffset,loc0,nor,NULL,NULL,NULL,NULL);
			mul_m4_v3(ob->obmat,loc0);

			state.time=cfra;
			psys_get_particle_state(&sim, i, &state, 1);

			vertco=CDDM_get_vert(explode,v)->co;
			
			mul_m4_v3(ob->obmat,vertco);

			VECSUB(vertco,vertco,loc0);

			/* apply rotation, size & location */
			mul_qt_v3(state.rot,vertco);
			if(emd->flag & eExplodeFlag_PaSize)
				mul_v3_fl(vertco,pa->size);
			VECADD(vertco,vertco,state.co);

			mul_m4_v3(imat,vertco);
		}
	}
	BLI_edgehashIterator_free(ehi);

	/*map new vertices to faces*/
	for (i=0; i<totface; i++) {
		MFace source;
		int orig_v4;

		if(facepa[i]!=totpart)
		{
			pa=pars+facepa[i];

			if(pa->alive==PARS_UNBORN && (emd->flag&eExplodeFlag_Unborn)==0) continue;
			if(pa->alive==PARS_ALIVE && (emd->flag&eExplodeFlag_Alive)==0) continue;
			if(pa->alive==PARS_DEAD && (emd->flag&eExplodeFlag_Dead)==0) continue;
		}

		dm->getFace(dm,i,&source);
		mf=CDDM_get_face(explode,i);
		
		orig_v4 = source.v4;

		if(facepa[i]!=totpart && cfra <= pa->time)
			mindex = totvert+totpart;
		else 
			mindex = totvert+facepa[i];

		source.v1 = edgecut_get(vertpahash, source.v1, mindex);
		source.v2 = edgecut_get(vertpahash, source.v2, mindex);
		source.v3 = edgecut_get(vertpahash, source.v3, mindex);
		if(source.v4)
			source.v4 = edgecut_get(vertpahash, source.v4, mindex);

		DM_copy_face_data(dm,explode,i,i,1);

		*mf = source;

		/* override uv channel for particle age */
		if(mtface) {
			float age = (cfra - pa->time)/pa->lifetime;
			/* Clamp to this range to avoid flipping to the other side of the coordinates. */
			CLAMP(age, 0.001f, 0.999f);

			mtf = mtface + i;

			mtf->uv[0][0] = mtf->uv[1][0] = mtf->uv[2][0] = mtf->uv[3][0] = age;
			mtf->uv[0][1] = mtf->uv[1][1] = mtf->uv[2][1] = mtf->uv[3][1] = 0.5f;
		}

		test_index_face(mf, &explode->faceData, i, (orig_v4 ? 4 : 3));
	}

	/* cleanup */
	BLI_edgehash_free(vertpahash, NULL);

	/* finalization */
	CDDM_calc_edges(explode);
	CDDM_calc_normals(explode);

	if(psmd->psys->lattice){
		end_latt_deform(psmd->psys->lattice);
		psmd->psys->lattice= NULL;
	}

	return explode;
}

static ParticleSystemModifierData * findPrecedingParticlesystem(Object *ob, ModifierData *emd)
{
	ModifierData *md;
	ParticleSystemModifierData *psmd= NULL;

	for (md=ob->modifiers.first; emd!=md; md=md->next){
		if(md->type==eModifierType_ParticleSystem)
			psmd= (ParticleSystemModifierData*) md;
	}
	return psmd;
}
static DerivedMesh * applyModifier(ModifierData *md, Object *ob,
						DerivedMesh *derivedData,
						int UNUSED(useRenderParams),
						int UNUSED(isFinalCalc))
{
	DerivedMesh *dm = derivedData;
	ExplodeModifierData *emd= (ExplodeModifierData*) md;
	ParticleSystemModifierData *psmd=findPrecedingParticlesystem(ob,md);

	if(psmd){
		ParticleSystem * psys=psmd->psys;

		if(psys==NULL || psys->totpart==0) return derivedData;
		if(psys->part==NULL || psys->particles==NULL) return derivedData;
		if(psmd->dm==NULL) return derivedData;

		/* 1. find faces to be exploded if needed */
		if(emd->facepa == NULL
				 || psmd->flag&eParticleSystemFlag_Pars
				 || emd->flag&eExplodeFlag_CalcFaces
				 || MEM_allocN_len(emd->facepa)/sizeof(int) != dm->getNumFaces(dm))
		{
			if(psmd->flag & eParticleSystemFlag_Pars)
				psmd->flag &= ~eParticleSystemFlag_Pars;
			
			if(emd->flag & eExplodeFlag_CalcFaces)
				emd->flag &= ~eExplodeFlag_CalcFaces;

			createFacepa(emd,psmd,derivedData);
		}
		/* 2. create new mesh */
		if(emd->flag & eExplodeFlag_EdgeCut){
			int *facepa = emd->facepa;
			DerivedMesh *splitdm=cutEdges(emd,dm);
			DerivedMesh *explode=explodeMesh(emd, psmd, md->scene, ob, splitdm);

			MEM_freeN(emd->facepa);
			emd->facepa=facepa;
			splitdm->release(splitdm);
			return explode;
		}
		else
			return explodeMesh(emd, psmd, md->scene, ob, derivedData);
	}
	return derivedData;
}


ModifierTypeInfo modifierType_Explode = {
	/* name */              "Explode",
	/* structName */        "ExplodeModifierData",
	/* structSize */        sizeof(ExplodeModifierData),
	/* type */              eModifierTypeType_Constructive,
	/* flags */             eModifierTypeFlag_AcceptsMesh,
	/* copyData */          copyData,
	/* deformVerts */       NULL,
	/* deformMatrices */    NULL,
	/* deformVertsEM */     NULL,
	/* deformMatricesEM */  NULL,
	/* applyModifier */     applyModifier,
	/* applyModifierEM */   NULL,
	/* initData */          initData,
	/* requiredDataMask */  requiredDataMask,
	/* freeData */          freeData,
	/* isDisabled */        NULL,
	/* updateDepgraph */    NULL,
	/* dependsOnTime */     dependsOnTime,
	/* dependsOnNormals */	NULL,
	/* foreachObjectLink */ NULL,
	/* foreachIDLink */     NULL,
	/* foreachTexLink */    NULL,
};
