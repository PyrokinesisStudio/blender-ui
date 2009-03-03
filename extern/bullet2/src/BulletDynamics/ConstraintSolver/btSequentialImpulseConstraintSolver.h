/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef SEQUENTIAL_IMPULSE_CONSTRAINT_SOLVER_H
#define SEQUENTIAL_IMPULSE_CONSTRAINT_SOLVER_H

#include "btConstraintSolver.h"
class btIDebugDraw;
#include "btContactConstraint.h"
#include "btSolverBody.h"
#include "btSolverConstraint.h"



///The btSequentialImpulseConstraintSolver uses a Propagation Method and Sequentially applies impulses
///The approach is the 3D version of Erin Catto's GDC 2006 tutorial. See http://www.gphysics.com
///Although Sequential Impulse is more intuitive, it is mathematically equivalent to Projected Successive Overrelaxation (iterative LCP)
///Applies impulses for combined restitution and penetration recovery and to simulate friction
class btSequentialImpulseConstraintSolver : public btConstraintSolver
{
protected:

	btAlignedObjectArray<btSolverBody>	m_tmpSolverBodyPool;
	btConstraintArray			m_tmpSolverContactConstraintPool;
	btConstraintArray			m_tmpSolverNonContactConstraintPool;
	btConstraintArray			m_tmpSolverContactFrictionConstraintPool;
	btAlignedObjectArray<int>	m_orderTmpConstraintPool;
	btAlignedObjectArray<int>	m_orderFrictionConstraintPool;

	btSolverConstraint&	addFrictionConstraint(const btVector3& normalAxis,int solverBodyIdA,int solverBodyIdB,int frictionIndex,btManifoldPoint& cp,const btVector3& rel_pos1,const btVector3& rel_pos2,btCollisionObject* colObj0,btCollisionObject* colObj1, btScalar relaxation);
	
	///m_btSeed2 is used for re-arranging the constraint rows. improves convergence/quality of friction
	unsigned long	m_btSeed2;

	void	initSolverBody(btSolverBody* solverBody, btCollisionObject* collisionObject);
	btScalar restitutionCurve(btScalar rel_vel, btScalar restitution);

	void	convertContact(btPersistentManifold* manifold,const btContactSolverInfo& infoGlobal);

	void	resolveSplitPenetrationImpulseCacheFriendly(
        btSolverBody& body1,
        btSolverBody& body2,
        const btSolverConstraint& contactConstraint,
        const btContactSolverInfo& solverInfo);

	//internal method
	int	getOrInitSolverBody(btCollisionObject& body);

	void	resolveSingleConstraintRowGeneric(btSolverBody& body1,btSolverBody& body2,const btSolverConstraint& contactConstraint);

	void	resolveSingleConstraintRowGenericSIMD(btSolverBody& body1,btSolverBody& body2,const btSolverConstraint& contactConstraint);
	
	void	resolveSingleConstraintRowLowerLimit(btSolverBody& body1,btSolverBody& body2,const btSolverConstraint& contactConstraint);
	
	void	resolveSingleConstraintRowLowerLimitSIMD(btSolverBody& body1,btSolverBody& body2,const btSolverConstraint& contactConstraint);
		
public:

	
	btSequentialImpulseConstraintSolver();
	virtual ~btSequentialImpulseConstraintSolver();

	virtual btScalar solveGroup(btCollisionObject** bodies,int numBodies,btPersistentManifold** manifold,int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& info, btIDebugDraw* debugDrawer, btStackAlloc* stackAlloc,btDispatcher* dispatcher);
	
	btScalar solveGroupCacheFriendlySetup(btCollisionObject** bodies,int numBodies,btPersistentManifold** manifoldPtr, int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal,btIDebugDraw* debugDrawer,btStackAlloc* stackAlloc);
	btScalar solveGroupCacheFriendlyIterations(btCollisionObject** bodies,int numBodies,btPersistentManifold** manifoldPtr, int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal,btIDebugDraw* debugDrawer,btStackAlloc* stackAlloc);

	///clear internal cached data and reset random seed
	virtual	void	reset();
	
	unsigned long btRand2();

	int btRandInt2 (int n);

	void	setRandSeed(unsigned long seed)
	{
		m_btSeed2 = seed;
	}
	unsigned long	getRandSeed() const
	{
		return m_btSeed2;
	}

};

#ifndef BT_PREFER_SIMD
typedef btSequentialImpulseConstraintSolver btSequentialImpulseConstraintSolverPrefered;
#endif


#endif //SEQUENTIAL_IMPULSE_CONSTRAINT_SOLVER_H

