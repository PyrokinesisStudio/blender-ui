#ifndef SM_OBJECT_H
#define SM_OBJECT_H

#include <vector>

#include "solid.h"

#include "SM_Callback.h"
#include "SM_MotionState.h"
#include <stdio.h>


class SM_FhObject;


// Properties of dynamic objects
struct SM_ShapeProps {
	MT_Scalar  m_mass;                  // Total mass
	MT_Scalar  m_inertia;               // Inertia, should be a tensor some time 
	MT_Scalar  m_lin_drag;              // Linear drag (air, water) 0 = concrete, 1 = vacuum 
	MT_Scalar  m_ang_drag;              // Angular drag
	MT_Scalar  m_friction_scaling[3];   // Scaling for anisotropic friction. Component in range [0, 1]   
	bool       m_do_anisotropic;        // Should I do anisotropic friction? 
	bool       m_do_fh;                 // Should the object have a linear Fh spring?
	bool       m_do_rot_fh;             // Should the object have an angular Fh spring?
};


// Properties of collidable objects (non-ghost objects)
struct SM_MaterialProps {
	MT_Scalar m_restitution;           // restitution of energie after a collision 0 = inelastic, 1 = elastic
	MT_Scalar m_friction;              // Coulomb friction (= ratio between the normal en maximum friction force)
	MT_Scalar m_fh_spring;             // Spring constant (both linear and angular)
	MT_Scalar m_fh_damping;            // Damping factor (linear and angular) in range [0, 1]
	MT_Scalar m_fh_distance;           // The range above the surface where Fh is active.    
	bool      m_fh_normal;             // Should the object slide off slopes?
};


class SM_Object : public SM_MotionState {
public:
    SM_Object() ;
    SM_Object(
		DT_ShapeHandle shape, 
		const SM_MaterialProps *materialProps,
		const SM_ShapeProps *shapeProps,
		SM_Object *dynamicParent
	);
	
    virtual ~SM_Object();

	bool isDynamic() const;  

	/* nzc experimental. There seem to be two places where kinematics
	 * are evaluated: proceedKinematic (called from SM_Scene) and
	 * proceed() in this object. I'll just try and bunge these out for
	 * now.  */

	void suspend(void);
	void resume(void);

	void suspendDynamics();
	
	void restoreDynamics();
	
	bool isGhost() const;

	void suspendMaterial();
	
	void restoreMaterial();
	
	SM_FhObject *getFhObject() const;
	
	void registerCallback(SM_Callback& callback);

	void calcXform();
	void notifyClient();
    
	// Save the current state information for use in the 
	// velocity computation in the next frame.  

	void proceedKinematic(MT_Scalar timeStep);

	void saveReactionForce(MT_Scalar timeStep) ;
	
	void clearForce() ;

    void clearMomentum() ;

    void setMargin(MT_Scalar margin) ;

    MT_Scalar getMargin() const ;

    const SM_MaterialProps *getMaterialProps() const ;

	const SM_ShapeProps *getShapeProps() const ;

    void setPosition(const MT_Point3& pos);
    void setOrientation(const MT_Quaternion& orn);
    void setScaling(const MT_Vector3& scaling);


	/**
	 * set an external velocity. This velocity complements
	 * the physics velocity. So setting it does not override the
	 * physics velocity. It is your responsibility to clear 
	 * this external velocity. This velocity is not subject to 
	 * friction or damping.
	 */


    void setExternalLinearVelocity(const MT_Vector3& lin_vel) ;
	void addExternalLinearVelocity(const MT_Vector3& lin_vel) ;

	/** Override the physics velocity */

    void addLinearVelocity(const MT_Vector3& lin_vel);
	void setLinearVelocity(const MT_Vector3& lin_vel);

	/**
	 * Set an external angular velocity. This velocity complemetns
	 * the physics angular velocity so does not override it. It is
	 * your responsibility to clear this velocity. This velocity
	 * is not subject to friction or damping.
	 */

    void setExternalAngularVelocity(const MT_Vector3& ang_vel) ;
    void addExternalAngularVelocity(const MT_Vector3& ang_vel);

	/** Override the physics angular velocity */

	void addAngularVelocity(const MT_Vector3& ang_vel);
	void setAngularVelocity(const MT_Vector3& ang_vel);

	/** Clear the external velocities */

	void clearCombinedVelocities();

	/** 
	 * Tell the physics system to combine the external velocity
	 * with the physics velocity. 
	 */

	void resolveCombinedVelocities(
		const MT_Vector3 & lin_vel,
		const MT_Vector3 & ang_vel
	) ;



	MT_Scalar getInvMass() const;

	MT_Scalar getInvInertia() const ;

