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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BMF_FontData.h"
#include "BMF_Settings.h"

#if BMF_INCLUDE_SCR15

static unsigned char bitmap_data[]= {
	0x80,0x80,0x00,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x90,0x90,0x90,0x90,0x48,0x48,
	0x48,0xfe,0x24,0x24,0x24,0x7f,0x12,0x12,
	0x20,0x70,0xa8,0xa8,0x28,0x30,0x60,0xa0,
	0xa8,0xa8,0x70,0x20,0x8c,0x52,0x52,0x2c,
	0x10,0x10,0x68,0x94,0x94,0x62,0x72,0x8c,
	0x84,0x8a,0x50,0x20,0x30,0x48,0x48,0x30,
	0x80,0x40,0x60,0x60,0x10,0x20,0x40,0x40,
	0x80,0x80,0x80,0x80,0x80,0x40,0x40,0x20,
	0x10,0x80,0x40,0x20,0x20,0x10,0x10,0x10,
	0x10,0x10,0x20,0x20,0x40,0x80,0x20,0xa8,
	0x70,0x70,0xa8,0x20,0x10,0x10,0x10,0xfe,
	0x10,0x10,0x10,0x80,0x40,0x20,0x60,0x60,
	0xfc,0xc0,0xc0,0x80,0x80,0x40,0x40,0x20,
	0x20,0x10,0x10,0x08,0x08,0x04,0x04,0x78,
	0x84,0x84,0xc4,0xa4,0x94,0x8c,0x84,0x84,
	0x78,0xe0,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0xc0,0x40,0xfc,0x80,0x40,0x20,0x10,
	0x08,0x04,0x84,0x84,0x78,0x78,0x84,0x04,
	0x04,0x04,0x38,0x04,0x04,0x84,0x78,0x08,
	0x08,0x08,0xfc,0x88,0x48,0x48,0x28,0x18,
	0x08,0x78,0x84,0x04,0x04,0x04,0xf8,0x80,
	0x80,0x80,0xfc,0x78,0x84,0x84,0x84,0x84,
	0xf8,0x80,0x80,0x84,0x78,0x20,0x20,0x20,
	0x10,0x10,0x08,0x08,0x04,0x04,0xfc,0x78,
	0x84,0x84,0x84,0x84,0x78,0x84,0x84,0x84,
	0x78,0x78,0x84,0x04,0x04,0x7c,0x84,0x84,
	0x84,0x84,0x78,0xc0,0xc0,0x00,0x00,0x00,
	0xc0,0xc0,0x80,0x40,0xc0,0xc0,0x00,0x00,
	0x00,0xc0,0xc0,0x04,0x08,0x10,0x20,0x40,
	0x80,0x40,0x20,0x10,0x08,0x04,0xfc,0x00,
	0x00,0xfc,0x80,0x40,0x20,0x10,0x08,0x04,
	0x08,0x10,0x20,0x40,0x80,0x10,0x10,0x00,
	0x10,0x10,0x08,0x04,0x84,0x84,0x78,0x38,
	0x44,0x80,0x98,0xa4,0xa4,0x9c,0x84,0x48,
	0x30,0x84,0x84,0xfc,0x84,0x48,0x48,0x48,
	0x30,0x30,0x30,0xf8,0x84,0x84,0x84,0x84,
	0xf8,0x84,0x84,0x84,0xf8,0x78,0x84,0x84,
	0x80,0x80,0x80,0x80,0x84,0x84,0x78,0xf0,
	0x88,0x84,0x84,0x84,0x84,0x84,0x84,0x88,
	0xf0,0xfc,0x80,0x80,0x80,0x80,0xf8,0x80,
	0x80,0x80,0xfc,0x80,0x80,0x80,0x80,0x80,
	0xf8,0x80,0x80,0x80,0xfc,0x74,0x8c,0x84,
	0x84,0x84,0x9c,0x80,0x80,0x84,0x78,0x84,
	0x84,0x84,0x84,0x84,0xfc,0x84,0x84,0x84,
	0x84,0xe0,0x40,0x40,0x40,0x40,0x40,0x40,
	0x40,0x40,0xe0,0x70,0x88,0x88,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x84,0x84,0x88,
	0x90,0xa0,0xc0,0xa0,0x90,0x88,0x84,0xfc,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x82,0x82,0x92,0x92,0xaa,0xaa,0xc6,
	0xc6,0x82,0x82,0x84,0x8c,0x8c,0x94,0x94,
	0xa4,0xa4,0xc4,0xc4,0x84,0x78,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x78,0x80,
	0x80,0x80,0x80,0xf8,0x84,0x84,0x84,0x84,
	0xf8,0x04,0x08,0x10,0x78,0xa4,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x78,0x84,0x84,
	0x88,0x90,0xf8,0x84,0x84,0x84,0x84,0xf8,
	0x78,0x84,0x84,0x04,0x18,0x60,0x80,0x84,
	0x84,0x78,0x10,0x10,0x10,0x10,0x10,0x10,
	0x10,0x10,0x10,0xfe,0x78,0x84,0x84,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x30,0x30,
	0x30,0x48,0x48,0x48,0x84,0x84,0x84,0x84,
	0x44,0x44,0x44,0xaa,0xaa,0xaa,0x92,0x92,
	0x92,0x82,0x84,0x84,0x48,0x48,0x30,0x30,
	0x48,0x48,0x84,0x84,0x10,0x10,0x10,0x10,
	0x10,0x28,0x44,0x44,0x82,0x82,0xfc,0x80,
	0x40,0x40,0x20,0x10,0x08,0x08,0x04,0xfc,
	0xf0,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0xf0,0x04,0x04,0x08,
	0x08,0x10,0x10,0x20,0x20,0x40,0x40,0x80,
	0x80,0xf0,0x10,0x10,0x10,0x10,0x10,0x10,
	0x10,0x10,0x10,0x10,0x10,0xf0,0x88,0x50,
	0x20,0xff,0x20,0x40,0xc0,0xc0,0x74,0x88,
	0x88,0x78,0x08,0x88,0x70,0xb8,0xc4,0x84,
	0x84,0x84,0xc4,0xb8,0x80,0x80,0x80,0x78,
	0x84,0x80,0x80,0x80,0x84,0x78,0x74,0x8c,
	0x84,0x84,0x84,0x8c,0x74,0x04,0x04,0x04,
	0x78,0x84,0x80,0xfc,0x84,0x84,0x78,0x20,
	0x20,0x20,0x20,0x20,0x20,0xf8,0x20,0x20,
	0x1c,0x78,0x84,0x04,0x04,0x74,0x8c,0x84,
	0x84,0x84,0x8c,0x74,0x84,0x84,0x84,0x84,
	0x84,0xc4,0xb8,0x80,0x80,0x80,0x20,0x20,
	0x20,0x20,0x20,0x20,0xe0,0x00,0x20,0x20,
	0x70,0x88,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x38,0x00,0x08,0x08,0x84,0x88,
	0x90,0xe0,0xa0,0x90,0x88,0x80,0x80,0x80,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0xe0,0x92,0x92,0x92,0x92,0x92,0x92,
	0xec,0x84,0x84,0x84,0x84,0x84,0xc4,0xb8,
	0x78,0x84,0x84,0x84,0x84,0x84,0x78,0x80,
	0x80,0x80,0x80,0xb8,0xc4,0x84,0x84,0x84,
	0xc4,0xb8,0x04,0x04,0x04,0x04,0x74,0x8c,
	0x84,0x84,0x84,0x8c,0x74,0x80,0x80,0x80,
	0x80,0x80,0xc4,0xb8,0x78,0x84,0x04,0x78,
	0x80,0x84,0x78,0x1c,0x20,0x20,0x20,0x20,
	0x20,0xf8,0x20,0x20,0x74,0x8c,0x84,0x84,
	0x84,0x84,0x84,0x30,0x30,0x48,0x48,0x84,
	0x84,0x84,0x6c,0x92,0x92,0x92,0x92,0x82,
	0x82,0x84,0x84,0x48,0x30,0x48,0x84,0x84,
	0x78,0x84,0x04,0x04,0x74,0x8c,0x84,0x84,
	0x84,0x84,0x84,0xfc,0x80,0x40,0x20,0x10,
	0x08,0xfc,0x1c,0x20,0x20,0x20,0x20,0x20,
	0xc0,0x20,0x20,0x20,0x20,0x20,0x1c,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0xe0,0x10,0x10,
	0x10,0x10,0x10,0x0c,0x10,0x10,0x10,0x10,
	0x10,0xe0,0x98,0xb4,0x64,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x00,0x80,0x80,0x20,
	0x20,0x70,0x88,0x80,0x80,0x88,0x70,0x20,
	0x20,0xb8,0x44,0x40,0x40,0xf0,0x40,0x40,
	0x40,0x48,0x30,0x84,0x78,0x84,0x84,0x84,
	0x78,0x84,0x38,0x10,0x7c,0x10,0x7c,0x28,
	0x44,0x44,0x82,0x82,0x80,0x80,0x80,0x80,
	0x80,0x80,0x00,0x00,0x80,0x80,0x80,0x80,
	0x80,0x80,0x78,0x84,0x04,0x18,0x24,0x44,
	0x84,0x88,0x90,0x60,0x80,0x84,0x78,0xd8,
	0x38,0x44,0x92,0xaa,0xa2,0xaa,0x92,0x44,
	0x38,0xf8,0x00,0x68,0x90,0x70,0x10,0x60,
	0x09,0x12,0x24,0x48,0x90,0x48,0x24,0x12,
	0x09,0x04,0x04,0xfc,0xfc,0x38,0x44,0xaa,
	0xaa,0xb2,0xaa,0xb2,0x44,0x38,0xf0,0x60,
	0x90,0x90,0x60,0xfe,0x00,0x10,0x10,0x10,
	0xfe,0x10,0x10,0x10,0xf0,0x40,0x20,0x90,
	0x60,0xe0,0x10,0x60,0x10,0xe0,0x80,0x40,
	0x80,0x80,0x80,0xb4,0xc8,0x88,0x88,0x88,
	0x88,0x88,0x24,0x24,0x24,0x24,0x24,0x24,
	0x64,0xa4,0xa4,0xa4,0xa4,0x7e,0xc0,0xc0,
	0x20,0x40,0xe0,0x40,0x40,0xc0,0x40,0xf8,
	0x00,0x70,0x88,0x88,0x88,0x70,0x90,0x48,
	0x24,0x12,0x09,0x12,0x24,0x48,0x90,0x04,
	0x9e,0x54,0x2c,0x14,0xe8,0x44,0x42,0xc0,
	0x40,0x1e,0x08,0x84,0x52,0x2c,0x10,0xe8,
	0x44,0x42,0xc0,0x40,0x04,0x9e,0x54,0x2c,
	0xd4,0x28,0x44,0x22,0xc0,0x78,0x84,0x84,
	0x80,0x40,0x20,0x20,0x00,0x20,0x20,0x84,
	0x84,0xfc,0x84,0x48,0x48,0x30,0x30,0x00,
	0x20,0x40,0x84,0x84,0xfc,0x84,0x48,0x48,
	0x30,0x30,0x00,0x10,0x08,0x84,0x84,0xfc,
	0x84,0x48,0x48,0x30,0x30,0x00,0x48,0x30,
	0x84,0x84,0xfc,0x84,0x48,0x48,0x30,0x30,
	0x00,0x98,0x64,0x84,0x84,0xfc,0x84,0x48,
	0x48,0x30,0x30,0x00,0x6c,0x84,0x84,0xfc,
	0x84,0x48,0x48,0x30,0x30,0x30,0x48,0x30,
	0x9e,0x90,0x90,0xf0,0x90,0x5c,0x50,0x50,
	0x30,0x1e,0x30,0x08,0x10,0x78,0x84,0x84,
	0x80,0x80,0x80,0x80,0x84,0x84,0x78,0xfc,
	0x80,0x80,0x80,0xf8,0x80,0x80,0xfc,0x00,
	0x20,0x40,0xfc,0x80,0x80,0x80,0xf8,0x80,
	0x80,0xfc,0x00,0x10,0x08,0xfc,0x80,0x80,
	0x80,0xf8,0x80,0x80,0xfc,0x00,0x48,0x30,
	0xfc,0x80,0x80,0x80,0xf8,0x80,0x80,0xfc,
	0x00,0x6c,0xe0,0x40,0x40,0x40,0x40,0x40,
	0x40,0xe0,0x00,0x40,0x80,0xe0,0x40,0x40,
	0x40,0x40,0x40,0x40,0xe0,0x00,0x40,0x20,
	0xe0,0x40,0x40,0x40,0x40,0x40,0x40,0xe0,
	0x00,0x90,0x60,0x70,0x20,0x20,0x20,0x20,
	0x20,0x20,0x70,0x00,0xd8,0x78,0x44,0x42,
	0x42,0x42,0xf2,0x42,0x42,0x44,0x78,0x84,
	0x8c,0x94,0x94,0xa4,0xa4,0xc4,0x84,0x00,
	0x98,0x64,0x78,0x84,0x84,0x84,0x84,0x84,
	0x84,0x78,0x00,0x20,0x40,0x78,0x84,0x84,
	0x84,0x84,0x84,0x84,0x78,0x00,0x10,0x08,
	0x78,0x84,0x84,0x84,0x84,0x84,0x84,0x78,
	0x00,0x48,0x30,0x78,0x84,0x84,0x84,0x84,
	0x84,0x84,0x78,0x00,0x98,0x64,0x78,0x84,
	0x84,0x84,0x84,0x84,0x84,0x78,0x00,0x6c,
	0x84,0x48,0x30,0x30,0x48,0x84,0xbc,0x42,
	0x62,0x52,0x52,0x4a,0x4a,0x46,0x42,0x3d,
	0x78,0x84,0x84,0x84,0x84,0x84,0x84,0x84,
	0x00,0x20,0x40,0x78,0x84,0x84,0x84,0x84,
	0x84,0x84,0x84,0x00,0x10,0x08,0x78,0x84,
	0x84,0x84,0x84,0x84,0x84,0x84,0x00,0x48,
	0x30,0x78,0x84,0x84,0x84,0x84,0x84,0x84,
	0x84,0x00,0x6c,0x10,0x10,0x10,0x10,0x28,
	0x44,0x44,0x82,0x00,0x10,0x08,0xe0,0x40,
	0x7c,0x42,0x42,0x42,0x42,0x7c,0x40,0xe0,
	0x98,0xa4,0x84,0x84,0x84,0x88,0xb0,0x88,
	0x88,0x70,0x74,0x88,0x88,0x78,0x08,0x88,
	0x70,0x00,0x20,0x40,0x74,0x88,0x88,0x78,
	0x08,0x88,0x70,0x00,0x20,0x10,0x74,0x88,
	0x88,0x78,0x08,0x88,0x70,0x00,0x48,0x30,
	0x74,0x88,0x88,0x78,0x08,0x88,0x70,0x00,
	0x98,0x64,0x74,0x88,0x88,0x78,0x08,0x88,
	0x70,0x00,0xd8,0x74,0x88,0x88,0x78,0x08,
	0x88,0x70,0x00,0x30,0x48,0x30,0x6c,0x92,
	0x90,0x7e,0x12,0x92,0x6c,0x30,0x08,0x10,
	0x78,0x84,0x80,0x80,0x80,0x84,0x78,0x78,
	0x84,0x80,0xfc,0x84,0x84,0x78,0x00,0x20,
	0x40,0x78,0x84,0x80,0xfc,0x84,0x84,0x78,
	0x00,0x10,0x08,0x78,0x84,0x80,0xfc,0x84,
	0x84,0x78,0x00,0x48,0x30,0x78,0x84,0x80,
	0xfc,0x84,0x84,0x78,0x00,0x6c,0x20,0x20,
	0x20,0x20,0x20,0x20,0xe0,0x00,0x40,0x80,
	0x20,0x20,0x20,0x20,0x20,0x20,0xe0,0x00,
	0x40,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0xe0,0x00,0x90,0x60,0x10,0x10,0x10,0x10,
	0x10,0x10,0x70,0x00,0xd8,0x78,0x84,0x84,
	0x84,0x84,0x84,0x7c,0x04,0xc8,0x30,0xc8,
	0x84,0x84,0x84,0x84,0x84,0xc4,0xb8,0x00,
	0x98,0x64,0x78,0x84,0x84,0x84,0x84,0x84,
	0x78,0x00,0x20,0x40,0x78,0x84,0x84,0x84,
	0x84,0x84,0x78,0x00,0x10,0x08,0x78,0x84,
	0x84,0x84,0x84,0x84,0x78,0x00,0x48,0x30,
	0x78,0x84,0x84,0x84,0x84,0x84,0x78,0x00,
	0x98,0x64,0x78,0x84,0x84,0x84,0x84,0x84,
	0x78,0x00,0x00,0x6c,0x30,0x00,0x00,0xfc,
	0x00,0x00,0x30,0xbc,0x62,0x52,0x4a,0x46,
	0x42,0x3d,0x74,0x8c,0x84,0x84,0x84,0x84,
	0x84,0x00,0x20,0x40,0x74,0x8c,0x84,0x84,
	0x84,0x84,0x84,0x00,0x20,0x10,0x74,0x8c,
	0x84,0x84,0x84,0x84,0x84,0x00,0x48,0x30,
	0x74,0x8c,0x84,0x84,0x84,0x84,0x84,0x00,
	0x00,0x6c,0x78,0x84,0x04,0x04,0x74,0x8c,
	0x84,0x84,0x84,0x84,0x84,0x00,0x20,0x10,
	0xe0,0x40,0x40,0x5c,0x62,0x42,0x42,0x42,
	0x62,0x5c,0x40,0x40,0xc0,0x78,0x84,0x04,
	0x04,0x74,0x8c,0x84,0x84,0x84,0x84,0x84,
	0x00,0x00,0x6c,
};

