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

/** \file source/blender/freestyle/intern/python/BPy_Id.h
 *  \ingroup freestyle
 */

#ifndef FREESTYLE_PYTHON_ID_H
#define FREESTYLE_PYTHON_ID_H

#include <Python.h>
#include <iostream>
using namespace std;

#include "../system/Id.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////

extern PyTypeObject Id_Type;

#define BPy_Id_Check(v)	(  PyObject_IsInstance( (PyObject *) v, (PyObject *) &Id_Type)  )

/*---------------------------Python BPy_Id structure definition----------*/
typedef struct {
	PyObject_HEAD
	Id *id;
} BPy_Id;

/*---------------------------Python BPy_Id visible prototypes-----------*/

int Id_Init( PyObject *module );

///////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif /* FREESTYLE_PYTHON_ID_H */
