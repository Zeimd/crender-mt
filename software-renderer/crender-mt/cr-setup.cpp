/*****************************************************************************
*
* cr-setup.cpp
*
* By Jari Korkala 30/6/2011
*
* Core methods for software Concept Renderer.
*
*****************************************************************************/

#include <ceng/datatypes/cpu-info.h>

#include <ceng/platform.h>

#include <ceng/datatypes/release-deleter.h>

#include "crender.h"

#include "rasterizer-cr.h"

#include "command-thread.h"
#include "render-thread.h"

#include "crender-context.h"

#include "cr-swap-chain.h"

#include "cr-sampler-state.h"

#include "cr-texture-2d.h"

using namespace Ceng;

//***************************************************************************
// Concept Renderer methods

ConceptRenderer::ConceptRenderer()
{
	outputDevice = nullptr;
	deviceWindow = nullptr;

	renderContext = nullptr;

	renderTileSize = 8;
	cacheLineSize = 16;

	renderThreadCount = 1;

	debugLog.OpenFile("crender-debug.txt");

	//Log::SetLog(&debugLog);

	Log::Print("ConceptRenderer : constructor");
}

ConceptRenderer::~ConceptRenderer()
{
	if (outputDevice != nullptr)
	{
		outputDevice->Release();
	}

	Log::Print("ConceptRenderer : destuctor");
}

void ConceptRenderer::Release()
{
	delete this;
}

CRESULT ConceptRenderer::Configure(CPU_Info *cpuInfo,Graphics2D *devicePtr,
									DisplayMode *desktopMode,
									SwapChainDesc &swapChainDesc)
{
	Log::Print("Configure : method start");

	if (devicePtr == nullptr)
	{
		return CE_ERR_NULL_PTR;
	}

	this->cpuInfo = cpuInfo;

	cacheLineSize = cpuInfo->CacheLine();
	
	// TODO: Better evaluation of core count
	renderThreadCount = cpuInfo->LogicalCores() - 1;

	maxScreenSubBuckets = 2*renderThreadCount;

	bufferFactory.SetSystemData(cacheLineSize,4096,16,maxScreenSubBuckets,8);

	outputDevice = devicePtr;

	if (swapChainDesc.autoDepthStencil)
	{
		CRESULT cresult = AutoCreateDepthStencil(swapChainDesc,desktopMode);

		if (cresult != CE_OK) return cresult;
	}

	deviceWindow = swapChainDesc.outputWindow;

	memcpy(&outputParams,&swapChainDesc, sizeof(SwapChainDesc));
	memcpy(&this->desktopMode, desktopMode, sizeof(DisplayMode));

	renderContext = new CR_RenderContext();
	renderContext->Configure(this);

	maxViewWidth = swapChainDesc.displayMode.width;
	maxViewHeight = swapChainDesc.displayMode.height;

	if (swapChainDesc.windowed)
	{
		maxViewWidth = desktopMode->width;
		maxViewHeight = desktopMode->height;
	}


	return CE_OK;
}

const Ceng::CRESULT ConceptRenderer::GetContext(RenderContext **contextPtr)
{
	*contextPtr = (Ceng::RenderContext*)renderContext;

	return CE_OK;
}

const CRESULT ConceptRenderer::AutoCreateDepthStencil(SwapChainDesc &swapChainDesc,
	DisplayMode *desktopMode)
{
	Ceng::UINT32 bufferWidth = swapChainDesc.displayMode.width;
	Ceng::UINT32 bufferHeight = swapChainDesc.displayMode.height;

	if (swapChainDesc.windowed)
	{
		bufferWidth = desktopMode->width;
		bufferHeight = desktopMode->height;
	}

	UINT32 bufferUsage = BufferBinding::render_target;
	UINT32 bufferOptions = 0;

	CR_NewTargetData *targetData;

	CRESULT cresult = bufferFactory.GetRenderTarget(bufferWidth,
			bufferHeight,
			bufferWidth,
			bufferHeight,
			swapChainDesc.autoDepthStencilFormat,
			Ceng::BUFFER_TILING::TILED8_LQUADS,
			bufferUsage,
			bufferOptions,
			&targetData);

	if (cresult != CE_OK)
	{
		return cresult;
	}

	depthBuffer = std::shared_ptr<CR_NewTargetData>(targetData);

	return CE_OK;
}
						
