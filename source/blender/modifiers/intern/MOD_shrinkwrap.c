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

#include <string.h>

#include "BLI_string.h"
#include "BLI_utildefines.h"

#include "BKE_cdderivedmesh.h"
#include "BKE_modifier.h"
#include "BKE_shrinkwrap.h"

#include "DNA_object_types.h"

#include "depsgraph_private.h"

#include "MOD_util.h"


static void initData(ModifierData *md)
{
	ShrinkwrapModifierData *smd = (ShrinkwrapModifierData*) md;
	smd->shrinkType = MOD_SHRINKWRAP_NEAREST_SURFACE;
	smd->shrinkOpts = MOD_SHRINKWRAP_PROJECT_ALLOW_POS_DIR;
	smd->keepDist	= 0.0f;

	smd->target		= NULL;
	smd->auxTarget	= NULL;
}

static void copyData(ModifierData *md, ModifierData *target)
{
	ShrinkwrapModifierData *smd  = (ShrinkwrapModifierData*)md;
	ShrinkwrapModifierData *tsmd = (ShrinkwrapModifierData*)target;

	tsmd->target	= smd->target;
	tsmd->auxTarget = smd->auxTarget;

	BLI_strncpy(tsmd->vgroup_name, smd->vgroup_name, sizeof(tsmd->vgroup_name));

	tsmd->keepDist	= smd->keepDist;
	tsmd->shrinkType= smd->shrinkType;
	tsmd->shrinkOpts= smd->shrinkOpts;
	tsmd->projAxis = smd->projAxis;
	tsmd->subsurfLevels = smd->subsurfLevels;
}

static CustomDataMask requiredDataMask(Object *UNUSED(ob), ModifierData *md)
{
	ShrinkwrapModifierData *smd = (ShrinkwrapModifierData *)md;
	CustomDataMask dataMask = 0;

	/* ask for vertexgroups if we need them */
	if(smd->vgroup_name[0])
		dataMask |= CD_MASK_MDEFORMVERT;

	if(smd->shrinkType == MOD_SHRINKWRAP_PROJECT
	&& smd->projAxis == MOD_SHRINKWRAP_PROJECT_OVER_NORMAL)
		dataMask |= CD_MASK_MVERT;
		
	return dataMask;
}

static int isDisabled(ModifierData *md, int UNUSED(useRenderParams))
{
	ShrinkwrapModifierData *smd = (ShrinkwrapModifierData*) md;
	return !smd->target;
}


static void foreachObjectLink(ModifierData *md, Object *ob, ObjectWalkFunc walk, void *userData)
{
	ShrinkwrapModifierData *smd = (ShrinkwrapModifierData*) md;

	walk(userData, ob, &smd->target);
	walk(userData, ob, &smd->auxTarget);
}

static void deformVerts(ModifierData *md, Object *ob,
						DerivedMesh *derivedData,
						float (*vertexCos)[3],
						int numVerts,
						int UNUSED(useRenderParams),
						int UNUSED(isFinalCalc))
{
	DerivedMesh *dm = derivedData;
	CustomDataMask dataMask = requiredDataMask(ob, md);

	/* ensure we get a CDDM with applied vertex coords */
	if(dataMask)
		dm= get_cddm(ob, NULL, dm, vertexCos);

	shrinkwrapModifier_deform((ShrinkwrapModifierData*)md, ob, dm, vertexCos, numVerts);

	if(dm != derivedData)
		dm->release(dm);
}

static void deformVertsEM(ModifierData *md, Object *ob, struct EditMesh *editData, DerivedMesh *derivedData, float (*vertexCos)[3], int numVerts)
{
	DerivedMesh *dm = derivedData;
	CustomDataMask dataMask = requiredDataMask(ob, md);

	/* ensure we get a CDDM with applied vertex coords */
	if(dataMask)
		dm= get_cddm(ob, editData, dm, vertexCos);

	shrinkwrapModifier_deform((ShrinkwrapModifierData*)md, ob, dm, vertexCos, numVerts);

	if(dm != derivedData)
		dm->release(dm);
}

static void updateDepgraph(ModifierData *md, DagForest *forest,
						struct Scene *UNUSED(scene),
						Object *UNUSED(ob),
						DagNode *obNode)
{
	ShrinkwrapModifierData *smd = (ShrinkwrapModifierData*) md;

	if (smd->target)
		dag_add_relation(forest, dag_get_node(forest, smd->target),   obNode, DAG_RL_OB_DATA | DAG_RL_DATA_DATA, "Shrinkwrap Modifier");

	if (smd->auxTarget)
		dag_add_relation(forest, dag_get_node(forest, smd->auxTarget), obNode, DAG_RL_OB_DATA | DAG_RL_DATA_DATA, "Shrinkwrap Modifier");
}


ModifierTypeInfo modifierType_Shrinkwrap = {
	/* name */              "Shrinkwrap",
	/* structName */        "ShrinkwrapModifierData",
	/* structSize */        sizeof(ShrinkwrapModifierData),
	/* type */              eModifierTypeType_OnlyDeform,
	/* flags */             eModifierTypeFlag_AcceptsMesh
							| eModifierTypeFlag_AcceptsCVs
							| eModifierTypeFlag_SupportsEditmode
							| eModifierTypeFlag_EnableInEditmode,

	/* copyData */          copyData,
	/* deformVerts */       deformVerts,
	/* deformMatrices */    0,
	/* deformVertsEM */     deformVertsEM,
	/* deformMatricesEM */  0,
	/* applyModifier */     0,
	/* applyModifierEM */   0,
	/* initData */          initData,
	/* requiredDataMask */  requiredDataMask,
	/* freeData */          0,
	/* isDisabled */        isDisabled,
	/* updateDepgraph */    updateDepgraph,
	/* dependsOnTime */     0,
	/* dependsOnNormals */	0,
	/* foreachObjectLink */ foreachObjectLink,
	/* foreachIDLink */     0,
};
