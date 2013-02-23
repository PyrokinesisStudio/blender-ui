/*
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
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2010 Blender Foundation.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file source/blender/freestyle/intern/python/Interface0D/BPy_SVertex.h
 *  \ingroup freestyle
 */

#ifndef __FREESTYLE_PYTHON_SVERTEX_H__
#define __FREESTYLE_PYTHON_SVERTEX_H__

#include "../../view_map/Silhouette.h"
#include "../BPy_Interface0D.h"


#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////

#include <Python.h>

extern PyTypeObject SVertex_Type;

#define BPy_SVertex_Check(v) (PyObject_IsInstance((PyObject *)v, (PyObject *)&SVertex_Type))

/*---------------------------Python BPy_SVertex structure definition----------*/
typedef struct {
	BPy_Interface0D py_if0D;
	SVertex *sv;
} BPy_SVertex;

/*---------------------------Python BPy_SVertex visible prototypes-----------*/

void SVertex_mathutils_register_callback();

///////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif /* __FREESTYLE_PYTHON_SVERTEX_H__ */