const CRESULT ConceptRenderer::CreateSwapChain(SwapChainDesc &swapChainDesc, SwapChain **out_swapChain)
{
	CRESULT cresult;

	UINT32 bufferUsage = BufferBinding::render_target;
	UINT32 bufferOptions = 0;

	Ceng::UINT32 bufferWidth = swapChainDesc.displayMode.width;
	Ceng::UINT32 bufferHeight = swapChainDesc.displayMode.height;

	if (swapChainDesc.windowed)
	{
		bufferWidth = desktopMode.width;
		bufferHeight = desktopMode.height;
	}

	CR_NewTargetData *targetData;

	cresult = bufferFactory.GetRenderTarget(bufferWidth,
		bufferHeight,
		bufferWidth,
		bufferHeight,
		swapChainDesc.displayMode.format,
		Ceng::BUFFER_TILING::LQUADS,
		bufferUsage,
		bufferOptions,
		&targetData);

	targetData->dataLayout = Ceng::BUFFER_LAYOUT::SOA;

	if (cresult != CE_OK) return cresult;

	*out_swapChain = (SwapChain*)new CR_SwapChain(renderContext,outputDevice, targetData);

	return CE_OK;
}

//*************************************************************************
// Matrix methods

CRESULT ConceptRenderer::CreateProjectionMatrix(Ceng::UINT32 projDisplayWidth,
												Ceng::UINT32 projDisplayHeight,
												FLOAT32 horizontalField,
												FLOAT32 zNear,FLOAT32 zFar,
												Matrix4 *dest)
{
	if (dest == nullptr)
	{
		return CE_ERR_FAIL;
	}

	// Determine dilation

	if (horizontalField >= FLOAT32(180.0f) 
		|| horizontalField <= FLOAT32(0.0f))
	{
		horizontalField = FLOAT32(90.0f);
	}

	FLOAT32 dilation;
	FLOAT32 aspectRatio;

	dilation = FLOAT32(1.0f) / 
		tan(FLOAT32(0.5f)*horizontalField*degrees_to_radians);

	aspectRatio = (FLOAT32(projDisplayWidth) / FLOAT32(projDisplayHeight));

	FLOAT32 depth0,depth1;

	depth0 = zFar / (zFar - zNear);
	depth1 = -(zNear*zFar) / (zFar - zNear);

	// Fill the projection matrix

	dest->data[0][0] = FLOAT32(1.0f) * dilation;
	dest->data[0][1] = FLOAT32(0.0f);
	dest->data[0][2] = FLOAT32(0.0f); //-centerX / dilation;
	dest->data[0][3] = FLOAT32(0.0f);

	dest->data[1][0] = FLOAT32(0.0f);
	dest->data[1][1] = aspectRatio * dilation;
	dest->data[1][2] = FLOAT32(0.0f);//-centerY / dilation;
	dest->data[1][3] = FLOAT32(0.0f);

	dest->data[2][0] = FLOAT32(0.0f);
	dest->data[2][1] = FLOAT32(0.0f);
	dest->data[2][2] = -depth0;
	dest->data[2][3] = depth1;

	dest->data[3][0] = FLOAT32(0.0f);
	dest->data[3][1] = FLOAT32(0.0f);
	dest->data[3][2] = FLOAT32(-1.0f);
	dest->data[3][3] = FLOAT32(0.0f);

	return CE_OK;
}

const CRESULT ConceptRenderer::CreateSamplerState(const SamplerStateDesc &desc,
	Ceng::SamplerState **out_state)
{
	CR_SamplerState *temp = new CR_SamplerState(desc);

	*out_state = (CR_SamplerState*)temp;

	return CE_OK;
}

