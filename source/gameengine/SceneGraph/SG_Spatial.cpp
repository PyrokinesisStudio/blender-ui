/**
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

#include "SG_Node.h"
#include "SG_Spatial.h"
#include "SG_Controller.h"
#include "SG_ParentRelation.h"

SG_Spatial::
SG_Spatial(
	void* clientobj,
	void* clientinfo,
	SG_Callbacks callbacks
): 

	SG_IObject(clientobj,clientinfo,callbacks),
	m_localPosition(MT_Point3(0,0,0)),
	m_localScaling(MT_Vector3(1.f,1.f,1.f)),
	m_localRotation(1,0,0,0,1,0,0,0,1),
	m_parent_relation (NULL),

	m_worldPosition(MT_Point3(0,0,0)),
	m_worldScaling(MT_Vector3(1.f,1.f,1.f)),
	m_worldRotation(0,0,0,0,0,0,0,0,0)

{
}

SG_Spatial::
SG_Spatial(
	const SG_Spatial& other
) : 
	SG_IObject(other),
	m_localPosition(other.m_localPosition),
	m_localScaling(other.m_localScaling),
	m_localRotation(other.m_localRotation),
	m_parent_relation(NULL),
	m_worldPosition(other.m_worldPosition),
	m_worldScaling(other.m_worldScaling),
	m_worldRotation(other.m_worldRotation)
{
	// duplicate the parent relation for this object
	m_parent_relation = other.m_parent_relation->NewCopy();
}
	
SG_Spatial::
~SG_Spatial()
{
	delete (m_parent_relation);
}

	void
SG_Spatial::
SetParentRelation(
	SG_ParentRelation *relation
){
	delete (m_parent_relation);
	m_parent_relation = relation;
}


/**
 * Update Spatial Data.
 * Calculates WorldTransform., (either doing itsself or using the linked SGControllers)
 */


	void 
SG_Spatial::
UpdateSpatialData(
	const SG_Spatial *parent,
	double time
){

    bool bComputesWorldTransform = false;

	// update spatial controllers
	
	SGControllerList::iterator cit = GetSGControllerList().begin();
	SGControllerList::const_iterator c_end = GetSGControllerList().end();

	for (;cit!=c_end;++cit)
	{
		bComputesWorldTransform = bComputesWorldTransform || (*cit)->Update(time);
	}

	// If none of the objects updated our values then we ask the
	// parent_relation object owned by this class to update 
	// our world coordinates.

	if (!bComputesWorldTransform)
    {
		ComputeWorldTransforms(parent);
    }
}

void	SG_Spatial::ComputeWorldTransforms(const SG_Spatial *parent)
{
	m_parent_relation->UpdateChildCoordinates(this,parent);
}


/**
 * Position and translation methods
 */


	void 
SG_Spatial::
RelativeTranslate(
	const MT_Vector3& trans,
	const SG_Spatial *parent,
	bool local
){
	if (local) {
			m_localPosition += m_localRotation * trans;
	} else {
		if (parent) {
			m_localPosition += trans * parent->GetWorldOrientation();
		} else {
			m_localPosition += trans;
		}
	}
}	
	
	void 
SG_Spatial::
SetLocalPosition(
	const MT_Point3& trans
){
	m_localPosition = trans;
}

	void				
SG_Spatial::
SetWorldPosition(
	const MT_Point3& trans
) {
	m_worldPosition = trans;
}

/**
 * Scaling methods.
 */ 

	void 
SG_Spatial::
RelativeScale(
	const MT_Vector3& scale
){
	m_localScaling = m_localScaling * scale;
}

	void 
SG_Spatial::
SetLocalScale(
	const MT_Vector3& scale
){
	m_localScaling = scale;
}


	void				
SG_Spatial::
SetWorldScale(
	const MT_Vector3& scale
){ 
	m_worldScaling = scale;
}

/**
 * Orientation and rotation methods.
 */


	void 
SG_Spatial::
RelativeRotate(
	const MT_Matrix3x3& rot,
	bool local
){
	m_localRotation = m_localRotation * (
	local ? 
		rot 
	:
	(GetWorldOrientation().inverse() * rot * GetWorldOrientation()));
}

	void 
SG_Spatial::
SetLocalOrientation(const MT_Matrix3x3& rot)
{
	m_localRotation = rot;
}



	void				
SG_Spatial::
SetWorldOrientation(
	const MT_Matrix3x3& rot
) {
	m_worldRotation = rot;
}

const 
	MT_Point3&
SG_Spatial::
GetLocalPosition(
) const	{
	 return m_localPosition;
}

const 
	MT_Matrix3x3&
SG_Spatial::
GetLocalOrientation(
) const	{
	return m_localRotation;
}

const 
	MT_Vector3&	
SG_Spatial::
GetLocalScale(
) const{
	return m_localScaling;
}


const 
	MT_Point3&
SG_Spatial::
GetWorldPosition(
) const	{
	return m_worldPosition;
}

const 
	MT_Matrix3x3&	
SG_Spatial::
GetWorldOrientation(
) const	{
	return m_worldRotation;
}

const 
	MT_Vector3&	
SG_Spatial::
GetWorldScaling(
) const	{
	return m_worldScaling;
}

