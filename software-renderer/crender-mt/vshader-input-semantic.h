/*****************************************************************************
*
* vshader-input-semantic.h
*
* Created By Jari Korkala 27/8/2014
*
*****************************************************************************/

#ifndef CRENDER_VSHADER_INPUT_SEMANTIC_H
#define CRENDER_VSHADER_INPUT_SEMANTIC_H

#include <ceng/enums/shader-semantic.h>

#include "crender-base.h"

namespace Ceng
{
	class CR_vsInputSemantic
	{
	public:
		Ceng::SHADER_SEMANTIC::value semantic;

		/**
		 * Which variable in the vertex format this corresponds to.
		 */
		Ceng::INT32 sourceIndex;

		Ceng::UINT32 options;

	public:
		CR_vsInputSemantic()
		{
			semantic = SHADER_SEMANTIC::FORMAT_END;
			sourceIndex = -1;
			options = 0;
		}
	};
}

#endif