const CRESULT ConceptRenderer::CreateShaderResourceView(RenderResource *resource,
	ShaderResourceViewDesc &desc,
	ShaderResourceView **out_resourceView)
{
	switch (resource->GetResourceType())
	{
	case BufferType::texture_2d:

		CR_Texture2D *temp = (CR_Texture2D*)resource;

		return temp->GetShaderView(desc, out_resourceView);
	}


	return CE_OK;
}


const CRESULT ConceptRenderer::CreateTexture2D(const Texture2dDesc &desc,
	const SubResourceData *initialData, Ceng::Texture2D **texturePtr)
{
	TextureArrayVector textures(desc.arraySize);

	CRESULT cresult;

	Texture2dDesc localDesc;

	memcpy(&localDesc, &desc, sizeof(Texture2dDesc));

	IMAGE_FORMAT::value internalFormat = localDesc.format;

	switch (localDesc.format)
	{
	case IMAGE_FORMAT::C24_RGB:
		internalFormat = IMAGE_FORMAT::C32_ARGB;
		break;
	default:
		break;
	}

	localDesc.format = internalFormat;

	if (desc.mipLevels == 0)
	{
		localDesc.width = desc.width;
		localDesc.height = desc.height;

		localDesc.mipLevels = 1;
		
		while (localDesc.width > 1 || localDesc.height > 1)
		{
			if (localDesc.width > 1)
			{
				localDesc.width = localDesc.width >> 1;
			}

			if (localDesc.height > 1)
			{
				localDesc.height = localDesc.height >> 1;
			}

			++localDesc.mipLevels;
		}
	}

	for (Ceng::UINT32 k = 0; k < desc.arraySize; ++k)
	{
		localDesc.width = desc.width;
		localDesc.height = desc.height;

		for (Ceng::UINT32 j = 0; j < localDesc.mipLevels; ++j)
		{
			CR_NewTargetData *tempTexture;

			cresult = bufferFactory.GetTexture2D(localDesc, &tempTexture);

			if (j == 0 || !(desc.miscFlags & BufferOptions::generate_mip_maps))
			{
				const SubResourceData *data = &initialData[k*localDesc.mipLevels + j];

				CopyTextureData(tempTexture, data,desc.format);
			}

			textures[k].push_back(std::shared_ptr<CR_NewTargetData>(tempTexture));

			if (localDesc.width > 1)
			{
				localDesc.width = localDesc.width >> 1;
			}

			if (localDesc.height > 1)
			{
				localDesc.height = localDesc.height >> 1;
			}
		}
	}

	localDesc.width = desc.width;
	localDesc.height = desc.height;

	CR_Texture2D *tempTex = new CR_Texture2D(localDesc, std::move(textures));

	if (desc.miscFlags & BufferOptions::generate_mip_maps)
	{
		tempTex->GenerateMipMaps(0);
	}

	// Convert textures to tiled layout

	textures = TextureArrayVector(desc.arraySize);

	for (Ceng::UINT32 k = 0; k < desc.arraySize; ++k)
	{
		localDesc.width = desc.width;
		localDesc.height = desc.height;

		for (Ceng::UINT32 j = 0; j < localDesc.mipLevels; ++j)
		{
			CR_NewTargetData *tempTexture;

			cresult = bufferFactory.GetTiledTexture2D(localDesc, &tempTexture);

			CopyToTiled(tempTexture, tempTex->textures[k][j].get());

			textures[k].push_back(std::shared_ptr<CR_NewTargetData>(tempTexture));

			if (localDesc.width > 1)
			{
				localDesc.width = localDesc.width >> 1;
			}

			if (localDesc.height > 1)
			{
				localDesc.height = localDesc.height >> 1;
			}
		}
	}

	localDesc.width = desc.width;
	localDesc.height = desc.height;

	delete tempTex;

	CR_Texture2D *finalTex = new CR_Texture2D(localDesc, std::move(textures));

	*texturePtr = finalTex;

	return CE_OK;
}

