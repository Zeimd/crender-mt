/*****************************************************************************
*
* cr-scene.cpp
*
* By Jari Korkala 6/2012
*
* Concept Renderer's scene methods.
*
*****************************************************************************/

#include <ceng/platform.h>

#include <ceng/datatypes/condition-variable.h>
#include <ceng/datatypes/critical-section.h>

#include "crender.h"
#include "crender-context.h"

#include "rasterizer-cr.h"
#include "clipper.h"

#include "cr-vertex-shader.h"

#include "api-command.h"

#include "task-clipper.h"

#include "draw-batch.h"

using namespace Ceng;

/*
#define TOP_LEFT 0
#define TOP_RIGHT 1
#define BOTTOM_LEFT 2
#define BOTTOM_RIGHT 3
*/

const CRESULT CR_RenderContext::StartScene()
{
	SwapChainDesc &outputParams = parentDevice->outputParams;

	if (!outputParams.windowed)
	{
		windowWidth = parentDevice->maxViewWidth;
		windowHeight = parentDevice->maxViewHeight;

		parentDevice->outputDevice->ChangeWindowSize(windowWidth,windowHeight);
	}
	else
	{
		parentDevice->deviceWindow->GetClientArea(&windowWidth,&windowHeight);

		if (windowWidth > parentDevice->maxViewWidth)
		{
			return CE_ERR_FAIL;
		}
		if (windowHeight > parentDevice->maxViewHeight)
		{
			return CE_ERR_FAIL;
		}

		if (windowWidth != oldWindowWidth || windowHeight != oldWindowHeight)
		{
			parentDevice->outputDevice->ChangeWindowSize(windowWidth,windowHeight);

			oldWindowWidth = windowWidth;
			oldWindowHeight = windowHeight;
		}
	}

	if (nextRenderState->renderTargets[2] != nullptr)
	{
		nextRenderState->renderTargets[2]->ChangeWindowSize(windowWidth, windowHeight);
		nextRenderState->renderTargets[2]->UpdateBucketAreas(parentDevice->maxScreenSubBuckets, 8);
	}

	/*
	parentDevice->colorBuffer->ChangeWindowSize(windowWidth,windowHeight);
	parentDevice->colorBuffer->UpdateBucketAreas(parentDevice->maxScreenSubBuckets,8);
	*/

	if (parentDevice->depthBuffer != nullptr)
	{
		parentDevice->depthBuffer->ChangeWindowSize(windowWidth,windowHeight);
		parentDevice->depthBuffer->UpdateBucketAreas(parentDevice->maxScreenSubBuckets,8);
	}

	//**********************************************************************
	// TODO: These should be selected according to renderTarget[0]

	FLOAT32 halfWidth = FLOAT32(windowWidth) / FLOAT32(2.0f);
	FLOAT32 halfHeight = FLOAT32(windowHeight) / FLOAT32(2.0f);

	ViewportState &viewState = nextRenderState->viewportState;

	viewState.windowWidth = windowWidth;
	viewState.windowHeight = windowHeight;
	
	viewState.screenCenter.x = FLOAT32(0.0f); //halfWidth - pixelCenterOffsetX; 
	viewState.screenCenter.y = FLOAT32(0.0f); //halfHeight - pixelCenterOffsetY; 
	viewState.screenCenter.z = FLOAT32(0.0f);
	viewState.screenCenter.w = FLOAT32(0.0f);

	viewState.screenDilation.x = halfWidth;
	viewState.screenDilation.y = -halfHeight;
	viewState.screenDilation.z = FLOAT32(1.0f);
	viewState.screenDilation.w = FLOAT32(1.0f);

	viewState.rasterizerPort.left = -INT32( ceil(halfWidth) );
	viewState.rasterizerPort.top = -INT32( ceil(halfHeight) );
	
	viewState.rasterizerPort.right = windowWidth + viewState.rasterizerPort.left;
	viewState.rasterizerPort.bottom = windowHeight + viewState.rasterizerPort.top;

	/*
	activeClipper->ConfigureClipPlanes(-halfHeight,-halfWidth,
										halfHeight,halfWidth,
										-halfHeight,-halfWidth,
										halfHeight,halfWidth);
										*/

	activeClipper->CreateClipperState(-halfHeight,-halfWidth,
										halfHeight,halfWidth,
										-1024.0f,-1024.0f,
										1024.0f,1024.0f,
										nextRenderState->clipperState);

	shaderLink.SetViewSize(parentDevice->maxViewWidth, parentDevice->maxViewHeight);

	//*****************************************************

	//triangleCounter = 0;

	/*
	if (activeVertexShader == NULL)
	{
		return CE_WARNING_NO_VERTEX_SHADER;
	}

	if (activePixelShader == NULL)
	{
		return CE_WARNING_NO_PIXEL_SHADER;
	}
	*/

	scene = true;


	return CE_OK;
}

