/*
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
 * General KX game object.
 */

#ifndef __KX_GAMEOBJECT
#define __KX_GAMEOBJECT


#ifdef WIN32
// get rid of this stupid "warning 'this' used in initialiser list", generated by VC when including Solid/Sumo
#pragma warning (disable : 4355) 
#endif 


#include "ListValue.h"
#include "SCA_IObject.h"
#include "SG_Node.h"
#include "MT_Transform.h"
#include "MT_CmMatrix4x4.h"
#include "GEN_Map.h"
#include "GEN_HashedPtr.h"

#define KX_FIXED_FRAME_PER_SEC 25.0f
#define KX_FIXED_SEC_PER_FRAME (1.0f / KX_FIXED_FRAME_PER_SEC)
#define KX_OB_DYNAMIC 1


//Forward declarations.
struct KX_ClientObjectInfo;
class RAS_MeshObject;
class KX_IPhysicsController;
class SM_Object;

class KX_GameObject : public SCA_IObject
{
	Py_Header;

	bool								m_bDyna;
	KX_ClientObjectInfo*				m_pClient_info;
	STR_String							m_name;
	STR_String							m_text;
	std::vector<RAS_MeshObject*>		m_meshes;
	
	bool								m_bSuspendDynamics;
	bool								m_bUseObjectColor;
	MT_Vector4							m_objectColor;

	// Is this object set to be visible? Only useful for the
	// visibility subsystem right now.
	bool       m_bVisible; 

	KX_IPhysicsController*				m_pPhysicsController1;
	SG_Node*							m_pSGNode;

protected:
	MT_CmMatrix4x4						m_OpenGL_4x4Matrix;
	
public:
	virtual void	/* This function should be virtual - derived classed override it */
	Relink(
		GEN_Map<GEN_HashedPtr, void*> *map
	);

	/**
	 * Compute an OpenGl compatable 4x4 matrix. Has the
	 * side effect of storing the result internally. The
	 * memory for the matrix remains the property of this class.
	 */ 
		double*						
	GetOpenGLMatrix(
	);

	/**
	 * Return a pointer to a MT_CmMatrix4x4 storing the 
	 * opengl transformation for this object. This is updated
	 * by a call to GetOpenGLMatrix(). This class owns the 
	 * memory for the returned matrix.
	 */

		MT_CmMatrix4x4*				
	GetOpenGLMatrixPtr(
	) { 
		return &m_OpenGL_4x4Matrix;
	};

	/** 
	 * Get a pointer to the game object that is the parent of 
	 * this object. Or NULL if there is no parent. The returned
	 * object is part of a reference counting scheme. Calling
	 * this function ups the reference count on the returned 
	 * object. It is the responsibility of the caller to decrement
	 * the reference count when you have finished with it.
	 */
		KX_GameObject*				
	GetParent(
	);


	/**
	 * Construct a game object. This class also inherits the 
	 * default constructors - use those with care!
	 */

	KX_GameObject(
		void* sgReplicationInfo,
		SG_Callbacks callbacks,
		PyTypeObject* T=&Type
	);

	virtual 
	~KX_GameObject(
	);

		CValue*				
	AddRef() { 
		/* temporarily to find memleaks */ return CValue::AddRef(); 
	}

	/** 
	 * @section Stuff which is here due to poor design.
	 * Inherited from CValue and needs an implementation. 
	 * Do not expect these functions do to anything sensible.
	 */

	/**
	 * Inherited from CValue -- does nothing!
	 */
		CValue*				
	Calc(
		VALUE_OPERATOR op,
		CValue *val
	);

	/**
	 * Inherited from CValue -- does nothing!
	 */
		CValue*				
	CalcFinal(
		VALUE_DATA_TYPE dtype,
		VALUE_OPERATOR op,
		CValue *val
	);

	/**
	 * Inherited from CValue -- does nothing!
	 */
	const 
		STR_String &	
	GetText(
	);

	/**
	 * Inherited from CValue -- does nothing!
	 */
		float				
	GetNumber(
	);

	/**
	 * @section Inherited from CValue. These are the useful
	 * part of the CValue interface that this class implements. 
	 */

	/**
	 * Inherited from CValue -- returns the name of this object.
	 */
		STR_String			
	GetName(
	);

