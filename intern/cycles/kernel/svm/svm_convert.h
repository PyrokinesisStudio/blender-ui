/*
 * Copyright 2011, Blender Foundation.
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
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

CCL_NAMESPACE_BEGIN

/* Conversion Nodes */

__device void svm_node_convert(ShaderData *sd, float *stack, uint type, uint from, uint to)
{
	switch(type) {
		case NODE_CONVERT_FV: {
			float f = stack_load_float(stack, from);
			stack_store_float3(stack, to, make_float3(f, f, f));
			break;
		}
		case NODE_CONVERT_CF: {
			float3 f = stack_load_float3(stack, from);
			float g = f.x*0.2126f + f.y*0.7152f + f.z*0.0722f;
			stack_store_float(stack, to, g);
			break;
		}
		case NODE_CONVERT_VF: {
			float3 f = stack_load_float3(stack, from);
			float g = (f.x + f.y + f.z)*(1.0f/3.0f);
			stack_store_float(stack, to, g);
			break;
		}

	}
}

CCL_NAMESPACE_END