const CRESULT CR_RenderContext::EndScene()
{
	scene = false;

	return CE_OK;
}

const CRESULT CR_RenderContext::ClearTarget(Ceng::RenderTarget *renderTarget,const CE_Color &color)
{
	while(commandQueue.IsFull())
	{
	};

	CRESULT cresult;

	cresult = InstantiateRenderState();
	if (cresult != CE_OK)
	{
		return cresult;
	}

	commandQueue.PushBack(std::shared_ptr<ApiCommand>(
		new Cmd_ClearTarget(apiCallCounter,renderTarget,color,currentRenderState)));

	++apiCallCounter;

	return CE_OK;
}

const CRESULT CR_RenderContext::ClearDepth(const FLOAT32 depth)
{
	while(commandQueue.IsFull())
	{
	};

	CRESULT cresult;

	cresult = InstantiateRenderState();
	if (cresult != CE_OK)
	{
		return cresult;
	}

	commandQueue.PushBack(std::shared_ptr<ApiCommand>(
		new Cmd_ClearDepth(apiCallCounter,depth,currentRenderState)));

	++apiCallCounter;

	return CE_OK;
}


const Ceng::CRESULT CR_RenderContext::ClearDepthStencil(const FLOAT32 depth,const UINT32 stencil)
{
	while(commandQueue.IsFull())
	{
	};

	CRESULT cresult;

	cresult = InstantiateRenderState();
	if (cresult != CE_OK)
	{
		return cresult;
	}

	commandQueue.PushBack(std::shared_ptr<ApiCommand>(
		new Cmd_ClearDepthStencil(apiCallCounter,depth,stencil,currentRenderState)));

	++apiCallCounter;

	return CE_OK;
}

const Ceng::CRESULT CR_RenderContext::DrawPrimitive(const Ceng::PRIMITIVE_TYPE::value type,
	const Ceng::UINT32 firstVertex,const Ceng::UINT32 primitiveCount)
{
	while(commandQueue.IsFull())
	{
	};

	CRESULT cresult;

	cresult = InstantiateRenderState();
	if (cresult != CE_OK)
	{
		return cresult;
	}

	commandQueue.PushBack(std::shared_ptr<ApiCommand>(
		new Cmd_DrawPrimitive(apiCallCounter,type,firstVertex,primitiveCount,currentRenderState)));

	++apiCallCounter;

	return CE_OK;
}

const CRESULT CR_RenderContext::InstantiateRenderState()
{
	nextRenderState->depthBuffer = parentDevice->depthBuffer;

	nextRenderState->rasterizer = activeRasterizer;
	nextRenderState->clipper = activeClipper;

	// Change render state only when a draw call is issued.

	CRESULT cresult = nextRenderState->CommitState(*currentRenderState,&shaderLink,
		parentDevice->cacheLineSize,parentDevice->renderThreadCount);

	if (cresult != CE_OK)
	{
		return cresult;
	}

	currentRenderState = nextRenderState;

	nextRenderState = std::shared_ptr<RenderState>(new RenderState(*currentRenderState));

	return CE_OK;
}