	/**
	 * Inherited from CValue -- set the name of this object.
	 */
		void				
	SetName(
		STR_String name
	);

	/**
	 * Inherited from CValue -- does nothing.
	 */
		void				
	ReplicaSetName(
		STR_String name
	);

	/** 
	 * Inherited from CValue -- return a new copy of this
	 * instance allocated on the heap. Ownership of the new 
	 * object belongs with the caller.
	 */
		CValue*				
	GetReplica(
	);
	
	/**
	 * Inherited from CValue -- Makes sure any internal 
	 * data owned by this class is deep copied. Called internally
	 */
		void				
	ProcessReplica(
		KX_GameObject* replica
	);

	/** 
	 * Return the linear velocity of the game object.
	 */
		MT_Vector3			
	GetLinearVelocity(
	);

	/** 
	 * Quick'n'dirty obcolor ipo stuff
	 */

		void				
	SetObjectColor(
		const MT_Vector4& rgbavec
	);




	/**
	 * @return a pointer to the physics controller owned by this class.
	 */

		KX_IPhysicsController* 
	GetPhysicsController(
	) ;

	void	SetPhysicsController
		(KX_IPhysicsController*	physicscontroller) 
	{ m_pPhysicsController1 = physicscontroller;};


	/**
	 * @section Coordinate system manipulation functions
	 */

		void						
	NodeSetLocalPosition(
		const MT_Point3& trans
	);

		void						
	NodeSetLocalOrientation(
		const MT_Matrix3x3& rot
	);

		void						
	NodeSetLocalScale(
		const MT_Vector3& scale
	);

		void						
	NodeSetRelativeScale(
		const MT_Vector3& scale
	);

		void						
	NodeUpdateGS(
		double time,
		bool bInitiator
	);

	const 
		MT_Matrix3x3&			
	NodeGetWorldOrientation(
	) const;

	const 
		MT_Vector3&			
	NodeGetWorldScaling(
	) const;

	const 
		MT_Point3&			
	NodeGetWorldPosition(
	) const;


	/**
	 * @section scene graph node accessor functions.
	 */

		SG_Node*					
	GetSGNode(
	) { 
		return m_pSGNode;
	}

	const 
		SG_Node*				
	GetSGNode(
	) const	{ 
		return m_pSGNode;
	}

	/**
	 * Set the Scene graph node for this game object.
	 * warning - it is your responsibility to make sure
	 * all controllers look at this new node. You must
	 * also take care of the memory associated with the
	 * old node. This class takes ownership of the new
	 * node.
	 */
		void						
	SetSGNode(
		SG_Node* node
	){ 
		m_pSGNode = node; 
	}
	
		bool						
	IsDynamic(
	) const { 
		return m_bDyna; 
	}
	

	/**
	 * @section Physics accessors for this node.
	 *
	 * All these calls get passed directly to the physics controller 
	 * owned by this object.
	 * This is real interface bloat. Why not just use the physics controller
	 * directly? I think this is because the python interface is in the wrong
	 * place.
	 */

		void						
	ApplyForce(
		const MT_Vector3& force,	bool local
	);

		void						
	ApplyTorque(
		const MT_Vector3& torque,
		bool local
	);

		void						
	ApplyRotation(
		const MT_Vector3& drot,
		bool local
	);

		void						
	ApplyMovement(
		const MT_Vector3& dloc,
		bool local
	);

		void						
	addLinearVelocity(
		const MT_Vector3& lin_vel,
		bool local
	);

		void						
	setLinearVelocity(
		const MT_Vector3& lin_vel,
		bool local
	);

		void						
	setAngularVelocity(
		const MT_Vector3& ang_vel,
		bool local
	);

	/**	
	 * Update the physics object transform based upon the current SG_Node
	 * position.
	 */
		void						
	UpdateTransform(
	);

	/**
	 * Only update the transform if it's a non-dynamic object
	 */
		void 
	UpdateNonDynas(
	);

	/**
	 * Odd function to update an ipo. ???
	 */ 
		void	
	UpdateIPO(
		float curframetime,
		bool resurse, 
		bool ipo_as_force,
		bool force_ipo_local
	);

