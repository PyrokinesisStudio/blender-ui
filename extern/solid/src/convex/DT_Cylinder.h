/*
 * SOLID - Software Library for Interference Detection
 * 
 * Copyright (C) 2001-2003  Dtecta.  All rights reserved.
 *
 * This library may be distributed under the terms of the Q Public License
 * (QPL) as defined by Trolltech AS of Norway and appearing in the file
 * LICENSE.QPL included in the packaging of this file.
 *
 * This library may be distributed and/or modified under the terms of the
 * GNU General Public License (GPL) version 2 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This library is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Commercial use or any other use of this library not covered by either 
 * the QPL or the GPL requires an additional license from Dtecta. 
 * Please contact info@dtecta.com for enquiries about the terms of commercial
 * use of this library.
 */

#ifndef DT_CYLINDER_H
#define DT_CYLINDER_H

#include "DT_Convex.h"

class DT_Cylinder : public DT_Convex {
public:
    DT_Cylinder(MT_Scalar r, MT_Scalar h) : 
        radius(r), 
        halfHeight(h * MT_Scalar(0.5)) {}
    
    virtual MT_Point3 support(const MT_Vector3& v) const;
    
protected:
    MT_Scalar radius;
    MT_Scalar halfHeight;
};

#endif