const CRESULT CR_RenderContext::Execute_DrawPrimitive(const Ceng::UINT32 apiCallId,Ceng::PRIMITIVE_TYPE::value type,
													 const Ceng::UINT32 firstVertex,
													 const Ceng::UINT32 primitiveCount,
													 std::shared_ptr<RenderState> &renderState)
{
	CRESULT cresult;

	const Ceng::UINT32 vsBatchSize = 3;

	// Always allocate new draw batch when renderstate changes
	drawBatch = std::shared_ptr<DrawBatch>(new DrawBatch(apiCallId,vsBatchSize,renderState));

	// Vertex or index count

	Ceng::UINT32 vertexCount = 0;

	switch(type)
	{
	case PRIMITIVE_TYPE::TRIANGLE_LIST:
		vertexCount = 3*primitiveCount;
		break;
	case PRIMITIVE_TYPE::TRIANGLE_STRIP:
	case PRIMITIVE_TYPE::TRIANGLE_FAN:
		vertexCount = 2 + primitiveCount;
		break;
	}

	Ceng::INT32 first = firstVertex;
	Ceng::INT32 primitives = primitiveCount; // -1 = debug

	CR_PrimitiveData primitiveData;

	switch(type)
	{
	case PRIMITIVE_TYPE::TRIANGLE_FAN:

		primitiveData.primitiveType = PRIMITIVE_TYPE::TRIANGLE_LIST;

		// Add first vertex

		Ceng::INT32 cacheSlot = drawBatch->CacheAlloc(first);

		primitiveData.fragmentPtr[0] = drawBatch->CacheAbsoluteAddress(cacheSlot);

		Ceng::INT32 second = first+1;

		Ceng::INT32 index = first+1;

		// Add second vertex

		cacheSlot = drawBatch->CacheAlloc(index);

		primitiveData.fragmentPtr[1] = drawBatch->CacheAbsoluteAddress(cacheSlot);

		++index;

		while (primitives > 0)
		{
			// Add third vertex
		
			cacheSlot = drawBatch->CacheAlloc(index);

			primitiveData.fragmentPtr[2] = drawBatch->CacheAbsoluteAddress(cacheSlot);

			--primitives;

			drawBatch->primitiveList.push_back(primitiveData);

			if (primitives == 0)
			{
				break;
			}

			++index;

			if (drawBatch->GetEmptySpace() == 0)
			{
				// Split the fan into parts if current batch's vertex cache
				// is full

				pipeline.drawQueue.PushBack(drawBatch);
				
				drawBatch = std::shared_ptr<DrawBatch>(new DrawBatch(apiCallId,vsBatchSize,renderState));

				--index;
				
				cacheSlot = drawBatch->CacheAlloc(first);

				primitiveData.fragmentPtr[0] = drawBatch->CacheAbsoluteAddress(cacheSlot);

				cacheSlot = drawBatch->CacheAlloc(index);

				primitiveData.fragmentPtr[1] = drawBatch->CacheAbsoluteAddress(cacheSlot);

				++index;
			}
		}

		break;
	}

	// NOTE: Assumes batch size is at least 3 for triangles

	//if (drawBatch->GetEmptySpace() < 3)
	//{
		pipeline.drawQueue.PushBack(drawBatch);

		//drawBatch = std::shared_ptr<DrawBatch>(new DrawBatch(apiCallId,vsBatchSize,renderState));
	//}

	//**********************************************************************
	// Vertex shader stage
	
	while( !pipeline.drawQueue.IsEmpty() )
	{
		std::shared_ptr<DrawBatch> currentBatch = pipeline.drawQueue.Front();
	
		pipeline.drawQueue.PopFront();

		// Vertex shader

		
		if (currentBatch->renderState->vshaderInstance != nullptr)
		{
			cresult = currentBatch->renderState->vshaderInstance->ProcessVertexBatch(currentBatch,
																						&pipeline.clipper);
		
			if (cresult != CE_OK)
			{
				break;
			}
		}
		
	}
	
	std::shared_ptr<RenderTask> task;
	
	// Wait until render pipeline has been flushed

	pipeline.minThreadCount.store(1);
	pipeline.maxThreadCount.store(parentDevice->renderThreadCount);
	
	pipeline.WakeAllThreads();
	while(pipeline.remainingTasks.load() + pipeline.activeThreads.load() > 0)
	{		
		cmdProcessorSleep->Wait(cmdProcessor.wakeCrit);
	}
	
	pipeline.minThreadCount.store(0);
	pipeline.maxThreadCount.store(0);

	/*
	rendererHasWork->WakeAll();

	while (pipeline.remainingTasks.load() + pipeline.activeThreads.load() > 0)
	{
	}
	*/
	

	return CE_OK;

}