	/**
	 * @section Mesh accessor functions.
	 */
	
	/**	
	 * Run through the meshes associated with this
	 * object and bucketize them. See RAS_Mesh for
	 * more details on this function. Interesting to 
	 * note that polygon bucketizing seems to happen on a per
	 * object basis. Which may explain why there is such
	 * a big performance gain when all static objects
	 * are joined into 1.
	 */
		void						
	Bucketize(
	);

	/**
	 * Clear the meshes associated with this class
	 * and remove from the bucketing system.
	 * Don't think this actually deletes any of the meshes.
	 */
		void						
	RemoveMeshes(
	);

	/**
	 * Add a mesh to the set of meshes associated with this
	 * node. Meshes added in this way are not deleted by this class.
	 * Make sure you call RemoveMeshes() before deleting the
	 * mesh though,
	 */
		void						
	AddMesh(
		RAS_MeshObject* mesh
	){ 
		m_meshes.push_back(mesh);
	}

	/**
	 * Pick out a mesh associated with the integer 'num'.
	 */
		RAS_MeshObject*				
	GetMesh(
		int num
	) const { 
		return m_meshes[num]; 
	}

	/**
	 * Return the number of meshes currently associated with this
	 * game object.
	 */
		int							
	GetMeshCount(
	) const { 
		return m_meshes.size(); 
	}
	
	/**	
	 * Set the debug color of the meshes associated with this
	 * class. Does this still work?
	 */
		void						
	SetDebugColor(
		unsigned int bgra
	);

	/** 
	 * Reset the debug color of meshes associated with this class.
	 */
		void						
	ResetDebugColor(
	);

	/** 
	 * Set the visibility of the meshes associated with this
	 * object.
	 */
		void						
	MarkVisible(
		bool visible
	);

	/** 
	 * Set the visibility according to the visibility flag.
	 */
		void						
	MarkVisible(
		void
	);

		
	/**
	 * Was this object marked visible? (only for the ewxplicit
	 * visibility system).
	 */
		bool
	GetVisible(
		void
	);

	/**
	 * Set visibility flag of this object
	 */
		void
	SetVisible(
		bool b
	);

		
	/**
	 * @section Logic bubbling methods.
	 */

	/**
	 * Stop making progress
	 */
	void Suspend(void);

	/**
	 * Resume making progress
	 */
	void Resume(void);
	
	/**
	 * @section Python interface functions.
	 */

	virtual 
		PyObject*			
	_getattr(
		char *attr
	);

		PyObject*					
	PySetPosition(
		PyObject* self,
		PyObject* args,
		PyObject* kwds
	);

	static 
		PyObject*			
	sPySetPosition(
		PyObject* self,
		PyObject* args,
		PyObject* kwds
	);
	
	KX_PYMETHOD(KX_GameObject,GetPosition);
	KX_PYMETHOD(KX_GameObject,GetLinearVelocity);
	KX_PYMETHOD(KX_GameObject,GetVelocity);
	KX_PYMETHOD(KX_GameObject,GetMass);
	KX_PYMETHOD(KX_GameObject,GetReactionForce);
	KX_PYMETHOD(KX_GameObject,GetOrientation);
	KX_PYMETHOD(KX_GameObject,SetOrientation);
	KX_PYMETHOD(KX_GameObject,SetVisible);
	KX_PYMETHOD(KX_GameObject,SuspendDynamics);
	KX_PYMETHOD(KX_GameObject,RestoreDynamics);
	KX_PYMETHOD(KX_GameObject,EnableRigidBody);
	KX_PYMETHOD(KX_GameObject,DisableRigidBody);
	KX_PYMETHOD(KX_GameObject,ApplyImpulse);
	KX_PYMETHOD(KX_GameObject,GetMesh);
	KX_PYMETHOD(KX_GameObject,GetParent);
	KX_PYMETHOD(KX_GameObject,GetPhysicsId);

private :

	/**	
	 * Random internal function to convert python function arguments
	 * to 2 vectors.
	 * @return true if conversion was possible.
	 */

		bool						
	ConvertPythonVectorArgs(
		PyObject* args,
		MT_Vector3& pos,
		MT_Vector3& pos2
	);	

};

#endif //__KX_GAMEOBJECT

