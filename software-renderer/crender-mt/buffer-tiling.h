/*****************************************************************************
*
* buffer-tiling.h
*
* Created By Jari Korkala 8/2014
*
*****************************************************************************/

#ifndef CRENDER_BUFFER_TILING_H
#define CRENDER_BUFFER_TILING_H

#include "crender-base.h"

namespace Ceng
{
	//*************************************************************
	// CR_BUFFER_TILING:
	//
	// Tells how to find a particular pixel from the buffer
	namespace BUFFER_TILING
	{
		enum value
		{
			// All pixels on the same row are linear in memory
			SCANLINES = 0 ,
			
			// All pixels in a 2x2 quad are linear in memory
			// using left->right / top->down order
			//
			// Also: Horizontally adjacent quads are linear in memory
			LQUADS = 1 ,
			
			// Pixels in quads are linear, and quads are linear
			// within a 8x8 pixel tile
			TILED8_LQUADS = 2 ,
			
			TILED16_LQUADS = 3 ,

			// All pixels in a 4x4 pixel tile are linear in memory
			// using left->right / top->down order
			TILED4 = 4 ,

			// All pixels in a 8x8 pixel tile are linear in memory
			// using left->right / top->down order
			TILED8 = 5,

			// All pixels in a 8x8 pixel tile are linear in memory
			// using left->right / top->down order
			TILED16 = 6,
			
			FORCE32B = UINT32(1) << 30 , // Not used , forces enum to compile as int32
		};
	}

}

#endif