/*****************************************************************************
*
* rtarget-data.cpp
*
* By Jari Korkala 10/2012
*
*****************************************************************************/

#include "buffer-clear.h"
#include "rtarget-data.h"

using namespace Ceng;

CR_NewTargetData::CR_NewTargetData()
{
	baseAddress = NULL;
	
	bytesPerPixel = 0;
	tileYstep = 0;
	tileXstep = 0;

	memset(channels,0,4*sizeof(CR_ChannelData));

	shaderSemantic = 0;
	bufferFormat = Ceng::IMAGE_FORMAT::UNKNOWN;

	dataLayout = Ceng::BUFFER_LAYOUT::AOS;
	tilingMode = Ceng::BUFFER_TILING::SCANLINES;

	//clear_bufferBlock = &Ceng::CLEAR_BUFFER_BLOCK;
	clear_bufferBlock = &Ceng::CLEAR_BUFFER_BLOCK_X86_SSE;
}

CR_NewTargetData::~CR_NewTargetData()
{
	if (baseAddress != NULL)
	{
		AlignedFree((UINT8*)baseAddress);
		baseAddress = NULL;
	}
}

CRESULT CR_NewTargetData::ChangeWindowSize(const Ceng::UINT32 newWidth,
										   const Ceng::UINT32 newHeight)
{
	windowWidth = newWidth;
	windowHeight = newHeight;

	return CE_OK;
}

const CRESULT CR_NewTargetData::UpdateBucketAreas(const Ceng::UINT32 maxBucketCount,
												  const Ceng::UINT32 tileSize)
{
	if (maxBucketCount == 0)
	{
		return CE_ERR_INVALID_PARAM;
	}

	if (tileSize == 0)
	{
		return CE_ERR_INVALID_PARAM;
	}

	// Subdivision for multiple threads

	Ceng::UINT32 visRows = windowHeight / tileSize;

	Ceng::UINT32 buckets;

	buckets = maxBucketCount;
	if (visRows < maxBucketCount)
	{
		buckets = visRows;
	}

	Ceng::UINT32 threadRows = visRows / buckets;
	Ceng::UINT32 remainder = visRows % buckets;

	bucketAreas = std::vector<Rectangle>(buckets);

	Rectangle tempRect;

	tempRect.left = 0;
	tempRect.right = windowWidth-1;
	tempRect.top = 0;
	tempRect.bottom = windowHeight - 1;

	if (buckets > 2)
	{
		for (Ceng::UINT32 k = 0; k < buckets - 2; k++)
		{
			tempRect.top = k*threadRows*tileSize;
			tempRect.bottom = (k + 1)*threadRows*tileSize - 1;

			bucketAreas[k] = tempRect;
		}

		tempRect.top = tempRect.bottom + 1;
		tempRect.bottom = windowHeight - 1;
	}
		
	bucketAreas[buckets-1] = tempRect;

	return CE_OK;
}

CRESULT CR_NewTargetData::GetBufferData2D(Ceng::BufferData2D *destPtr)
{
	destPtr->width = windowWidth;
	destPtr->height = windowHeight;
	destPtr->format = bufferFormat;
	destPtr->bytesPerPixel = channels[0].bytesPerPixel;
	destPtr->pitchBytes = channels[0].tileYstep;
	destPtr->baseAddress = baseAddress;

	return CE_OK;
}

CRESULT CR_NewTargetData::SetDepthStencilState(const Ceng::DepthStencilDesc *state)
{
	memcpy(&dsTest,state,sizeof(Ceng::DepthStencilDesc));
	return CE_OK;
}

POINTER CR_NewTargetData::GetQuadAddress(const Ceng::UINT32 channel,
										 const Ceng::UINT32 x,const Ceng::UINT32 y) const
{
	POINTER temp = baseAddress + channels[channel].bufferOffset;

	switch(tilingMode)
	{
	case Ceng::BUFFER_TILING::SCANLINES:

		temp += x * channels[channel].unitXstep;
		temp += y * channels[channel].unitYstep;
		break;

	case Ceng::BUFFER_TILING::LQUADS:

		temp += (x>>1) * channels[channel].unitXstep;
		temp += (y>>1) * channels[channel].unitYstep;
		break;

	case Ceng::BUFFER_TILING::TILED8_LQUADS:

		temp += (x>>3) * channels[channel].tileXstep;
		
		temp += ((x&7)>>1) * channels[channel].unitXstep;

		temp += (y>>3) * channels[channel].tileYstep;

		temp += ((y&7)>>1) * channels[channel].unitYstep;

		break;
	};

	return temp;
}

POINTER CR_NewTargetData::GetQuadAddress(const Ceng::UINT32 tileSize,const Ceng::UINT32 channel,
										 const Ceng::UINT32 tileX,const Ceng::UINT32 tileY,
										 const Ceng::UINT32 quadX,const Ceng::UINT32 quadY) const
{
	POINTER temp = baseAddress;

	switch(tilingMode)
	{
	case BUFFER_TILING::SCANLINES:

		temp = baseAddress + channels[channel].bufferOffset;

		temp += ( (tileX<<3) + (quadX << 1)) * channels[channel].unitXstep;

		temp += ( (tileY<<3) + (quadY << 1)) * channels[channel].unitYstep;

		return temp;

		break;
	case BUFFER_TILING::LQUADS:

		temp = baseAddress + channels[channel].bufferOffset;

		temp += ((tileX<<2) + quadX) * channels[channel].unitXstep;

		temp += ((tileY<<2) + quadY) * channels[channel].unitYstep;

		return temp;

		break;
	case BUFFER_TILING::TILED8_LQUADS:

		temp = baseAddress + channels[channel].bufferOffset;

		temp += tileX * channels[channel].tileXstep + tileY * channels[channel].tileYstep;

		temp += quadX * channels[channel].unitXstep + quadY * channels[channel].unitYstep;

		return temp;

		break;

	case BUFFER_TILING::TILED16_LQUADS:

		return baseAddress;
		break;
	}

	return baseAddress;
}

POINTER CR_NewTargetData::GetTileAddress(const Ceng::UINT32 tileSize,const Ceng::UINT32 channel,
										 const Ceng::UINT32 tileX,const Ceng::UINT32 tileY) const
{
	POINTER temp;

	switch(tilingMode)
	{
	case BUFFER_TILING::SCANLINES:
		break;
	case BUFFER_TILING::LQUADS:
		break;
	case BUFFER_TILING::TILED8_LQUADS:

		temp = baseAddress + channels[channel].bufferOffset;

		temp += tileX * channels[channel].tileXstep + tileY * channels[channel].tileYstep;
		return temp;

		break;
	case BUFFER_TILING::TILED16_LQUADS:
		break;
	}

	return 0;
}