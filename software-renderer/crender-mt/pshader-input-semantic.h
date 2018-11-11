/*****************************************************************************
*
* pshader-input-semantic.h
*
* Created By Jari Korkala 2/9/2014
*
*****************************************************************************/

#ifndef CRENDER_PSHADER_INPUT_SEMANTIC_H
#define CRENDER_PSHADER_INPUT_SEMANTIC_H

#include <ceng/enums/shader-semantic.h>
#include <ceng/enums/shader-datatype.h>

namespace Ceng
{
	class CR_PixelShaderSemantic
	{
	public:

		/**
		 * A CR_VTX_SEMANTIC_FLAG value.
		 */
		Ceng::SHADER_SEMANTIC::value semantic;
	};
}

#endif