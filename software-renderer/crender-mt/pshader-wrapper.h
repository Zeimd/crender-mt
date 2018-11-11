/*****************************************************************************
*
* pshader-wrapper.h
*
* Created By Jari Korkala 2/9/2014
*
*****************************************************************************/

#ifndef CRENDER_PSHADER_WRAPPER_H
#define CRENDER_PSHADER_WRAPPER_H

#include "shader-wrapper.h"

namespace Ceng
{
	class CR_PixelShader;

	class PixelShaderWrapper : public ShaderWrapper
	{
	public:

		CR_PixelShader *shader;

	public:

		virtual const CRESULT ReadUniform(const Ceng::UINT32 index,void *destBuffer) override;

		virtual const CRESULT WriteUniform(const Ceng::UINT32 index,void *sourceBuffer) override;

	};
}

#endif