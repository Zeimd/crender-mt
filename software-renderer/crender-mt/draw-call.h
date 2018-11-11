/*****************************************************************************
*
* draw-call.h
*
* By Jari Korkala 8/2014
*
*****************************************************************************/

#ifndef CRENDER_DRAW_CALL_H
#define CRENDER_DRAW_CALL_H

#include <ceng/enums/primitive-type.h>

#include "crender-base.h"

namespace Ceng
{
	struct DrawCall
	{
	public:

		PRIMITIVE_TYPE::value primitiveType;

		Ceng::BOOL indexed;

		Ceng::UINT32 firstIndex;

		/**
		 * Second index for triangle fan, even/odd indicator for strip.
		 */
		Ceng::UINT32 secondIndex;


		Ceng::UINT32 primitives;
	};

	
}

#endif