    void applyForceField(const MT_Vector3& accel) ;

    void applyCenterForce(const MT_Vector3& force) ;

    void applyTorque(const MT_Vector3& torque) ;

    void applyImpulse(const MT_Point3& attach, const MT_Vector3& impulse) ;

    void applyCenterImpulse(const MT_Vector3& impulse);

    void applyAngularImpulse(const MT_Vector3& impulse);

    MT_Point3 getWorldCoord(const MT_Point3& local) const;

	MT_Vector3 getVelocity(const MT_Point3& local) const;


	const MT_Vector3& getReactionForce() const ;

	void getMatrix(double *m) const ;

	const double *getMatrix() const ;

	// Still need this???
	const MT_Transform&  getScaledTransform()  const; 

	DT_ObjectHandle getObjectHandle() const ;
	DT_ShapeHandle getShapeHandle() const ;

	void  setClientObject(void *clientobj) ;
	void *getClientObject()	;

	SM_Object *getDynamicParent() ;

	void integrateForces(MT_Scalar timeStep);
	void integrateMomentum(MT_Scalar timeSteo);

	void setRigidBody(bool is_rigid_body) ;

	bool isRigidBody() const ;


	// This is the callback for handling collisions of dynamic objects
	static 
		void 
	boing(
		void *client_data,  
		void *object1,
		void *object2,
		const DT_CollData *coll_data
	);

private:

	// return the actual linear_velocity of this object this 
	// is the addition of m_combined_lin_vel and m_lin_vel.

	const 
		MT_Vector3
	actualLinVelocity(
	) const ;

	const 
		MT_Vector3
	actualAngVelocity(
	) const ;

	typedef std::vector<SM_Callback *> T_CallbackList;


    T_CallbackList          m_callbackList;    // Each object can have multiple callbacks from the client (=game engine)
	SM_Object              *m_dynamicParent;   // Collisions between parent and children are ignored

    // as the collision callback now has only information
	// on an SM_Object, there must be a way that the SM_Object client
	// can identify it's clientdata after a collision
	void                   *m_client_object;

    DT_ShapeHandle          m_shape;                 // Shape for collision detection

	// Material and shape properties are not owned by this class.

	const SM_MaterialProps *m_materialProps;         
	const SM_MaterialProps *m_materialPropsBackup;   // Backup in case the object temporarily becomes a ghost.
	const SM_ShapeProps    *m_shapeProps;           
	const SM_ShapeProps    *m_shapePropsBackup;      // Backup in case the object's dynamics is temporarily suspended
    DT_ObjectHandle         m_object;                // A handle to the corresponding object in SOLID.
	MT_Scalar               m_margin;                // Offset for the object's shape (also for collision detection)
	MT_Vector3              m_scaling;               // Non-uniform scaling of the object's shape

	double                  m_ogl_matrix[16];        // An OpenGL-type 4x4 matrix      
	MT_Transform            m_xform;                 // The object's local coordinate system
	MT_Transform            m_prev_xform;            // The object's local coordinate system in the previous frame
	SM_MotionState          m_prev_state;            // The object's motion state in the previous frame
	MT_Scalar               m_timeStep;              // The duration of the last frame 

	MT_Vector3              m_reaction_impulse;      // The accumulated impulse resulting from collisions
	MT_Vector3              m_reaction_force;        // The reaction force derived from the reaction impulse   

	unsigned int            m_kinematic      : 1;    // Have I been displaced (translated, rotated, scaled) in this frame? 
	unsigned int            m_prev_kinematic : 1;    // Have I been displaced (translated, rotated, scaled) in the previous frame? 
	unsigned int            m_is_rigid_body  : 1;    // Should friction give me a change in angular momentum?

	MT_Vector3              m_lin_mom;               // Linear momentum (linear velocity times mass)
	MT_Vector3              m_ang_mom;               // Angular momentum (angualr velocity times inertia)
	MT_Vector3              m_force;                 // Force on center of mass (afffects linear momentum)
	MT_Vector3              m_torque;                // Torque around center of mass (affects angualr momentum)

	// Here are the values of externally set linear and angular
	// velocity. These are updated from the outside
	// (actuators and python) each frame and combined with the
	// physics values. At the end of each frame (at the end of a
	// call to proceed) they are set to zero. This allows the
	// outside world to contribute to the velocity of an object
	// but still have it react to physics. 

	MT_Vector3				m_combined_lin_vel;
	MT_Vector3				m_combined_ang_vel;

	// The force and torque are the accumulated forces and torques applied by the client (game logic, python).

	SM_FhObject            *m_fh_object;             // The ray object used for Fh
	bool                    m_suspended;             // Is this object frozen?
};

#endif
