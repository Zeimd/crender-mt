/*****************************************************************************
*
* cr-shader-const.cpp
*
* Created By Jari Korkala 31/8/2014
*
* - Moved CR_ShaderConstHandle methods here from header.
*
*****************************************************************************/

#include "cr-shader-const.h"

#include "cr-vertex-shader.h"

using namespace Ceng;

CR_ShaderConstHandle::CR_ShaderConstHandle()
{
	index = 0;
	shader = nullptr;
}

CR_ShaderConstHandle::CR_ShaderConstHandle(const Ceng::UINT32 index,ShaderWrapper *shader)
{
	this->index = index;
	this->shader = shader;
}

CR_ShaderConstHandle::~CR_ShaderConstHandle()
{
}

void CR_ShaderConstHandle::Release()
{
	delete this;
}

		
const CRESULT CR_ShaderConstHandle::GetValue(void *dest)
{
	if (shader != nullptr)
	{
		return shader->ReadUniform(index,dest);
	}

	return CE_ERR_FAIL;
}

	
const CRESULT CR_ShaderConstHandle::SetValue(void *source)
{
	if (shader != nullptr)
	{
		return shader->WriteUniform(index,source);			
	}

	return CE_ERR_FAIL;
}