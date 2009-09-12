/*
 * $Id$
 *
 * ***** BEGIN LGPL LICENSE BLOCK *****
 *
 * Copyright 2009 Jörg Hermann Müller
 *
 * This file is part of AudaSpace.
 *
 * AudaSpace is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AudaSpace is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AudaSpace.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ***** END LGPL LICENSE BLOCK *****
 */

#include "AUD_ConverterFunctions.h"
#include "AUD_Buffer.h"

#define AUD_U8_0		0x80
#define AUD_S16_MAX		0x7FFF
#define AUD_S16_MIN		0x8000
#define AUD_S16_FLT		32768.0
#define AUD_S32_MAX		0x7FFFFFFF
#define AUD_S32_MIN		0x80000000
#define AUD_S32_FLT		2147483648.0
#define AUD_FLT_MAX		1.0
#define AUD_FLT_MIN		-1.0

void AUD_convert_u8_s16(sample_t* target, sample_t* source, int length)
{
	int16_t* t = (int16_t*) target;
	for(int i = 0; i < length; i++)
		t[i] = (((int16_t)source[i]) - AUD_U8_0) << 8;
}

void AUD_convert_u8_s24_be(sample_t* target, sample_t* source, int length)
{
	for(int i = 0; i < length; i++)
	{
		target[i*3] = source[i] - AUD_U8_0;
		target[i*3+1] = 0;
		target[i*3+2] = 0;
	}
}

void AUD_convert_u8_s24_le(sample_t* target, sample_t* source, int length)
{
	for(int i = 0; i < length; i++)
	{
		target[i*3+2] = source[i] - AUD_U8_0;
		target[i*3+1] = 0;
		target[i*3] = 0;
	}
}

void AUD_convert_u8_s32(sample_t* target, sample_t* source, int length)
{
	int32_t* t = (int32_t*) target;
	for(int i = 0; i < length; i++)
		t[i] = (((int32_t)source[i]) - AUD_U8_0) << 24;
}

void AUD_convert_u8_float(sample_t* target, sample_t* source, int length)
{
	float* t = (float*) target;
	for(int i = 0; i < length; i++)
		t[i] = (((int32_t)source[i]) - AUD_U8_0) / ((float)AUD_U8_0);
}

void AUD_convert_u8_double(sample_t* target, sample_t* source, int length)
{
	double* t = (double*) target;
	for(int i = 0; i < length; i++)
		t[i] = (((int32_t)source[i]) - AUD_U8_0) / ((double)AUD_U8_0);
}

void AUD_convert_s16_u8(sample_t* target, sample_t* source, int length)
{
	int16_t* s = (int16_t*) source;
	for(int i = 0; i < length; i++)
		target[i] = (unsigned char)((s[i] >> 8) + AUD_U8_0);
}

void AUD_convert_s16_s24_be(sample_t* target, sample_t* source, int length)
{
	int16_t* s = (int16_t*) source;
	for(int i = 0; i < length; i++)
	{
		target[i*3] = s[i] >> 8 & 0xFF;
		target[i*3+1] = s[i] & 0xFF;
		target[i*3+2] = 0;
	}
}

void AUD_convert_s16_s24_le(sample_t* target, sample_t* source, int length)
{
	int16_t* s = (int16_t*) source;
	for(int i = 0; i < length; i++)
	{
		target[i*3+2] = s[i] >> 8 & 0xFF;
		target[i*3+1] = s[i] & 0xFF;
		target[i*3] = 0;
	}
}

void AUD_convert_s16_s32(sample_t* target, sample_t* source, int length)
{
	int16_t* s = (int16_t*) source;
	int32_t* t = (int32_t*) target;
	for(int i = 0; i < length; i++)
		t[i] = ((int32_t)s[i]) << 16;
}

void AUD_convert_s16_float(sample_t* target, sample_t* source, int length)
{
	int16_t* s = (int16_t*) source;
	float* t = (float*) target;
	for(int i = 0; i < length; i++)
		t[i] = s[i] / AUD_S16_FLT;
}

