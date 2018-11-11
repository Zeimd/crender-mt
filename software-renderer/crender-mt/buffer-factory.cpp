/*****************************************************************************
*
* buffer-factory.cpp
*
* By Jari Korkala 10/2012
*
*****************************************************************************/

#include <ceng/datatypes/tex2d-desc.h>

#include <ceng/enums/buffer-binding.h>

#include "buffer-factory.h"

#include "rtarget-data.h"

using namespace Ceng;

CR_BufferFactory::CR_BufferFactory()
{
	vectorAlign = 16;
	cacheLine = 16;
	pageSize = 4096;
	maxThreads = 1;

	rasterTileSize = 8;
}

CR_BufferFactory::~CR_BufferFactory()
{
}

CRESULT CR_BufferFactory::SetSystemData(Ceng::UINT32 nCacheLine, 
										Ceng::UINT32 nPageSize, 
										Ceng::UINT32 nVectorAlign,
										Ceng::UINT32 nMaxThreads, 
										Ceng::UINT32 nTileSize)
{
	cacheLine = nCacheLine;
	pageSize = nPageSize;
	vectorAlign = nVectorAlign;

	maxThreads = nMaxThreads;

	rasterTileSize = nTileSize;

	return CE_OK;
}

CRESULT CR_BufferFactory::GetRenderTarget(Ceng::UINT32 startWidth, 
										  Ceng::UINT32 startHeight, 
										  Ceng::UINT32 maxWidth, 
										  Ceng::UINT32 maxHeight, 
										  Ceng::IMAGE_FORMAT::value format,
										  Ceng::BUFFER_TILING::value tilingMode,
										  Ceng::UINT32 usage, 
										  Ceng::UINT32 options,
										  CR_NewTargetData **targetPtr)
{

	CRESULT cresult;

	// Set up channels

	CR_ChannelData channels[4];

	Ceng::UINT32 channelCount;

	cresult = GetChannelData(format, channels, &channelCount);

	if (cresult != CE_OK)
	{
		return cresult;
	}

	UINT32 k;

	Ceng::BUFFER_LAYOUT::value layout;

	layout = Ceng::BUFFER_LAYOUT::AOS;
	
	if (channelCount == 2)
	{
		layout = Ceng::BUFFER_LAYOUT::SOA;
	}

	// Configure tiling

	UINT32 bytesPerPixel = 0;
	
	for(k=0;k<channelCount;k++)
	{
		bytesPerPixel += channels[k].bytesPerPixel;
	}

	UINT32 blockSize;
	
	UINT32 tileWidth,tileHeight;

	UINT32 linearPixels;

	UINT32 columns; // Number of linear blocks in x-direction
	UINT32 rows; // Number of linear blocks in y-direction

	switch(tilingMode)
	{
	case Ceng::BUFFER_TILING::SCANLINES:

		linearPixels = maxWidth;

		columns = 1;
		rows = maxHeight;
		
		tileWidth = maxWidth;
		tileHeight = 1;

		break;
	case Ceng::BUFFER_TILING::LQUADS:

		linearPixels = 2*(maxWidth + (maxWidth % 2));

		columns = 1;

		rows = maxHeight / 2;

		if (maxHeight % 2)
		{
			rows++;
		}
		/*
		if (rows % 2)
		{
			rows++;
		}
		*/

		tileWidth = 2;
		tileHeight = 2;

		break;
	case Ceng::BUFFER_TILING::TILED4:

		linearPixels = 16;

		columns = maxWidth / 4;
		if (maxWidth % 4)
		{
			++columns;
		}

		rows = maxHeight / 4;
		if (maxHeight % 4)
		{
			++rows;
		}

		tileWidth = 4;
		tileHeight = 4;

		break;
	case Ceng::BUFFER_TILING::TILED8:

		linearPixels = 64;

		columns = maxWidth / 8;
		if (maxWidth % 8)
		{
			++columns;
		}

		rows = maxHeight / 8;
		if (maxHeight % 8)
		{
			++rows;
		}

		tileWidth = 8;
		tileHeight = 8;

		break;
	case Ceng::BUFFER_TILING::TILED16:

		linearPixels = 256;

		columns = maxWidth / 16;
		if (maxWidth % 16)
		{
			++columns;
		}

		rows = maxHeight / 16;
		if (maxHeight % 16)
		{
			++rows;
		}

		tileWidth = 16;
		tileHeight = 16;

		break;
	case Ceng::BUFFER_TILING::TILED8_LQUADS:

		linearPixels = 64;

		columns = maxWidth / 8;
		if (maxWidth % 8)
		{
			columns++;
		}

		rows = maxHeight / 8;
		if (maxHeight % 8)
		{
			rows++;
		}
		
		tileWidth = 8;
		tileHeight = 8;
		
		break;
	case Ceng::BUFFER_TILING::TILED16_LQUADS:

		return CE_ERR_NOT_SUPPORTED;
		
		if (rasterTileSize < 16)
		{
			return CE_ERR_NOT_SUPPORTED;
		}

		linearPixels = 256;

		columns = maxWidth / 16;
		if (columns % 16)
		{
			columns++;
		}

		rows = maxHeight / 16;
		if (rows % 16)
		{
			rows++;
		}

		tileWidth = 16;

		break;
	default:
		return CE_ERR_INVALID_PARAM;
	}

	// Calculate channel offsets and the step to next horizontal linear unit

	UINT32 channelStride;
	UINT32 linearStep = 0; // Address step to next linear block

	for(k=0;k<channelCount;k++)
	{	
		channels[k].bufferOffset = linearStep;
		
		channelStride = channels[k].bytesPerPixel * linearPixels;
		
		// Pad channel stride to vector unit load/store alignment
		if (channelStride % vectorAlign)
		{
			channelStride += vectorAlign - (channelStride % vectorAlign);
		}

		channels[k].linearBytes = channelStride;
		
		linearStep += channelStride;
	}

	// Calculate step to the linear unit below (y++ for scanlines)

	UINT32 rowBytes = columns * linearStep;

	// Pad to full cache lines so that sections assigned to different
	// processors won't share a cache line

//	if (usage & BufferBinding::render_target)
//	{
		if (rowBytes % cacheLine)
		{
			rowBytes += cacheLine - (rowBytes % cacheLine);
		}
//	}

	UINT32 allocSize = rowBytes * rows;

	UINT8 *allocPtr;

	allocPtr = (UINT8*)AlignedMalloc(allocSize,pageSize);
	if (allocPtr == NULL)
	{
		return CE_ERR_OUT_OF_MEMORY;
	}

	CR_NewTargetData *targetData;

	try
	{
		targetData = new CR_NewTargetData();
	}
	catch(std::bad_alloc &ba)
	{
		free(allocPtr);
		return CE_ERR_OUT_OF_MEMORY;
	}

	targetData->dataLayout = layout;
	targetData->tilingMode = tilingMode;
	
	targetData->bufferWidth = maxWidth;
	targetData->bufferHeight = maxHeight;

	targetData->windowWidth = startWidth;
	targetData->windowHeight = startHeight;

	targetData->baseAddress = (POINTER)allocPtr;

	targetData->bytesPerPixel = bytesPerPixel;

	targetData->rows = rows;
	targetData->columns = columns;
	targetData->linearPixels = linearPixels;

	// Configure address step variables
	switch(tilingMode)
	{
	case Ceng::BUFFER_TILING::SCANLINES:

		for(k=0;k<channelCount;k++)
		{
			channels[k].unitXstep = channels[k].bytesPerPixel; // Step a pixel right
			channels[k].unitYstep = rowBytes; // Step a scanline down
			
			channels[k].tileXstep = channels[k].bytesPerPixel; // Step a pixel right
			channels[k].tileYstep = rowBytes; // Step a scanline down
			
			// Scanlined mode uses different address stepping
			// for quads 
			channels[k].quadSteps[0] = 0;
			channels[k].quadSteps[1] = channels[k].bytesPerPixel;
			channels[k].quadSteps[2] = rowBytes;
			channels[k].quadSteps[3] = rowBytes + channels[k].bytesPerPixel;
		}

		break;
	case Ceng::BUFFER_TILING::LQUADS:

		for(k=0;k<channelCount;k++)
		{
			channels[k].unitXstep = 4*channels[k].bytesPerPixel; // Step a quad right
			channels[k].unitYstep = rowBytes; // Step a quad down
			
			channels[k].tileXstep = 4*channels[k].bytesPerPixel;
			channels[k].tileYstep = rowBytes;

			channels[k].quadSteps[0] = 0;
			channels[k].quadSteps[1] = channels[k].bytesPerPixel;
			channels[k].quadSteps[2] = 2*channels[k].bytesPerPixel;
			channels[k].quadSteps[3] = 3*channels[k].bytesPerPixel;
		}

		break;
	case Ceng::BUFFER_TILING::TILED4:
		for (k = 0; k<channelCount; k++)
		{
			channels[k].unitXstep = channels[k].bytesPerPixel; // Step a pixel right
			channels[k].unitYstep = 4 * channels[k].bytesPerPixel; // Step a pixel down

			channels[k].tileXstep = linearStep; // Step a tile right
			channels[k].tileYstep = rowBytes; // Step a tile down

			channels[k].quadSteps[0] = 0;
			channels[k].quadSteps[1] = channels[k].bytesPerPixel;
			channels[k].quadSteps[2] = 2 * channels[k].bytesPerPixel;
			channels[k].quadSteps[3] = 3 * channels[k].bytesPerPixel;
		}
		break;
	case Ceng::BUFFER_TILING::TILED8:
		for (k = 0; k<channelCount; k++)
		{
			channels[k].unitXstep = channels[k].bytesPerPixel; // Step a pixel right
			channels[k].unitYstep = 8 * channels[k].bytesPerPixel; // Step a pixel down

			channels[k].tileXstep = linearStep; // Step a tile right
			channels[k].tileYstep = rowBytes; // Step a tile down

			channels[k].quadSteps[0] = 0;
			channels[k].quadSteps[1] = channels[k].bytesPerPixel;
			channels[k].quadSteps[2] = 2 * channels[k].bytesPerPixel;
			channels[k].quadSteps[3] = 3 * channels[k].bytesPerPixel;
		}

		break;
	case Ceng::BUFFER_TILING::TILED16:
		for (k = 0; k<channelCount; k++)
		{
			channels[k].unitXstep = channels[k].bytesPerPixel; // Step a pixel right
			channels[k].unitYstep = 16 * channels[k].bytesPerPixel; // Step a pixel down

			channels[k].tileXstep = linearStep; // Step a tile right
			channels[k].tileYstep = rowBytes; // Step a tile down

			channels[k].quadSteps[0] = 0;
			channels[k].quadSteps[1] = channels[k].bytesPerPixel;
			channels[k].quadSteps[2] = 2 * channels[k].bytesPerPixel;
			channels[k].quadSteps[3] = 3 * channels[k].bytesPerPixel;
		}

		break;
	case Ceng::BUFFER_TILING::TILED8_LQUADS:

		for(k=0;k<channelCount;k++)
		{
			channels[k].unitXstep = 4*channels[k].bytesPerPixel; // Step a quad right
			channels[k].unitYstep = 16*channels[k].bytesPerPixel; // Step a quad down
			
			channels[k].tileXstep = linearStep; // Step a tile right
			channels[k].tileYstep = rowBytes; // Step a tile down

			channels[k].quadSteps[0] = 0;
			channels[k].quadSteps[1] = channels[k].bytesPerPixel;
			channels[k].quadSteps[2] = 2*channels[k].bytesPerPixel;
			channels[k].quadSteps[3] = 3*channels[k].bytesPerPixel;
		}
		break;
	case Ceng::BUFFER_TILING::TILED16_LQUADS:

		return CE_ERR_NOT_SUPPORTED;

		break;
	default:
		break;
	}

	targetData->tileXstep = channels[0].tileXstep;
	targetData->tileYstep = channels[0].tileYstep;

	targetData->uWrapStep = Ceng::INT32(bytesPerPixel) - targetData->tileYstep;
	targetData->vWrapStep = -Ceng::INT32(targetData->tileYstep)*Ceng::INT32(targetData->bufferHeight-1);

	targetData->channelCount = channelCount;
	memcpy(targetData->channels,channels,channelCount*sizeof(CR_ChannelData));

	targetData->shaderSemantic = -1;
	targetData->bufferFormat = format;

	if (usage & BufferBinding::render_target)
	{
		targetData->UpdateBucketAreas(maxThreads, rasterTileSize);
	}
	
	*targetPtr = targetData;

	return CE_OK;
}

