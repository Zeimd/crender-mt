/*****************************************************************************
*
* rendertarget-cr.cpp
*
* By Jari Korkala 6/2012
*
*****************************************************************************/

#include "rendertarget-cr.h"

using namespace Ceng;

CR_RenderTarget::CR_RenderTarget()
{
}

CR_RenderTarget::CR_RenderTarget(const std::shared_ptr<CR_NewTargetData> &newBuffer)
{
	bufferObject = newBuffer;
}

const std::shared_ptr<CR_NewTargetData> CR_RenderTarget::GetBufferObject()
{
	return bufferObject;
}

CR_NewTargetData* CR_RenderTarget::GetBufferPtr()
{
	return (CR_NewTargetData*)bufferObject.get();
}

CR_RenderTarget::~CR_RenderTarget()
{
}

void CR_RenderTarget::Release()
{
	delete this;
}

const BufferType::value CR_RenderTarget::GetResourceType()
{
	return BufferType::back_buffer;
}