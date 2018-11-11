/*****************************************************************************
*
* pshader-wrapper.cpp
*
* Created By Jari Korkala 2/9/2014
*
*****************************************************************************/

#include "pshader-wrapper.h"

#include "cr-pixel-shader.h"

using namespace Ceng;

const CRESULT PixelShaderWrapper::ReadUniform(const Ceng::UINT32 index,void *destBuffer)
{
	return shader->ReadUniform(index,destBuffer);
}

const CRESULT PixelShaderWrapper::WriteUniform(const Ceng::UINT32 index,void *sourceBuffer)
{
	return shader->WriteUniform(index,sourceBuffer);
}