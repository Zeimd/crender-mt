/*****************************************************************************
*
* crender-context.cpp
*
* Created By Jari Korkala 2/3/2015
*
*****************************************************************************/

#include <ceng/platform.h>

#include <ceng/datatypes/condition-variable.h>
#include <ceng/datatypes/critical-section.h>

#include <ceng/datatypes/release-deleter.h>

#include "crender-context.h"

#include "render-state.h"

#include "command-thread.h"

#include "rasterizer-cr.h"
#include "crender.h"

#include "clipper.h"

#include "cr-vertex-shader.h"

#include "cr-vertex-buffer.h"

using namespace Ceng;

CR_RenderContext::CR_RenderContext()
{
	cmdThread = nullptr;

	activeRasterizer = nullptr;

	activeClipper = nullptr;

	scene = false;

	nextRenderState = std::shared_ptr<RenderState>(new RenderState(CRENDER_MAX_VERTEX_STREAMS,
		CRENDER_MAX_SHADER_TEXTURES));
}

CR_RenderContext::~CR_RenderContext()
{
	if (cmdThread != nullptr)
	{
		cmdThread->Release();
	}

	if (activeRasterizer != nullptr)
	{
		delete activeRasterizer;
		activeRasterizer = nullptr;
	}

	if (activeClipper != nullptr)
	{
		delete activeClipper;
		activeClipper = nullptr;
	}
}

void CR_RenderContext::Release()
{
	delete this;
}

const Ceng::CRESULT CR_RenderContext::Configure(ConceptRenderer *parentDevice)
{
	this->parentDevice = parentDevice;

	if (activeRasterizer == nullptr)
	{
		activeRasterizer = new CR_Rasterizer();
	}

	activeRasterizer->cacheLineSize = parentDevice->cacheLineSize;

	if (activeClipper == nullptr)
	{
		activeClipper = new CR_Clipper();
	}

	shaderLink = CR_ShaderLink(parentDevice->cacheLineSize);

	// Command queue

	apiCallCounter = 0;

	commandQueue = RingBuffer<std::shared_ptr<ApiCommand>>::Allocate(4096, parentDevice->cacheLineSize);

	// Command processor thread

	ConditionVariable *tempCond;

	CRESULT cresult = Ceng_CreateConditionVar(&tempCond);

	cmdProcessorSleep = std::shared_ptr<ConditionVariable>(tempCond, ReleaseDeleter<ConditionVariable>());

	cmdProcessor = CommandProcessor(this, cmdProcessorSleep);

	cresult = Ceng_CreateThread(&cmdProcessor, true, &cmdThread);

	pipeline.Configure(parentDevice->cacheLineSize,parentDevice->maxScreenSubBuckets,
		parentDevice->renderThreadCount, cmdProcessorSleep);


	pipeline.ResumeThreads();

	cmdThread->Resume();

	return CE_OK;
}

const CRESULT CR_RenderContext::SetPixelShader(Ceng::PixelShader *shaderPtr)
{
	if (shaderPtr == nullptr)
	{
		return CE_ERR_NULL_PTR;
	}

	nextRenderState->pixelShader = (CR_PixelShader*)shaderPtr;

	return CE_OK;
}


const CRESULT CR_RenderContext::SetVertexFormat(Ceng::VertexFormat *formatPtr)
{
	if (formatPtr == nullptr)
	{
		Log::Print("SetVertexFormat : nullptr input");
		return CE_ERR_NULL_PTR;
	}

	nextRenderState->vertexFormat = (CR_VertexFormat*)formatPtr;

	return CE_OK;
}

const CRESULT CR_RenderContext::SetVertexStream(Ceng::UINT32 index, Ceng::VertexBuffer *source)
{
	if (index >= CRENDER_MAX_VERTEX_STREAMS)
	{
		Log::Print("SetVertexStream : index out of range");
		return CE_ERR_PARAM;
	}

	if (source == nullptr)
	{
		Log::Print("SetVertexStream : nullptr input");
		return CE_ERR_NULL_PTR;
	}

	source->BufferPtr((UINT8**)&nextRenderState->vertexStreams[index].inputPtr);
	source->ElementSize(&nextRenderState->vertexStreams[index].elementSize);

	UINT32 tempCount;

	source->ElementCount(&tempCount);

	// Check that the new stream has at least as many elements
	// as stream 0.

	if (index != 0)
	{
		if (tempCount < nextRenderState->vertexStreams[0].elementCount)
		{
			Log::Print("SetVertexStream : element count mismatch");
			return CE_ERR_INCOMPATIBLE_FORMAT;
		}
	}

	nextRenderState->vertexStreams[index].elementCount = tempCount;

	return CE_OK;
}

const CRESULT CR_RenderContext::SetVertexShader(Ceng::VertexShader *shaderPtr)
{
	if (shaderPtr == NULL)
	{
		Ceng::Log::Print("Error : shaderPtr = NULL");
		return CE_ERR_NULL_PTR;
	}

	CR_VertexShader *accessPtr = (CR_VertexShader*)shaderPtr;

	nextRenderState->vertexShader = accessPtr;

	return CE_OK;
}

const Ceng::CRESULT CR_RenderContext::SetShaderResource(const Ceng::UINT32 index,
	ShaderResourceView *resource)
{
	nextRenderState->textureUnits[index].view = (CR_ShaderResourceView*)resource;

	return CE_OK;
}

const Ceng::CRESULT CR_RenderContext::SetSamplerState(const Ceng::UINT32 index,
	SamplerState *sampler)
{
	nextRenderState->textureUnits[index].sampler = (CR_SamplerState*)sampler;

	return CE_OK;
}