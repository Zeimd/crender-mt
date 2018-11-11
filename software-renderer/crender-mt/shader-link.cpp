/*****************************************************************************
*
* shader-link.cpp
*
* By Jari Korkala 5/2013
*
* -----------------------------------------------------------
*
* Jari Korkala 8/2014
*
* - Renamed to shader-link.cpp
*
*****************************************************************************/

#include "shader-link.h"

#include "cr-vertex-shader.h"
#include "cr-pixel-shader.h"

using namespace Ceng;

CR_ShaderLink::CR_ShaderLink()
{
	vertexShader = nullptr;
	pixelShader = nullptr;

	maxViewWidth = 0;
	maxViewHeight = 0;
}

CR_ShaderLink::CR_ShaderLink(const Ceng::UINT32 cacheLineSize)
{
	vertexShader = nullptr;
	pixelShader = nullptr;

	maxViewWidth = 0;
	maxViewHeight = 0;

	this->cacheLineSize = cacheLineSize;
}

CR_ShaderLink::~CR_ShaderLink()
{
	Clear();
}

void CR_ShaderLink::Clear()
{

}

CRESULT CR_ShaderLink::SetViewSize(const Ceng::UINT32 maxViewWidth,const Ceng::UINT32 maxViewHeight)
{
	if (this->maxViewWidth == maxViewWidth && this->maxViewHeight == maxViewHeight)
	{
		return CE_OK;
	}

	CRESULT cresult;

	cresult = SyncQuadCache();

	if (cresult != CE_OK)
	{
		return cresult;
	}

	this->maxViewWidth = maxViewWidth;
	this->maxViewHeight = maxViewHeight;

	return CE_OK;
}

CRESULT CR_ShaderLink::SyncQuadCache()
{
	// TODO: Pad width,height to nearest power of two

	quadCacheSize = (maxViewWidth >> 1)*(maxViewHeight>>1)*sizeof(CR_QuadHeader);


	return CE_OK;
}

CRESULT CR_ShaderLink::SetRenderTargets(const Ceng::UINT32 targets)
{
	CRESULT cresult;

	cresult = quadFormat.SetRenderTargets(targets);

	if (cresult != CE_OK)
	{
		return cresult;
	}

	cresult = SyncQuadCache();

	if (cresult != CE_OK)
	{
		return cresult;
	}

	if (pixelShader != nullptr)
	{
		cresult = pixelShader->SetFragmentFormat(&quadFormat);
	}

	return CE_OK;
}

CRESULT CR_ShaderLink::Configure(CR_VertexShader *vertexShader,CR_PixelShader *pixelShader)
{
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;

	if (this->vertexShader == nullptr || this->pixelShader == nullptr)
	{
		Log::Print("ShaderLink.Configure : don't have two shaders -> abort");
		return CE_OK;
	}

	// Create fragment format

	Clear();

	CRESULT cresult;

	cresult = fragmentFormat.Configure(vertexShader->outputSemantics,pixelShader->inputSemantics);

	if (cresult != CE_OK)
	{
		Log::Print("ShaderLink.Configure : fragment format configure failed");
		return cresult;
	}

	cresult = quadFormat.Configure(&fragmentFormat);

	if (cresult != CE_OK)
	{
		Log::Print("ShaderLink.Configure : quad format configure failed");
		return cresult;
	}

	cresult = vertexShader->SetFragmentFormat(&fragmentFormat);

	if (cresult != CE_OK)
	{
		Log::Print("ShaderLink.Configure : failed to sync vertex shader output");
		return cresult;
	}

	cresult = pixelShader->SetFragmentFormat(&quadFormat);

	if (cresult != CE_OK)
	{
		Log::Print("ShaderLink.Configure : failed to sync pixel shader input");
		return cresult;
	}

	clipperCacheSize = 10*fragmentFormat.size;

	//*******************************************************
	// Allocate space for a full screen of quads

	cresult = SyncQuadCache();

	if (cresult != CE_OK)
	{
		Log::Print("ShaderLink.Configure : failed to sync quad cache");
		return cresult;
	}

	return CE_OK;
}

AlignedBuffer<Ceng::UINT8>* CR_ShaderLink::GetClipperCache(const Ceng::UINT32 batchSize)
{
	return new AlignedBuffer<Ceng::UINT8>(batchSize*clipperCacheSize,cacheLineSize);
}

AlignedBuffer<Ceng::UINT8>* CR_ShaderLink::GetShaderStepBuffer()
{
	return new AlignedBuffer<Ceng::UINT8>(quadFormat.gradientBufferSize,cacheLineSize);
}

AlignedBuffer<Ceng::UINT8>* CR_ShaderLink::GetFragmentCache(const Ceng::UINT32 batchSize)
{
	return new AlignedBuffer<Ceng::UINT8>(batchSize*fragmentFormat.size,cacheLineSize);
}

AlignedBuffer<CR_FloatFragment> * CR_ShaderLink::GetFloatVariableBuffer()
{
	return new AlignedBuffer<CR_FloatFragment>(quadFormat.floatBlocks,cacheLineSize);
}

AlignedBuffer<CR_DoubleFragment> * CR_ShaderLink::GetDoubleVariableBuffer()
{
	return new AlignedBuffer<CR_DoubleFragment>(quadFormat.doubleBlocks,cacheLineSize);
}

AlignedBuffer<CR_QuadHeader>* CR_ShaderLink::GetQuadCache(const Ceng::UINT32 maxQuadCount)
{
	return new AlignedBuffer<CR_QuadHeader>(maxQuadCount*sizeof(CR_QuadHeader),cacheLineSize);
}
