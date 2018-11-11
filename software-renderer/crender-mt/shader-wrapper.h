/*****************************************************************************
*
* shader-wrapper.h
*
* Created By Jari Korkala 2/9/2014
*
* Class ShaderWrapper
*
*	Abstracts shader type so that shader constants can be read and
*	written by client application
*
*****************************************************************************/

#ifndef CRENDER_SHADER_WRAPPER_H
#define CRENDER_SHADER_WRAPPER_H

#include "shader-const-data.h"

namespace Ceng
{
	class ShaderWrapper
	{
	public:

		virtual const CRESULT ReadUniform(const Ceng::UINT32 index,void *destBuffer)=0;

		virtual const CRESULT WriteUniform(const Ceng::UINT32 index,void *sourceBuffer)=0;
	};
}

#endif