BMF_FontData BMF_font_scr15 = {
	0, -4,
	8, 11,
	{
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0, 0, 0, 0, 8, -1},
		{1, 10, -3, 0, 8, 0},
		{4, 4, -2, -6, 8, 10},
		{8, 10, 0, 0, 8, 14},
		{5, 12, -1, 1, 8, 24},
		{7, 10, 0, 0, 8, 36},
		{7, 10, 0, 0, 8, 46},
		{3, 4, -2, -6, 8, 56},
		{4, 13, -2, 2, 8, 60},
		{4, 13, -2, 2, 8, 73},
		{5, 6, -1, -2, 8, 86},
		{7, 7, 0, -1, 8, 92},
		{3, 5, -2, 3, 8, 99},
		{6, 1, -1, -4, 8, 104},
		{2, 2, -3, 0, 8, 105},
		{6, 12, -1, 1, 8, 107},
		{6, 10, -1, 0, 8, 119},
		{3, 10, -3, 0, 8, 129},
		{6, 10, -1, 0, 8, 139},
		{6, 10, -1, 0, 8, 149},
		{6, 10, -1, 0, 8, 159},
		{6, 10, -1, 0, 8, 169},
		{6, 10, -1, 0, 8, 179},
		{6, 10, -1, 0, 8, 189},
		{6, 10, -1, 0, 8, 199},
		{6, 10, -1, 0, 8, 209},
		{2, 7, -3, 0, 8, 219},
		{2, 9, -3, 2, 8, 226},
		{6, 11, -1, 1, 8, 235},
		{6, 4, -1, -3, 8, 246},
		{6, 11, -1, 1, 8, 250},
		{6, 10, -1, 0, 8, 261},
		{6, 10, -1, 0, 8, 271},
		{6, 10, -1, 0, 8, 281},
		{6, 10, -1, 0, 8, 291},
		{6, 10, -1, 0, 8, 301},
		{6, 10, -1, 0, 8, 311},
		{6, 10, -1, 0, 8, 321},
		{6, 10, -1, 0, 8, 331},
		{6, 10, -1, 0, 8, 341},
		{6, 10, -1, 0, 8, 351},
		{3, 10, -2, 0, 8, 361},
		{5, 10, -1, 0, 8, 371},
		{6, 10, -1, 0, 8, 381},
		{6, 10, -1, 0, 8, 391},
		{7, 10, 0, 0, 8, 401},
		{6, 10, -1, 0, 8, 411},
		{6, 10, -1, 0, 8, 421},
		{6, 10, -1, 0, 8, 431},
		{6, 13, -1, 3, 8, 441},
		{6, 10, -1, 0, 8, 454},
		{6, 10, -1, 0, 8, 464},
		{7, 10, 0, 0, 8, 474},
		{6, 10, -1, 0, 8, 484},
		{6, 10, -1, 0, 8, 494},
		{7, 10, 0, 0, 8, 504},
		{6, 10, -1, 0, 8, 514},
		{7, 10, 0, 0, 8, 524},
		{6, 10, -1, 0, 8, 534},
		{4, 13, -2, 2, 8, 544},
		{6, 12, -1, 1, 8, 557},
		{4, 13, -2, 2, 8, 569},
		{5, 3, -1, -6, 8, 582},
		{8, 1, 0, 3, 8, 585},
		{3, 4, -2, -6, 8, 586},
		{6, 7, -1, 0, 8, 590},
		{6, 10, -1, 0, 8, 597},
		{6, 7, -1, 0, 8, 607},
		{6, 10, -1, 0, 8, 614},
		{6, 7, -1, 0, 8, 624},
		{6, 10, -1, 0, 8, 631},
		{6, 11, -1, 4, 8, 641},
		{6, 10, -1, 0, 8, 652},
		{3, 10, -2, 0, 8, 662},
		{5, 14, -1, 4, 8, 672},
		{6, 10, -1, 0, 8, 686},
		{3, 10, -2, 0, 8, 696},
		{7, 7, 0, 0, 8, 706},
		{6, 7, -1, 0, 8, 713},
		{6, 7, -1, 0, 8, 720},
		{6, 11, -1, 4, 8, 727},
		{6, 11, -1, 4, 8, 738},
		{6, 7, -1, 0, 8, 749},
		{6, 7, -1, 0, 8, 756},
		{6, 9, -1, 0, 8, 763},
		{6, 7, -1, 0, 8, 772},
		{6, 7, -1, 0, 8, 779},
		{7, 7, 0, 0, 8, 786},
		{6, 7, -1, 0, 8, 793},
		{6, 11, -1, 4, 8, 800},
		{6, 7, -1, 0, 8, 811},
		{6, 13, -1, 2, 8, 818},
		{1, 14, -3, 3, 8, 831},
		{6, 13, -1, 2, 8, 845},
		{6, 3, -1, -3, 8, 858},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{1, 10, -3, 3, 8, 861},
		{5, 10, -1, 0, 8, 871},
		{6, 10, -1, 0, 8, 881},
		{6, 7, -1, -2, 8, 891},
		{7, 10, 0, 0, 8, 898},
		{1, 14, -3, 3, 8, 908},
		{6, 13, -1, 3, 8, 922},
		{5, 1, -1, -9, 8, 935},
		{7, 9, 0, 0, 8, 936},
		{5, 7, -1, -3, 8, 945},
		{8, 9, 0, 0, 8, 952},
		{6, 3, -1, -3, 8, 961},
		{6, 1, -1, -4, 8, 964},
		{7, 9, 0, 0, 8, 965},
		{4, 1, -2, -9, 8, 974},
		{4, 4, -2, -4, 8, 975},
		{7, 9, 0, 0, 8, 979},
		{4, 5, -2, -5, 8, 988},
		{4, 5, -2, -5, 8, 993},
		{2, 2, -3, -9, 8, 998},
		{6, 10, -1, 3, 8, 1000},
		{7, 12, 0, 2, 8, 1010},
		{2, 1, -3, -4, 8, 1022},
		{3, 3, -3, 3, 8, 1023},
		{3, 5, -3, -5, 8, 1026},
		{5, 7, -1, -3, 8, 1031},
		{8, 9, 0, 0, 8, 1038},
		{7, 10, 0, 0, 8, 1047},
		{7, 11, 0, 1, 8, 1057},
		{7, 9, 0, -1, 8, 1068},
		{6, 10, -1, 2, 8, 1077},
		{6, 11, -1, 0, 8, 1087},
		{6, 11, -1, 0, 8, 1098},
		{6, 11, -1, 0, 8, 1109},
		{6, 11, -1, 0, 8, 1120},
		{6, 10, -1, 0, 8, 1131},
		{6, 11, -1, 0, 8, 1141},
		{7, 10, 0, 0, 8, 1152},
		{6, 13, -1, 3, 8, 1162},
		{6, 11, -1, 0, 8, 1175},
		{6, 11, -1, 0, 8, 1186},
		{6, 11, -1, 0, 8, 1197},
		{6, 10, -1, 0, 8, 1208},
		{3, 11, -2, 0, 8, 1218},
		{3, 11, -2, 0, 8, 1229},
		{4, 11, -2, 0, 8, 1240},
		{5, 10, -1, 0, 8, 1251},
		{7, 10, 0, 0, 8, 1261},
		{6, 11, -1, 0, 8, 1271},
		{6, 11, -1, 0, 8, 1282},
		{6, 11, -1, 0, 8, 1293},
		{6, 11, -1, 0, 8, 1304},
		{6, 11, -1, 0, 8, 1315},
		{6, 10, -1, 0, 8, 1326},
		{6, 6, -1, -1, 8, 1336},
		{8, 10, 0, 0, 8, 1342},
		{6, 11, -1, 0, 8, 1352},
		{6, 11, -1, 0, 8, 1363},
		{6, 11, -1, 0, 8, 1374},
		{6, 10, -1, 0, 8, 1385},
		{7, 11, 0, 0, 8, 1395},
		{7, 10, 0, 0, 8, 1406},
		{6, 10, -1, 0, 8, 1416},
		{6, 10, -1, 0, 8, 1426},
		{6, 10, -1, 0, 8, 1436},
		{6, 10, -1, 0, 8, 1446},
		{6, 10, -1, 0, 8, 1456},
		{6, 9, -1, 0, 8, 1466},
		{6, 11, -1, 0, 8, 1475},
		{7, 7, 0, 0, 8, 1486},
		{6, 10, -1, 3, 8, 1493},
		{6, 10, -1, 0, 8, 1503},
		{6, 10, -1, 0, 8, 1513},
		{6, 10, -1, 0, 8, 1523},
		{6, 9, -1, 0, 8, 1533},
		{3, 10, -2, 0, 8, 1542},
		{3, 10, -2, 0, 8, 1552},
		{4, 10, -2, 0, 8, 1562},
		{5, 9, -1, 0, 8, 1572},
		{6, 11, -1, 0, 8, 1581},
		{6, 10, -1, 0, 8, 1592},
		{6, 10, -1, 0, 8, 1602},
		{6, 10, -1, 0, 8, 1612},
		{6, 10, -1, 0, 8, 1622},
		{6, 10, -1, 0, 8, 1632},
		{6, 10, -1, 0, 8, 1642},
		{6, 7, -1, 0, 8, 1652},
		{8, 7, 0, 0, 8, 1659},
		{6, 10, -1, 0, 8, 1666},
		{6, 10, -1, 0, 8, 1676},
		{6, 10, -1, 0, 8, 1686},
		{6, 10, -1, 0, 8, 1696},
		{6, 14, -1, 4, 8, 1706},
		{7, 13, 0, 3, 8, 1720},
		{6, 14, -1, 4, 8, 1733},
	},
	bitmap_data
};

#endif