const CRESULT CR_BufferFactory::GetTexture2D(const Texture2dDesc &desc,
	CR_NewTargetData **texturePtr)
{
	CRESULT cresult;

	return GetRenderTarget(desc.width, desc.height, desc.width, desc.height,
		desc.format, BUFFER_TILING::SCANLINES, 0, 0, texturePtr);
}

const CRESULT CR_BufferFactory::GetTiledTexture2D(const Texture2dDesc &desc,
	CR_NewTargetData **texturePtr)
{
	CRESULT cresult;

	return GetRenderTarget(desc.width, desc.height, desc.width, desc.height,
		desc.format, BUFFER_TILING::TILED4, 0, 0, texturePtr);
	
	/*
	return GetRenderTarget(desc.width, desc.height, desc.width, desc.height,
		desc.format, BUFFER_TILING::TILED8, 0, 0, texturePtr);
		
	return GetRenderTarget(desc.width, desc.height, desc.width, desc.height,
		desc.format, BUFFER_TILING::TILED16, 0, 0, texturePtr);
		*/
}

const CRESULT CR_BufferFactory::GetChannelData(const Ceng::IMAGE_FORMAT::value format,
	CR_ChannelData *out_channels,Ceng::UINT32 *channelCount)
{
	memset(out_channels, 0, 4 * sizeof(CR_ChannelData));

	*channelCount = 1;

	switch (format)
	{
	case Ceng::IMAGE_FORMAT::C32_ARGB:

		out_channels[0].semantic = Ceng::CHANNEL_SEMANTIC::FORMAT_COLOR;
		out_channels[0].dataType = Ceng::VECTOR_TYPE::INT8;
		out_channels[0].bytesPerPixel = 4;

		break;

	case Ceng::IMAGE_FORMAT::C24_RGB:

		out_channels[0].semantic = Ceng::CHANNEL_SEMANTIC::FORMAT_COLOR;
		out_channels[0].dataType = Ceng::VECTOR_TYPE::INT8;
		out_channels[0].bytesPerPixel = 3;

		break;
	case Ceng::IMAGE_FORMAT::D32F:

		out_channels[0].semantic = Ceng::CHANNEL_SEMANTIC::DEPTH;
		out_channels[0].dataType = Ceng::VECTOR_TYPE::FLOAT32;
		out_channels[0].bytesPerPixel = 4;

		break;
	case Ceng::IMAGE_FORMAT::D32F_S8:

		*channelCount = 2;

		out_channels[0].semantic = Ceng::CHANNEL_SEMANTIC::DEPTH;
		out_channels[0].dataType = Ceng::VECTOR_TYPE::FLOAT32;
		out_channels[0].bytesPerPixel = 4;

		out_channels[1].semantic = Ceng::CHANNEL_SEMANTIC::STENCIL;
		out_channels[1].dataType = Ceng::VECTOR_TYPE::INT8;
		out_channels[1].bytesPerPixel = 1;

		break;
	case Ceng::IMAGE_FORMAT::D24_S8:

		out_channels[0].semantic = Ceng::CHANNEL_SEMANTIC::FORMAT_DEPTH_STENCIL;
		out_channels[0].dataType = Ceng::VECTOR_TYPE::INT32;
		out_channels[0].bytesPerPixel = 4;
		out_channels[0].mixedData = true;

		break;
	default:
		return CE_ERR_NOT_SUPPORTED;
	};

	return CE_OK;
}

