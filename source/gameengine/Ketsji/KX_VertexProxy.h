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
#ifndef __KX_VERTEXPROXY
#define __KX_VERTEXPROXY

#include "SCA_IObject.h"

class KX_VertexProxy	: public SCA_IObject
{
	Py_Header;
protected:

	class RAS_TexVert*	m_vertex;
public:
	KX_VertexProxy(class RAS_TexVert* vertex);
	virtual ~KX_VertexProxy();

	// stuff for cvalue related things
	CValue*		Calc(VALUE_OPERATOR op, CValue *val) ;
	CValue*		CalcFinal(VALUE_DATA_TYPE dtype, VALUE_OPERATOR op, CValue *val);
	const STR_String &	GetText();
	float		GetNumber();
	STR_String	GetName();
	void		SetName(STR_String name);								// Set the name of the value
	void		ReplicaSetName(STR_String name);
	CValue*		GetReplica();


// stuff for python integration
	virtual PyObject* _getattr(const STR_String& attr);
	virtual int    _setattr(const STR_String& attr, PyObject *pyvalue);

	KX_PYMETHOD(KX_VertexProxy,GetXYZ);
	KX_PYMETHOD(KX_VertexProxy,SetXYZ);
	KX_PYMETHOD(KX_VertexProxy,GetUV);
	KX_PYMETHOD(KX_VertexProxy,SetUV);
	KX_PYMETHOD(KX_VertexProxy,GetRGBA);
	KX_PYMETHOD(KX_VertexProxy,SetRGBA);
	KX_PYMETHOD(KX_VertexProxy,GetNormal);
	KX_PYMETHOD(KX_VertexProxy,SetNormal);

};

#endif //__KX_VERTEXPROXY

