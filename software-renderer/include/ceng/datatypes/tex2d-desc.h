/*****************************************************************************
*
* tex2d-desc.h
*
* Created By Jari Korkala 5/3/2015
*
*****************************************************************************/

#ifndef CENG_TEX2D_DESC_H
#define CENG_TEX2D_DESC_H

#include "basic-types.h"

#include "../enums/image-formats.h"
#include "multisample-desc.h"

#include "../enums/buffer-usage.h"

namespace Ceng
{
	class Texture2dDesc
	{
	public:
		Ceng::UINT32 width;
		Ceng::UINT32 height;

		// Number of mipmap levels present per texture.
		// If set to 0, assume all levels are present.
		Ceng::UINT32 mipLevels;
		Ceng::UINT32 arraySize;

		Ceng::IMAGE_FORMAT::value format;
		Ceng::MultisampleDesc multisampleDesc;

		Ceng::BufferUsage::value usage;

		Ceng::UINT32 bindFlags;
		Ceng::UINT32 cpuAccessFlags;
		Ceng::UINT32 miscFlags;
	};
}

#endif