const CRESULT ConceptRenderer::CopyToTiled(CR_NewTargetData *tiledTex, CR_NewTargetData *sourceTex)
{
	Ceng::UINT8 *sourceAddress = (Ceng::UINT8*)sourceTex->baseAddress;

	Ceng::UINT32 sourceOffset;

	for (Ceng::UINT32 y = 0; y < sourceTex->bufferHeight; ++y)
	{
		sourceOffset = sourceTex->tileYstep*y;

		for (Ceng::UINT32 x = 0; x < sourceTex->bufferWidth; ++x)
		{
			Ceng::UINT32 *destAddress = (Ceng::UINT32*)GetTiledAddress(tiledTex, x, y);

			*destAddress = *(Ceng::UINT32*)&sourceAddress[sourceOffset];

			sourceOffset += sourceTex->bytesPerPixel;
		}
	}

	return CE_OK;
}

const Ceng::UINT8* ConceptRenderer::GetTiledAddress(CR_NewTargetData *tiledTex,
	const Ceng::UINT32 u, const Ceng::UINT32 v) const
{
	
	Ceng::UINT32 tileX = u >> 2;
	Ceng::UINT32 posX = u & 3;

	Ceng::UINT32 tileY = v >> 2;
	Ceng::UINT32 posY = v & 3;
	
	
	/*
	Ceng::UINT32 tileX = u >> 3;
	Ceng::UINT32 posX = u & 7;

	Ceng::UINT32 tileY = v >> 3;
	Ceng::UINT32 posY = v & 7;
	
	Ceng::UINT32 tileX = u >> 4;
	Ceng::UINT32 posX = u & 15;

	Ceng::UINT32 tileY = v >> 4;
	Ceng::UINT32 posY = v & 15;
	*/

	Ceng::UINT8 *texelPtr = (Ceng::UINT8*)tiledTex->baseAddress;

	texelPtr += tiledTex->tileXstep*tileX;
	texelPtr += tiledTex->tileYstep*tileY;

	texelPtr += tiledTex->channels[0].unitYstep*posY;
	texelPtr += tiledTex->channels[0].unitXstep*posX;

	return texelPtr;
}

const CRESULT ConceptRenderer::CopyTextureData(CR_NewTargetData *texture, const SubResourceData *sourceData,
	const Ceng::IMAGE_FORMAT::value sourceFormat)
{
	Ceng::UINT8 *destAddress = (Ceng::UINT8*)texture->baseAddress;
	Ceng::UINT8 *sourceAddress = (Ceng::UINT8*)sourceData->sourcePtr;

	if (texture->bufferFormat == sourceFormat)
	{
		Ceng::UINT32 rowBytes = sourceData->rowPitch;

		for (Ceng::UINT32 k = 0; k < texture->bufferHeight; ++k)
		{
			memcpy(destAddress, sourceAddress, rowBytes);

			destAddress += texture->channels[0].tileYstep;
			sourceAddress += rowBytes;
		}
	}

	if (texture->bufferFormat == IMAGE_FORMAT::C32_ARGB && sourceFormat == IMAGE_FORMAT::C24_RGB)
	{
		Ceng::UINT32 rowBytes = sourceData->rowPitch;

		for (Ceng::UINT32 k = 0; k < texture->bufferHeight; ++k)
		{
			Ceng::UINT8 *tempSource = sourceAddress;
			Ceng::UINT8 *tempDest = destAddress;

			for (Ceng::UINT32 j = 0; j < texture->bufferWidth; ++j)
			{
				tempDest[0] = tempSource[0];
				tempDest[1] = tempSource[1];
				tempDest[2] = tempSource[2];
				tempDest[3] = 255;

				tempSource += 3;
				tempDest += texture->channels[0].bytesPerPixel;

				//memcpy(destAddress, sourceAddress, rowBytes);

			}

			destAddress += texture->channels[0].tileYstep;
			sourceAddress += rowBytes;
		}
	}

	return CE_OK;
}

