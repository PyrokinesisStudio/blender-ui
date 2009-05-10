/**
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The Original Code is Copyright (C) 2009 Blender Foundation.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): André Pinto.
 *
 * ***** END GPL LICENSE BLOCK *****
 */
#ifndef RE_RAYOBJECT_H
#define RE_RAYOBJECT_H

#include "RE_raytrace.h"
#include <float.h>

/* RayObject
	
	A ray object is everything where we can cast rays like:
		* a face/triangle
		* an octree
		* a bvh tree
		* an octree of bvh's
		* a bvh of bvh's
	
		
	All types of RayObjects can be created by implementing the
	callbacks of the RayObject.

	Due to high computing time evolved with casting on faces
	there is a special type of RayObject (named RayFace)
	which won't use callbacks like other generic nodes.
	
	In order to allow a mixture of RayFace+RayObjects,
	all RayObjects must be 4byte aligned, allowing us to use the
	2 least significant bits (with the mask 0x02) to define the
	type of RayObject.
	
	This leads to 4 possible types of RayObject, but at the moment
	only 2 are used:

	 addr&2  - type of object
		0     	RayFace
		1		RayObject (generic with API callbacks)
		2		unused
		3		unused

	0 was choosed to RayFace because thats the one where speed will be needed.
	
	You actually don't need to care about this if you are only using the API
	described on RE_raytrace.h
 */
 
typedef struct RayFace
{
	float *v1, *v2, *v3, *v4;
	
	void *ob;
	void *face;
	
} RayFace;

struct RayObject
{
	struct RayObjectAPI *api;
	
};

typedef int  (*RayObject_raycast_callback)(RayObject *, Isect *);
typedef void (*RayObject_add_callback)(RayObject *, RayObject *);
typedef void (*RayObject_done_callback)(RayObject *);
typedef void (*RayObject_free_callback)(RayObject *);
typedef void (*RayObject_bb_callback)(RayObject *, float *min, float *max);

typedef struct RayObjectAPI
{
	RayObject_raycast_callback	raycast;
	RayObject_add_callback		add;
	RayObject_done_callback		done;
	RayObject_free_callback		free;
	RayObject_bb_callback		bb;
	
} RayObjectAPI;

//TODO use intptr_t
#define RayObject_align(o)		((RayObject*)(((int)o)&(~3)))
#define RayObject_unalign(o)	((RayObject*)(((int)o)|1))
#define RayObject_isFace(o)		((((int)o)&3) == 0)

/*
 * Extend min/max coords so that the rayobject is inside them
 */
void RayObject_merge_bb(RayObject *ob, float *min, float *max);

/*
 * This function differs from RayObject_raycast
 * RayObject_intersect does NOT perform last-hit optimization
 * So this is probably a function to call inside raytrace structures
 */
int RayObject_intersect(RayObject *r, Isect *i);

#define ISECT_EPSILON ((float)FLT_EPSILON)

#endif
