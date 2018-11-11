/*****************************************************************************
*
* vertex-stream.h
*
* By Jari Korkala 8/2014
*
* Class VertexStreamData moved here.
*
*****************************************************************************/

#ifndef CRENDER_VERTEX_STREAM_H
#define CRENDER_VERTEX_STREAM_H

#include "crender-base.h"

namespace Ceng
{
	class VertexStreamData
	{
	public:
		Ceng::UINT8 *inputPtr;
		Ceng::UINT32 elementSize;
		Ceng::UINT32 elementCount;

		VertexStreamData()
		{
			inputPtr = nullptr;
			elementSize = 0;
			elementCount = 0;
		}
	};
}

#endif