const CRESULT CR_RenderContext::Execute_ClearTarget(Ceng::RenderTarget *renderTarget,
												   const CE_Color &color,
												   std::shared_ptr<RenderState> &renderState)
{
	if (renderTarget == nullptr)
	{
		return CE_ERR_NULL_PTR;
	}

	CR_RenderTarget *accessPtr = (CR_RenderTarget*)renderTarget;

	CR_NewTargetData *target = accessPtr->GetBufferPtr();

	/*
	Rectangle scissor;

	if (renderState->rasterizerState.scissorEnable)
	{	
		scissor.top = renderState->scissorRect.top;
		scissor.left = renderState->scissorRect.left;
		
		scissor.bottom = renderState->scissorRect.bottom;
		scissor.right = renderState->scissorRect.right;
	}
	else
	{
		scissor.top = 0;
		scissor.left = 0;

		scissor.right = windowWidth;
		scissor.bottom = windowHeight;
	}
	*/

	return target->ClearTarget(color,&renderState->activeRect);
}

const CRESULT CR_RenderContext::Execute_ClearDepth(const FLOAT32 depth,std::shared_ptr<RenderState> &renderState)
{
	// Change render state only when a draw call is issued.

	if (renderState->depthBuffer == nullptr)
	{
		return CE_OK;
	}

	return parentDevice->depthBuffer->ClearDepth(depth);
}

const CRESULT CR_RenderContext::Execute_ClearDepthStencil(const FLOAT32 depth,
														 const Ceng::UINT32 stencil,
														 std::shared_ptr<RenderState> &renderState)
{
	if (renderState->depthBuffer == nullptr)
	{
		return CE_OK;
	}

	return parentDevice->depthBuffer->ClearDepthStencil(depth,stencil);
}

const CRESULT CR_RenderContext::SetScissorRect(const Ceng::Rectangle *rect)
{
	if (rect == nullptr)
	{
		return CE_ERR_NULL_PTR;
	}

	if (rect->bottom < 0)
	{
		return CE_ERR_INVALID_PARAM;
	}

	if (rect->right < 0)
	{
		return CE_ERR_INVALID_PARAM;
	}

	if (rect->right < rect->left)
	{
		return CE_ERR_INVALID_PARAM;
	}

	if (rect->bottom < rect->top)
	{
		return CE_ERR_INVALID_PARAM;
	}

	// TODO: Check against current render targets

	nextRenderState->scissorRect.top = rect->top;
	nextRenderState->scissorRect.left = rect->left;

	nextRenderState->scissorRect.bottom = rect->bottom;
	nextRenderState->scissorRect.right = rect->right;

	return CE_OK;
}

const CRESULT CR_RenderContext::SetRasterizerState(const Ceng::RasterizerState *state)
{
	if (state == nullptr)
	{
		return CE_ERR_NULL_PTR;
	}

	nextRenderState->rasterizerState = *state;

	return CE_OK;
}

const CRESULT CR_RenderContext::SetDepthStencilState(const Ceng::DepthStencilDesc *state)
{
	if (state == nullptr)
	{
		return CE_ERR_NULL_PTR;
	}

	nextRenderState->depthStencilState = *state;

	return CE_OK;
}

const CRESULT CR_RenderContext::SetRenderTarget(const Ceng::UINT32 targetNum,Ceng::RenderTarget *renderTarget)
{
	if (targetNum >= CRENDER_MAX_COLOR_TARGETS)
	{
		return CE_ERR_NOT_SUPPORTED;
	}

	CR_RenderTarget *targetPtr = (CR_RenderTarget*)renderTarget;

	if (renderTarget == nullptr)
	{
		nextRenderState->renderTargets[2+targetNum] = nullptr;
	}
	else if (targetPtr->GetBufferObject() == nextRenderState->renderTargets[2+targetNum])
	{
		return CE_OK;
	}
	else
	{
		// NOTE: targetHandles[0] = depthBuffer ,
		//       targetHandles[1] = stencilBuffer

		nextRenderState->renderTargets[2+targetNum] = targetPtr->GetBufferObject();
	}

	// Assign render targets to internal buffer so that color targets
	// are continuous in memory
	
	nextRenderState->activeTargets = 2;

	UINT32 k;

	for(k=0;k<CRENDER_MAX_COLOR_TARGETS;k++)
	{
		if (nextRenderState->renderTargets[k+2] != nullptr)
		{
			nextRenderState->renderTargets[k+2]->shaderSemantic = k+2;
			nextRenderState->activeTargets++;
		}
	}

	return CE_OK;
}


