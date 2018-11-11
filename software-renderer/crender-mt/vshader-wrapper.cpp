/*****************************************************************************
*
* vshader-wrapper.cpp
*
* Created By Jari Korkala 2/9/2014
*
*****************************************************************************/

#include "vshader-wrapper.h"

#include "cr-vertex-shader.h"

using namespace Ceng;

const CRESULT VertexShaderWrapper::ReadUniform(const Ceng::UINT32 index,void *destBuffer)
{
	return shader->ReadUniform(index,destBuffer);
}

const CRESULT VertexShaderWrapper::WriteUniform(const Ceng::UINT32 index,void *sourceBuffer)
{
	return shader->WriteUniform(index,sourceBuffer);
}