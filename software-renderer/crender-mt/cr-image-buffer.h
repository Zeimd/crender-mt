/*****************************************************************************
*
* cr-image-buffer.h
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#ifndef _CENG_CR_IMAGE_BUFFER_H
#define _CENG_CR_IMAGE_BUFFER_H

#include <ceng/interfaces/image-buffer.h>

#include "crender-base.h"


namespace Ceng
{
	class CR_NewTargetData;

	class CR_ImageBuffer : public Ceng::ImageBuffer
	{
	public:

		CR_NewTargetData *renderTarget;

	public:

		CR_ImageBuffer();
		virtual ~CR_ImageBuffer();

		virtual void Release() override;

		virtual const CRESULT ToFrameBuffer(const Ceng::Rectangle &sourceArea,
										const Ceng::BufferData &destBuffer);

		virtual const Ceng::UINT32 GetWidth();
		virtual const Ceng::UINT32 GetHeight();
	};
};

#endif