/**
* $Id:
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
 * The Original Code is Copyright (C) 2006 Blender Foundation
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

struct ShadeInput;
struct ShadeResult;
struct RenderPart;
struct RenderLayer;
struct PixStr;

/* shadeinput.c */


/* needed to calculate shadow and AO for an entire pixel */
typedef struct ShadeSample {
	int tot;				/* amount of shi in use, can be 1 for not FULL_OSA */
	ShadeInput shi[16];		/* RE_MAX_OSA */
	ShadeResult shr[16];	/* RE_MAX_OSA */
} ShadeSample;


	/* also the node shader callback */
void shade_material_loop(struct ShadeInput *shi, struct ShadeResult *shr);

void shade_input_set_triangle_i(struct ShadeInput *shi, struct VlakRen *vlr, short i1, short i2, short i3);
void shade_input_set_triangle(struct ShadeInput *shi, volatile int facenr, int normal_flip);
void shade_input_copy_triangle(struct ShadeInput *shi, struct ShadeInput *from);
void shade_input_set_viewco(struct ShadeInput *shi, float x, float y, float z);
void shade_input_set_uv(struct ShadeInput *shi);
void shade_input_set_normals(struct ShadeInput *shi);
void shade_input_set_shade_texco(struct ShadeInput *shi);
void shade_input_do_shade(struct ShadeInput *shi, struct ShadeResult *shr);

void shade_sample_initialize(struct ShadeSample *ssamp, struct RenderPart *pa, struct RenderLayer *rl);
void shade_samples_do_shadow(struct ShadeSample *ssamp);
int shade_samples(struct ShadeSample *ssamp, struct PixStr *ps, int x, int y);

void vlr_set_uv_indices(struct VlakRen *vlr, int *i1, int *i2, int *i3);

void	calc_R_ref(struct ShadeInput *shi);


/* shadeoutput. */
void shade_lamp_loop(struct ShadeInput *shi, struct ShadeResult *shr);

void shade_color(struct ShadeInput *shi, ShadeResult *shr);

void ambient_occlusion_to_diffuse(struct ShadeInput *shi, float *diff);
void ambient_occlusion(struct ShadeInput *shi);

float lamp_get_visibility(struct LampRen *lar, float *co, float *lv, float *dist);
void lamp_get_shadow(struct LampRen *lar, ShadeInput *shi, float inp, float *shadfac, int do_real);

float	fresnel_fac(float *view, float *vn, float fresnel, float fac);
