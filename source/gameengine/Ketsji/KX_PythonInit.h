/**
 * $Id$
 *
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. The Blender
 * Foundation also sells licenses for use in proprietary software under
 * the Blender License.  See http://www.blender.org/BL/ for information
 * about this.
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
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */
#ifndef __KX_PYTHON_INIT
#define __KX_PYTHON_INIT

#include "KX_Python.h"
#include "STR_String.h"

typedef enum {
	psl_Lowest = 0,
	psl_Highest
} TPythonSecurityLevel;

extern bool gUseVisibilityTemp;


PyObject*	initGameLogic(class KX_Scene* ketsjiscene);
PyObject*	initGameKeys();
PyObject*	initRasterizer(class RAS_IRasterizer* rasty,class RAS_ICanvas* canvas);
PyObject*	initGamePythonScripting(const STR_String& progname, TPythonSecurityLevel level);
void		exitGamePythonScripting();
void		exitGamePythonScripting();
void PHY_SetActiveScene(class KX_Scene* scene);

#endif //__KX_PYTHON_INIT