void AUD_convert_s16_double(sample_t* target, sample_t* source, int length)
{
	int16_t* s = (int16_t*) source;
	double* t = (double*) target;
	for(int i = 0; i < length; i++)
		t[i] = s[i] / AUD_S16_FLT;
}

void AUD_convert_s24_u8_be(sample_t* target, sample_t* source, int length)
{
	for(int i = 0; i < length; i++)
		target[i] = source[i*3] ^ AUD_U8_0;
}

void AUD_convert_s24_u8_le(sample_t* target, sample_t* source, int length)
{
	for(int i = 0; i < length; i++)
		target[i] = source[i*3+2] ^ AUD_U8_0;
}

void AUD_convert_s24_s16_be(sample_t* target, sample_t* source, int length)
{
	int16_t* t = (int16_t*) target;
	for(int i = 0; i < length; i++)
		t[i] = source[i*3] << 8 | source[i*3+1];
}

void AUD_convert_s24_s16_le(sample_t* target, sample_t* source, int length)
{
	int16_t* t = (int16_t*) target;
	for(int i = 0; i < length; i++)
		t[i] = source[i*3+2] << 8 | source[i*3+1];
}

void AUD_convert_s24_s24(sample_t* target, sample_t* source, int length)
{
	memcpy(target, source, length * 3);
}

void AUD_convert_s24_s32_be(sample_t* target, sample_t* source, int length)
{
	int32_t* t = (int32_t*) target;
	for(int i = 0; i < length; i++)
		t[i] = source[i*3] << 24 | source[i*3+1] << 16 | source[i*3+2] << 8;
}

void AUD_convert_s24_s32_le(sample_t* target, sample_t* source, int length)
{
	int32_t* t = (int32_t*) target;
	for(int i = 0; i < length; i++)
		t[i] = source[i*3+2] << 24 | source[i*3+1] << 16 | source[i*3] << 8;
}

void AUD_convert_s24_float_be(sample_t* target, sample_t* source, int length)
{
	float* t = (float*) target;
	int32_t s;
	for(int i = 0; i < length; i++)
	{
		s = source[i*3] << 24 | source[i*3+1] << 16 | source[i*3+2] << 8;
		t[i] = s / AUD_S32_FLT;
	}
}

void AUD_convert_s24_float_le(sample_t* target, sample_t* source, int length)
{
	float* t = (float*) target;
	int32_t s;
	for(int i = 0; i < length; i++)
	{
		s = source[i*3+2] << 24 | source[i*3+1] << 16 | source[i*3] << 8;
		t[i] = s / AUD_S32_FLT;
	}
}

void AUD_convert_s24_double_be(sample_t* target, sample_t* source, int length)
{
	double* t = (double*) target;
	int32_t s;
	for(int i = 0; i < length; i++)
	{
		s = source[i*3] << 24 | source[i*3+1] << 16 | source[i*3+2] << 8;
		t[i] = s / AUD_S32_FLT;
	}
}

void AUD_convert_s24_double_le(sample_t* target, sample_t* source, int length)
{
	double* t = (double*) target;
	int32_t s;
	for(int i = 0; i < length; i++)
	{
		s = source[i*3+2] << 24 | source[i*3+1] << 16 | source[i*3] << 8;
		t[i] = s / AUD_S32_FLT;
	}
}

void AUD_convert_s32_u8(sample_t* target, sample_t* source, int length)
{
	int16_t* s = (int16_t*) source;
	for(int i = 0; i < length; i++)
		target[i] = (unsigned char)((s[i] >> 24) + AUD_U8_0);
}

void AUD_convert_s32_s16(sample_t* target, sample_t* source, int length)
{
	int16_t* t = (int16_t*) target;
	int32_t* s = (int32_t*) source;
	for(int i = 0; i < length; i++)
		t[i] = s[i] >> 16;
}

void AUD_convert_s32_s24_be(sample_t* target, sample_t* source, int length)
{
	int32_t* s = (int32_t*) source;
	for(int i = 0; i < length; i++)
	{
		target[i*3] = s[i] >> 24 & 0xFF;
		target[i*3+1] = s[i] >> 16 & 0xFF;
		target[i*3+2] = s[i] >> 8 & 0xFF;
	}
}

