/**
 * $Id$
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

/*

 * Copyright (c) 2000 Gino van den Bergen <gino@acm.org>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Gino van den Bergen makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#ifndef MT_Tuple2_H
#define MT_Tuple2_H

#include "MT_Stream.h"
#include "MT_Scalar.h"

class MT_Tuple2 {
public:
    MT_Tuple2() {}
    MT_Tuple2(const float *v) { setValue(v); }
    MT_Tuple2(const double *v) { setValue(v); }
    MT_Tuple2(MT_Scalar x, MT_Scalar y) { setValue(x, y); }
    
    MT_Scalar&       operator[](int i)       { return m_co[i]; }
    const MT_Scalar& operator[](int i) const { return m_co[i]; }
    
    MT_Scalar&       x()       { return m_co[0]; } 
    const MT_Scalar& x() const { return m_co[0]; } 

    MT_Scalar&       y()       { return m_co[1]; }
    const MT_Scalar& y() const { return m_co[1]; } 

	MT_Scalar&       u()       { return m_co[0]; } 
    const MT_Scalar& u() const { return m_co[0]; } 

    MT_Scalar&       v()       { return m_co[1]; }
    const MT_Scalar& v() const { return m_co[1]; } 

    MT_Scalar       *getValue()       { return m_co; }
    const MT_Scalar *getValue() const { return m_co; }

    void getValue(float *v) const { 
        v[0] = m_co[0]; v[1] = m_co[1];
    }
    
    void getValue(double *v) const { 
        v[0] = m_co[0]; v[1] = m_co[1];
    }
    
    void setValue(const float *v) {
        m_co[0] = v[0]; m_co[1] = v[1];
    }
    
    void setValue(const double *v) {
        m_co[0] = v[0]; m_co[1] = v[1];
    }
    
    void setValue(MT_Scalar x, MT_Scalar y) {
        m_co[0] = x; m_co[1] = y; 
    }
    
protected:
    MT_Scalar m_co[2];                            
};

inline bool operator==(const MT_Tuple2& t1, const MT_Tuple2& t2) {
    return t1[0] == t2[0] && t1[1] == t2[1];
}

inline MT_OStream& operator<<(MT_OStream& os, const MT_Tuple2& t) {
    return os << t[0] << ' ' << t[1];
}

#endif

