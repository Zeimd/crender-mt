/*****************************************************************************
*
* buffer-factory.h
*
* By Jari Korkala 10/2012
*
*****************************************************************************/

#ifndef _CONCEPT2_BUFFER_FACTORY_H
#define _CONCEPT2_BUFFER_FACTORY_H

#include <ceng/enums/image-formats.h>

#include <vector>

#include "buffer-channel.h"

#include "buffer-tiling.h"

namespace Ceng
{
	class CR_NewTargetData;

	class CR_BufferFactory
	{
	protected:
		UINT32 cacheLine;
		UINT32 pageSize;
		UINT32 vectorAlign; // Alignment requirement for vector unit load/store

		UINT32 maxThreads;
		UINT32 rasterTileSize;

	public:
		CR_BufferFactory();
		~CR_BufferFactory();

		CRESULT SetSystemData(UINT32 nCacheLine,
								UINT32 nPageSize,
								UINT32 nVectorAlign,
								UINT32 nMaxThread,
								UINT32 rasterTileSize);

		CRESULT GetRenderTarget(UINT32 startWidth,
								UINT32 startHeight,
								UINT32 maxWidth,
								UINT32 maxHeight,
								Ceng::IMAGE_FORMAT::value format,
								Ceng::BUFFER_TILING::value tilingMode,
								UINT32 usage,
								UINT32 options,
								CR_NewTargetData **targetPtr);

		const CRESULT GetTexture2D(const Texture2dDesc &desc,
							 CR_NewTargetData **texturePtr);

		const CRESULT GetTiledTexture2D(const Texture2dDesc &desc, CR_NewTargetData **texturePtr);

	protected:
		CRESULT CreateChannelData(Ceng::IMAGE_FORMAT::value format);

		const CRESULT GetChannelData(const Ceng::IMAGE_FORMAT::value format, 
			CR_ChannelData *out_channels,Ceng::UINT32 *channelCount);
	};

} // namespace Ceng

#endif // Include guard