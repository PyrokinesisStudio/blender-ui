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

/** \file source/blender/freestyle/intern/python/UnaryPredicate0D/BPy_TrueUP0D.h
 *  \ingroup freestyle
 */

#ifndef __FREESTYLE_PYTHON_TRUEUP0D_H__
#define __FREESTYLE_PYTHON_TRUEUP0D_H__

#include "../BPy_UnaryPredicate0D.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////

#include <Python.h>

extern PyTypeObject TrueUP0D_Type;

#define BPy_TrueUP0D_Check(v) (PyObject_IsInstance((PyObject *)v, (PyObject *)&TrueUP0D_Type))

/*---------------------------Python BPy_TrueUP0D structure definition----------*/
typedef struct {
	BPy_UnaryPredicate0D py_up0D;
} BPy_TrueUP0D;

///////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif /* __FREESTYLE_PYTHON_TRUEUP0D_H__ */