void AUD_convert_s32_s24_le(sample_t* target, sample_t* source, int length)
{
	int16_t* s = (int16_t*) source;
	for(int i = 0; i < length; i++)
	{
		target[i*3+2] = s[i] >> 24 & 0xFF;
		target[i*3+1] = s[i] >> 16 & 0xFF;
		target[i*3] = s[i] >> 8 & 0xFF;
	}
}

void AUD_convert_s32_float(sample_t* target, sample_t* source, int length)
{
	int32_t* s = (int32_t*) source;
	float* t = (float*) target;
	for(int i = 0; i < length; i++)
		t[i] = s[i] / AUD_S32_FLT;
}

void AUD_convert_s32_double(sample_t* target, sample_t* source, int length)
{
	int32_t* s = (int32_t*) source;
	double* t = (double*) target;
	for(int i = 0; i < length; i++)
		t[i] = s[i] / AUD_S32_FLT;
}

void AUD_convert_float_u8(sample_t* target, sample_t* source, int length)
{
	float* s = (float*) source;
	float t;
	for(int i = 0; i < length; i++)
	{
		t = s[i] + AUD_FLT_MAX;
		if(t <= 0.0f)
			target[i] = 0;
		else if(t >= 2.0f)
			target[i] = 255;
		else
			target[i] = (unsigned char)(t*127);
	}
}

void AUD_convert_float_s16(sample_t* target, sample_t* source, int length)
{
	int16_t* t = (int16_t*) target;
	float* s = (float*) source;
	for(int i = 0; i < length; i++)
	{
		if(s[i] <= AUD_FLT_MIN)
			t[i] = AUD_S16_MIN;
		else if(s[i] >= AUD_FLT_MAX)
			t[i] = AUD_S16_MAX;
		else
			t[i] = (int16_t)(s[i] * AUD_S16_MAX);
	}
}

void AUD_convert_float_s24_be(sample_t* target, sample_t* source, int length)
{
	int32_t t;
	float* s = (float*) source;
	for(int i = 0; i < length; i++)
	{
		if(s[i] <= AUD_FLT_MIN)
			t = AUD_S32_MIN;
		else if(s[i] >= AUD_FLT_MAX)
			t = AUD_S32_MAX;
		else
			t = (int32_t)(s[i]*AUD_S32_MAX);
		target[i*3] = t >> 24 & 0xFF;
		target[i*3+1] = t >> 16 & 0xFF;
		target[i*3+2] = t >> 8 & 0xFF;
	}
}

void AUD_convert_float_s24_le(sample_t* target, sample_t* source, int length)
{
	int32_t t;
	float* s = (float*) source;
	for(int i = 0; i < length; i++)
	{
		if(s[i] <= AUD_FLT_MIN)
			t = AUD_S32_MIN;
		else if(s[i] >= AUD_FLT_MAX)
			t = AUD_S32_MAX;
		else
			t = (int32_t)(s[i]*AUD_S32_MAX);
		target[i*3+2] = t >> 24 & 0xFF;
		target[i*3+1] = t >> 16 & 0xFF;
		target[i*3] = t >> 8 & 0xFF;
	}
}

void AUD_convert_float_s32(sample_t* target, sample_t* source, int length)
{
	int32_t* t = (int32_t*) target;
	float* s = (float*) source;
	for(int i = 0; i < length; i++)
	{
		if(s[i] <= AUD_FLT_MIN)
			t[i] = AUD_S32_MIN;
		else if(s[i] >= AUD_FLT_MAX)
			t[i] = AUD_S32_MAX;
		else
			t[i] = (int32_t)(s[i]*AUD_S32_MAX);
	}
}

void AUD_convert_float_double(sample_t* target, sample_t* source, int length)
{
	float* s = (float*) source;
	double* t = (double*) target;
	for(int i = 0; i < length; i++)
		t[i] = s[i];
}

