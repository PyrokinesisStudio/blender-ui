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
#ifndef __KX_POLYGONMATERIAL_H__
#define __KX_POLYGONMATERIAL_H__

#include "PyObjectPlus.h"

#include "RAS_MaterialBucket.h"
#include "RAS_IRasterizer.h"

struct TFace;
struct Material;
struct MTex;

/**
 *  Material class.
 *
 *  This holds the shader, textures and python methods for setting the render state before
 *  rendering.
 */
class KX_PolygonMaterial : public PyObjectPlus, public RAS_IPolyMaterial
{
	Py_Header;
private:
	/** Blender texture face structure. */
	TFace*			m_tface;
	Material*		m_material;
	
	PyObject*		m_pymaterial;

	mutable int		m_pass;
public:
	
	KX_PolygonMaterial(const STR_String &texname,
		Material* ma,
		int tile,
		int tilexrep,
		int tileyrep,
		int mode,
		bool transparant,
		bool zsort,
		int lightlayer,
		bool bIsTriangle,
		void* clientobject,
		struct TFace* tface,
		PyTypeObject *T = &Type);
	virtual ~KX_PolygonMaterial();
	
	/**
	 * Returns the caching information for this material,
	 * This can be used to speed up the rasterizing process.
	 * @return The caching information.
	 */
	virtual TCachingInfo GetCachingInfo(void) const
	{
		return (void*) this;
	}

	/**
	 * Activates the material in the (OpenGL) rasterizer.
	 * On entry, the cachingInfo contains info about the last activated material.
	 * On exit, the cachingInfo should contain updated info about this material.
	 * @param rasty			The rasterizer in which the material should be active.
	 * @param cachingInfo	The information about the material used to speed up rasterizing.
	 */
	void DefaultActivate(RAS_IRasterizer* rasty, TCachingInfo& cachingInfo) const;
	virtual bool Activate(RAS_IRasterizer* rasty, TCachingInfo& cachingInfo) const;

	/**
	 * Returns the Blender texture face structure that is used for this material.
	 * @return The material's texture face.
	 */
	TFace* GetTFace(void) const
	{
		return m_tface;
	}
	
	
	KX_PYMETHOD_DOC(KX_PolygonMaterial, updateTexture);
	KX_PYMETHOD_DOC(KX_PolygonMaterial, setTexture);
	KX_PYMETHOD_DOC(KX_PolygonMaterial, activate);
	
	KX_PYMETHOD_DOC(KX_PolygonMaterial, setCustomMaterial);
	KX_PYMETHOD_DOC(KX_PolygonMaterial, loadProgram);
	
	virtual PyObject* _getattr(const STR_String& attr);
	virtual int       _setattr(const STR_String& attr, PyObject *pyvalue);
};

#endif // __KX_POLYGONMATERIAL_H__