CRESULT CR_BufferFactory::CreateChannelData(Ceng::IMAGE_FORMAT::value format)
{
	/*
	outputStreams.clear();

	CR_BufferChannel tempChannel;

	switch(format)
	{
	case CE_FORMAT_C32_ARGB:

		outputStreams.push_back(CR_BufferStream());

		outputStreams[0].bufferOffset = 0;
		outputStreams[0].structSize = 4;
		outputStreams[0].

		tempChannel.dataType = CHANNEL_DATA::UINT;
		tempChannel.bits = 8;

		tempChannel.minimum = 0;
		tempChannel.maximum = 255;
		
		tempChannel.options = CHANNEL_OPTION::NORMALIZED |
			CHANNEL_OPTION::MIN_SATURATED | CHANNEL_OPTION::MAX_SATURATED;

		tempChannel.semantics = CHANNEL_SEMANTIC::BLUE;
		inputChannels.push_back(tempChannel);

		tempChannel.semantics = CHANNEL_SEMANTIC::GREEN;
		inputChannels.push_back(tempChannel);

		tempChannel.semantics = CHANNEL_SEMANTIC::RED;
		inputChannels.push_back(tempChannel);

		tempChannel.semantics = CHANNEL_SEMANTIC::ALPHA;
		inputChannels.push_back(tempChannel);
		
		break;
	case CE_FORMAT_DF32:

		tempChannel.semantics = CHANNEL_SEMANTIC::DEPTH;

		tempChannel.dataType = CHANNEL_DATA::FLOAT32;
		tempChannel.bits = 32;

		inputChannels.push_back(tempChannel);

		break;
	case CE_FORMAT_DF32_S8:

		tempChannel.semantics = CHANNEL_SEMANTIC::DEPTH;
		tempChannel.dataType = CHANNEL_DATA::FLOAT32;
		tempChannel.bits = 32;

		inputChannels.push_back(tempChannel);

		tempChannel.semantics = CHANNEL_SEMANTIC::STENCIL;
		tempChannel.dataType = CHANNEL_DATA::UINT;
		tempChannel.bits = 8;

		inputChannels.push_back(tempChannel);

		break;
	default:
		return CE_ERR_INVALID_PARAM;
	}
	*/

	return CE_OK;
}