void AUD_convert_double_u8(sample_t* target, sample_t* source, int length)
{
	double* s = (double*) source;
	double t;
	for(int i = 0; i < length; i++)
	{
		t = s[i] + AUD_FLT_MAX;
		if(t <= 0.0)
			target[i] = 0;
		else if(t >= 2.0)
			target[i] = 255;
		else
			target[i] = (unsigned char)(t*127);
	}
}

void AUD_convert_double_s16(sample_t* target, sample_t* source, int length)
{
	int16_t* t = (int16_t*) target;
	double* s = (double*) source;
	for(int i = 0; i < length; i++)
	{
		if(s[i] <= AUD_FLT_MIN)
			t[i] = AUD_S16_MIN;
		else if(s[i] >= AUD_FLT_MAX)
			t[i] = AUD_S16_MAX;
		else
			t[i] = (int16_t)(s[i]*AUD_S16_MAX);
	}
}

void AUD_convert_double_s24_be(sample_t* target, sample_t* source, int length)
{
	int32_t t;
	double* s = (double*) source;
	for(int i = 0; i < length; i++)
	{
		if(s[i] <= AUD_FLT_MIN)
			t = AUD_S32_MIN;
		else if(s[i] >= AUD_FLT_MAX)
			t = AUD_S32_MAX;
		else
			t = (int32_t)(s[i]*AUD_S32_MAX);
		target[i*3] = t >> 24 & 0xFF;
		target[i*3+1] = t >> 16 & 0xFF;
		target[i*3+2] = t >> 8 & 0xFF;
	}
}

void AUD_convert_double_s24_le(sample_t* target, sample_t* source, int length)
{
	int32_t t;
	double* s = (double*) source;
	for(int i = 0; i < length; i++)
	{
		if(s[i] <= AUD_FLT_MIN)
			t = AUD_S32_MIN;
		else if(s[i] >= AUD_FLT_MAX)
			t = AUD_S32_MAX;
		else
			t = (int32_t)(s[i]*AUD_S32_MAX);
		target[i*3+2] = t >> 24 & 0xFF;
		target[i*3+1] = t >> 16 & 0xFF;
		target[i*3] = t >> 8 & 0xFF;
	}
}

void AUD_convert_double_s32(sample_t* target, sample_t* source, int length)
{
	int32_t* t = (int32_t*) target;
	double* s = (double*) source;
	for(int i = 0; i < length; i++)
	{
		if(s[i] <= AUD_FLT_MIN)
			t[i] = AUD_S32_MIN;
		else if(s[i] >= AUD_FLT_MAX)
			t[i] = AUD_S32_MAX;
		else
			t[i] = (int32_t)(s[i]*AUD_S32_MAX);
	}
}

void AUD_convert_double_float(sample_t* target, sample_t* source, int length)
{
	double* s = (double*) source;
	float* t = (float*) target;
	for(int i = 0; i < length; i++)
		t[i] = s[i];
}

void AUD_volume_adjust_u8(sample_t* target, sample_t* source,
						  int count, float volume)
{
	for(int i=0; i<count; i++)
		target[i] = (unsigned char)((source[i]-0x0080) * volume + 0x80);
}

void AUD_volume_adjust_s24_le(sample_t* target, sample_t* source,
							  int count, float volume)
{
	count *= 3;
	int value;

	for(int i=0; i<count; i+=3)
	{
		value = source[i+2] << 16 | source[i+1] << 8 | source[i];
		value |= (((value & 0x800000) >> 23) * 255) << 24;
		value *= volume;
		target[i+2] = value >> 16;
		target[i+1] = value >> 8;
		target[i] = value;
	}
}

void AUD_volume_adjust_s24_be(sample_t* target, sample_t* source,
							  int count, float volume)
{
	count *= 3;
	int value;

	for(int i=0; i < count; i+=3)
	{
		value = source[i] << 16 | source[i+1] << 8 | source[i+2];
		value |= (((value & 0x800000) >> 23) * 255) << 24;
		value *= volume;
		target[i] = value >> 16;
		target[i+1] = value >> 8;
		target[i+2] = value;
	}
}

