/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2007 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
///btDbvtBroadphase implementation by Nathanael Presson
#ifndef BT_DBVT_BROADPHASE_H
#define BT_DBVT_BROADPHASE_H

#include "BulletCollision/BroadphaseCollision/btDbvt.h"
#include "BulletCollision/BroadphaseCollision/btOverlappingPairCache.h"

//
// Compile time config
//

#define	DBVT_BP_PROFILE					0
#define DBVT_BP_SORTPAIRS				1
#define DBVT_BP_PREVENTFALSEUPDATE		0
#define DBVT_BP_ACCURATESLEEPING		0
#define DBVT_BP_ENABLE_BENCHMARK		0
#define DBVT_BP_MARGIN					(btScalar)0.05

#if DBVT_BP_PROFILE
	#define	DBVT_BP_PROFILING_RATE	256
	#include "LinearMath/btQuickprof.h"
#endif

//
// btDbvtProxy
//
struct btDbvtProxy : btBroadphaseProxy
{
/* Fields		*/ 
btDbvtAabbMm	aabb;
btDbvtNode*		leaf;
btDbvtProxy*	links[2];
int				stage;
/* ctor			*/ 
btDbvtProxy(void* userPtr,short int collisionFilterGroup, short int collisionFilterMask) :
	btBroadphaseProxy(userPtr,collisionFilterGroup,collisionFilterMask)
	{
	links[0]=links[1]=0;
	}
};

typedef btAlignedObjectArray<btDbvtProxy*>	btDbvtProxyArray;

///The btDbvtBroadphase implements a broadphase using two dynamic AABB bounding volume hierarchies/trees (see btDbvt).
///One tree is used for static/non-moving objects, and another tree is used for dynamic objects. Objects can move from one tree to the other.
///This is a very fast broadphase, especially for very dynamic worlds where many objects are moving. Its insert/add and remove of objects is generally faster than the sweep and prune broadphases btAxisSweep3 and bt32BitAxisSweep3.
struct	btDbvtBroadphase : btBroadphaseInterface
{
/* Config		*/ 
enum	{
		DYNAMIC_SET			=	0,	/* Dynamic set index	*/ 
		FIXED_SET			=	1,	/* Fixed set index		*/ 
		STAGECOUNT			=	2	/* Number of stages		*/ 
		};
/* Fields		*/ 
btDbvt					m_sets[2];					// Dbvt sets
btDbvtProxy*			m_stageRoots[STAGECOUNT+1];	// Stages list
btOverlappingPairCache*	m_paircache;				// Pair cache
btScalar				m_prediction;				// Velocity prediction
int						m_stageCurrent;				// Current stage
int						m_fupdates;					// % of fixed updates per frame
int						m_dupdates;					// % of dynamic updates per frame
int						m_cupdates;					// % of cleanup updates per frame
int						m_newpairs;					// Number of pairs created
int						m_fixedleft;				// Fixed optimization left
unsigned				m_updates_call;				// Number of updates call
unsigned				m_updates_done;				// Number of updates done
btScalar				m_updates_ratio;			// m_updates_done/m_updates_call
int						m_pid;						// Parse id
int						m_cid;						// Cleanup index
int						m_gid;						// Gen id
bool					m_releasepaircache;			// Release pair cache on delete
bool					m_deferedcollide;			// Defere dynamic/static collision to collide call
bool					m_needcleanup;				// Need to run cleanup?
bool					m_initialize;				// Initialization
#if DBVT_BP_PROFILE
btClock					m_clock;
struct	{
		unsigned long		m_total;
		unsigned long		m_ddcollide;
		unsigned long		m_fdcollide;
		unsigned long		m_cleanup;
		unsigned long		m_jobcount;
		}				m_profiling;
#endif
/* Methods		*/ 
btDbvtBroadphase(btOverlappingPairCache* paircache=0);
~btDbvtBroadphase();
void							collide(btDispatcher* dispatcher);
void							optimize();
/* btBroadphaseInterface Implementation	*/ 
btBroadphaseProxy*				createProxy(const btVector3& aabbMin,const btVector3& aabbMax,int shapeType,void* userPtr,short int collisionFilterGroup,short int collisionFilterMask,btDispatcher* dispatcher,void* multiSapProxy);
void							destroyProxy(btBroadphaseProxy* proxy,btDispatcher* dispatcher);
void							setAabb(btBroadphaseProxy* proxy,const btVector3& aabbMin,const btVector3& aabbMax,btDispatcher* dispatcher);
void							calculateOverlappingPairs(btDispatcher* dispatcher);
btOverlappingPairCache*			getOverlappingPairCache();
const btOverlappingPairCache*	getOverlappingPairCache() const;
void							getBroadphaseAabb(btVector3& aabbMin,btVector3& aabbMax) const;
void							printStats();
static void						benchmark(btBroadphaseInterface*);
};

#endif
