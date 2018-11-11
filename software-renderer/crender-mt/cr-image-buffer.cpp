/*****************************************************************************
*
* cr-image-buffer.cpp
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#include "cr-image-buffer.h"

#include "rtarget-data.h"

using namespace Ceng;

CR_ImageBuffer::CR_ImageBuffer()
{
}

CR_ImageBuffer::~CR_ImageBuffer()
{
}

void CR_ImageBuffer::Release()
{
	delete this;
}

const Ceng::UINT32 CR_ImageBuffer::GetWidth()
{
	return renderTarget->windowWidth;
}

const Ceng::UINT32 CR_ImageBuffer::GetHeight()
{
	return renderTarget->windowHeight;
}

const CRESULT CR_ImageBuffer::ToFrameBuffer(const Ceng::Rectangle &sourceArea,
											const Ceng::BufferData &destBuffer)
{
	renderTarget->ToFrameBuffer(sourceArea,destBuffer);

	return CE_OK;
}