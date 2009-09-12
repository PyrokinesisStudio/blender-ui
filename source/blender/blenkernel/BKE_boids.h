/* BKE_particle.h
 *
 *
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
 * The Original Code is Copyright (C) 2009 by Janne Karhu.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#ifndef BKE_BOIDS_H
#define BKE_BOIDS_H

#include "DNA_boid_types.h"

typedef struct BoidBrainData {
	Scene *scene;
	struct Object *ob;
	struct ParticleSystem *psys;
	struct ParticleSettings *part;
	float timestep, cfra, dfra;
	float wanted_co[3], wanted_speed;

	/* Goal stuff */
	struct Object *goal_ob;
	float goal_co[3];
	float goal_nor[3];
	float goal_priority;
} BoidBrainData;

void boids_precalc_rules(struct ParticleSettings *part, float cfra);
void boid_brain(BoidBrainData *bbd, int p, struct ParticleData *pa);
void boid_body(BoidBrainData *bbd, struct ParticleData *pa);
void boid_default_settings(BoidSettings *boids);
BoidRule *boid_new_rule(int type);
BoidState *boid_new_state(BoidSettings *boids);
BoidState *boid_duplicate_state(BoidSettings *boids, BoidState *state);
void boid_free_settings(BoidSettings *boids);
BoidSettings *boid_copy_settings(BoidSettings *boids);
BoidState *boid_get_current_state(BoidSettings *boids);
#endif
