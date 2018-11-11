/*****************************************************************************
*
* viewport-state.h
*
* Created By Jari Korkala 4/10/2014
*
*****************************************************************************/

#ifndef CENG_VIEWPORT_STATE_H
#define CENG_VIEWPORT_STATE_H

#include <ceng/datatypes/basic-types.h>
#include <ceng/math/ce-vector.h>
#include <ceng/datatypes/rectangle.h>

namespace Ceng
{
	class ViewportState
	{
	public:

		/*
		 * Where center of a pixel is relative to its left edge.
		 */
		Ceng::FLOAT32 pixelCenterOffsetX;

		/*
		 * Where center of a pixel is relative to its top edge.
		 */
		Ceng::FLOAT32 pixelCenterOffsetY;

		/*
		 * Width of output window for this frame.
		 */
		UINT32 windowWidth;

		/*
		 * Height of output window for this frame.
		 */
		UINT32 windowHeight;

		Ceng::VectorF4 screenCenter;
		Ceng::VectorF4 screenDilation;

		Ceng::Rectangle rasterizerPort;

	public:

		ViewportState() : pixelCenterOffsetX(0.0f),pixelCenterOffsetY(0.0f),
			windowWidth(0),windowHeight(0)
		{
		}
	};
}

#endif