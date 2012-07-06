/*
 * Copyright 2011, Blender Foundation.
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
 * Contributor:
 *		Dalai Felinto
 */

#ifndef _COM_ConvertYCCToRGBOperation_h
#define _COM_ConvertYCCToRGBOperation_h
#include "COM_NodeOperation.h"


/**
 * this program converts an input color to an output value.
 * it assumes we are in sRGB color space.
 */
class ConvertYCCToRGBOperation : public NodeOperation {
private:
	/**
	 * Cached reference to the inputProgram
	 */
	SocketReader *m_inputOperation;

	/**
	 * YCbCr mode (Jpeg, ITU601, ITU709)
	 */
	int m_mode;
public:
	/**
	 * Default constructor
	 */
	ConvertYCCToRGBOperation();

	/**
	 * the inner loop of this program
	 */
	void executePixel(float *color, float x, float y, PixelSampler sampler, MemoryBuffer * inputBuffers[]);

	/**
	 * Initialize the execution
	 */
	void initExecution();

	/**
	 * Deinitialize the execution
	 */
	void deinitExecution();

	/**
	 * Set the YCC mode
	 */
	void setMode(int mode);
};
#endif
