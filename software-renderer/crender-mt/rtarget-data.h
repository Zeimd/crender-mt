/*****************************************************************************
*
* rtarget-data.h
*
* By Jari Korkala 10/2012
*
* Concept Renderer: Internal render target information object
*
*****************************************************************************/

#ifndef _CONCEPT2_RTARGET_DATA_H
#define _CONCEPT2_RTARGET_DATA_H

#include <vector>

#include <ceng/datatypes/color-format.h>
#include <ceng/datatypes/rectangle.h>
#include <ceng/datatypes/bufferdata-2d.h>

#include <ceng/interfaces/image-buffer.h>
#include <ceng/interfaces/depth-stencil-desc.h>

#include "crender-base.h"
#include "buffer-channel.h"

#include "buffer-tiling.h"
#include "buffer-layout.h"


namespace Ceng
{
	
	
	//***************************************************
	// New render target data structure

	class CR_NewTargetData
	{
	public:
		UINT32 bufferWidth;
		UINT32 bufferHeight;

		UINT32 windowWidth; // Current target window size
		UINT32 windowHeight;

		UINT32 shaderSemantic; // Pixel shader output semantic
		Ceng::IMAGE_FORMAT::value bufferFormat; // Pixel shader output format

		// Either Array of Structs (AOS) or Struct of Arrays (SOA)
		Ceng::BUFFER_LAYOUT::value dataLayout; 

		// Determines how pixels are located in the buffer
		Ceng::BUFFER_TILING::value tilingMode;

		POINTER baseAddress;
		POINTER bytesPerPixel;

		UINT32 linearPixels; // Pixels in a linear unit
		
		UINT32 rows;
		UINT32 columns;

		UINT32 channelCount;
		CR_ChannelData channels[4];

		// Steps one tile in y-direction
		POINTER tileYstep; 

		// Steps one tile in x-direction
		POINTER tileXstep;

		UINT32 uWrapStep;
		UINT32 vWrapStep;

		struct LocalDepthStencilState
		{
			Ceng::BOOL depthEnable;
			Ceng::BOOL stencilEnable;
		};

		LocalDepthStencilState dsLocal;

		Ceng::DepthStencilDesc dsTest;

		std::vector<Rectangle> bucketAreas;

	public:
		CR_NewTargetData();
		~CR_NewTargetData();

		CRESULT ClearTarget(const CE_Color &color,const Ceng::Rectangle *scissorRect);

		CRESULT ClearDepth(const FLOAT32 depth);
		CRESULT ClearDepthStencil(const FLOAT32 depth,const UINT32 stencil);

		CRESULT ToFrameBuffer(const Ceng::Rectangle &sourceArea,
								const Ceng::BufferData &destBuffer);

		CRESULT BufferSwap_Scanlines(const UINT32 startX,const UINT32 startY,const UINT32 targetWidth,
								const UINT32 targetHeight,const Ceng::IMAGE_FORMAT::value destFormat,
								const POINTER destPitch,void *destBuffer);

		CRESULT BufferSwap_Lquads_AOS(const UINT32 startX,const UINT32 startY,const UINT32 targetWidth,
										const UINT32 targetHeight,const Ceng::IMAGE_FORMAT::value destFormat,
										const POINTER destPitch,void *destBuffer);

		CRESULT BufferSwap_Lquads_SOA(const Ceng::Rectangle &sourceArea,
										const Ceng::BufferData &destBuffer);

		CRESULT BufferSwap_Tiled8(const UINT32 startX,const UINT32 startY,const UINT32 width,const UINT32 height,
								const Ceng::IMAGE_FORMAT::value format,const POINTER bufferPitch,void *destBuffer);

		CRESULT SetDepthStencilState(const Ceng::DepthStencilDesc *state);

		void DepthStencilTestTile(const UINT32 tileSize,const UINT32 tileX,const UINT32 tileY,
									void *depthValues,void *depthStepX,void *depthStepY,
									UINT64 coverageMask,UINT64 *resultMask,
									Ceng::BOOL frontFace);

		POINTER GetTileAddress(const UINT32 tileSize,const UINT32 channel,const UINT32 tileX,const UINT32 tileY) const;

		POINTER GetQuadAddress(const UINT32 channel,const UINT32 x,const UINT32 y) const;

		POINTER GetQuadAddress(const UINT32 tileSize,const UINT32 channel,
									const UINT32 tileX,const UINT32 tileY,
									const UINT32 quadX,const UINT32 quadY) const;

		CRESULT ChangeWindowSize(const UINT32 newWidth,
									const UINT32 newHeight);
		
		CRESULT GetBufferData2D(Ceng::BufferData2D *destPtr);

		const CRESULT UpdateBucketAreas(const Ceng::UINT32 maxBucketCount,
										const Ceng::UINT32 tileSize);

	protected:

		void (*clear_bufferBlock)(void *bufferPtr,void *fillVec,POINTER vecAmount);
	};
	
	

} // namespace Ceng

#endif // Include guard