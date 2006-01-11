/*

Copyright (c) 2005 Gino van den Bergen / Erwin Coumans http://continuousphysics.com

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef SIMD_QUADWORD_H
#define SIMD_QUADWORD_H

#include "SimdScalar.h"





class	SimdQuadWord
{
	protected:
		ATTRIBUTE_ALIGNED16	(SimdScalar	m_x);
		SimdScalar	m_y;
		SimdScalar	m_z;
		SimdScalar	m_unusedW;

	public:
	
		SIMD_FORCE_INLINE SimdScalar&       operator[](int i)       { return (&m_x)[i];	}      
		SIMD_FORCE_INLINE const SimdScalar& operator[](int i) const { return (&m_x)[i]; }

		SIMD_FORCE_INLINE const SimdScalar& getX() const { return m_x; }

		SIMD_FORCE_INLINE const SimdScalar& getY() const { return m_y; }

		SIMD_FORCE_INLINE const SimdScalar& getZ() const { return m_z; }

		SIMD_FORCE_INLINE void	setX(float x) { m_x = x;};

		SIMD_FORCE_INLINE void	setY(float y) { m_y = y;};

		SIMD_FORCE_INLINE void	setZ(float z) { m_z = z;};

		SIMD_FORCE_INLINE const SimdScalar& x() const { return m_x; }

		SIMD_FORCE_INLINE const SimdScalar& y() const { return m_y; }

		SIMD_FORCE_INLINE const SimdScalar& z() const { return m_z; }


		operator       SimdScalar *()       { return &m_x; }
		operator const SimdScalar *() const { return &m_x; }

		SIMD_FORCE_INLINE void 	setValue(const SimdScalar& x, const SimdScalar& y, const SimdScalar& z)
		{
			m_x=x;
			m_y=y;
			m_z=z;
		}

/*		void getValue(SimdScalar *m) const 
		{
			m[0] = m_x;
			m[1] = m_y;
			m[2] = m_z;
		}
*/
		SIMD_FORCE_INLINE void	setValue(const SimdScalar& x, const SimdScalar& y, const SimdScalar& z,const SimdScalar& w)
		{
			m_x=x;
			m_y=y;
			m_z=z;
			m_unusedW=w;
		}

		SIMD_FORCE_INLINE SimdQuadWord() :
		m_x(0.f),m_y(0.f),m_z(0.f),m_unusedW(0.f)
		{
		}

		SIMD_FORCE_INLINE SimdQuadWord(const SimdScalar& x, const SimdScalar& y, const SimdScalar& z) 
		:m_x(x),m_y(y),m_z(z)
		//todo, remove this in release/simd ?
		,m_unusedW(0.f)
		{
		}

		SIMD_FORCE_INLINE SimdQuadWord(const SimdScalar& x, const SimdScalar& y, const SimdScalar& z,const SimdScalar& w) 
			:m_x(x),m_y(y),m_z(z),m_unusedW(w)
		{
		}


		SIMD_FORCE_INLINE void	setMax(const SimdQuadWord& other)
		{
			if (other.m_x > m_x)
				m_x = other.m_x;

			if (other.m_y > m_y)
				m_y = other.m_y;

			if (other.m_z > m_z)
				m_z = other.m_z;

			if (other.m_unusedW > m_unusedW)
				m_unusedW = other.m_unusedW;
		}

		SIMD_FORCE_INLINE void	setMin(const SimdQuadWord& other)
		{
			if (other.m_x < m_x)
				m_x = other.m_x;

			if (other.m_y < m_y)
				m_y = other.m_y;

			if (other.m_z < m_z)
				m_z = other.m_z;

			if (other.m_unusedW < m_unusedW)
				m_unusedW = other.m_unusedW;
		}



};

#endif //SIMD_QUADWORD_H
