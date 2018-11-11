/*****************************************************************************
*
* cr-swap-chain.cpp
*
* Created By Jari Korkala 3/3/2015
*
*****************************************************************************/

#include <ceng/platform.h>

#include "cr-swap-chain.h"
#include "crender-context.h"

#include "api-command.h"

#include "command-thread.h"

#include "rtarget-data.h"

#include "rendertarget-cr.h"

using namespace Ceng;

CR_SwapChain::CR_SwapChain()
{
}

CR_SwapChain::~CR_SwapChain()
{
	if (presentReady != nullptr)
	{
		presentReady->Release();
	}

	if (presentCaller != nullptr)
	{
		presentCaller->Release();
	}
}

void CR_SwapChain::Release()
{
	delete this;
}

CR_SwapChain::CR_SwapChain(CR_RenderContext *renderContext,Graphics2D *outputDevice, CR_NewTargetData *backBuffer)
	: renderContext(renderContext),outputDevice(outputDevice),backBuffer(backBuffer)
{
	Ceng_CreateConditionVar(&presentReady);
	Ceng_CreateCriticalSection(&presentCaller);
}

const CRESULT CR_SwapChain::Present()
{
	presentCaller->Lock();

	while (renderContext->commandQueue.IsFull())
	{
	};

	renderContext->commandQueue.PushBack(std::shared_ptr<ApiCommand>(new Cmd_FlipBuffers(renderContext->apiCallCounter,this)));

	++renderContext->apiCallCounter;

	// Wait until flipbuffers is executed

	presentReady->Wait(presentCaller);

	presentCaller->Unlock();

	return CE_OK;
}

const CRESULT CR_SwapChain::Present(const Ceng::UINT32 syncInterval, const Ceng::UINT32 options,
	const Ceng::UINT32 dirtyRectCount,
	const Ceng::Rectangle *dirtyRects,
	const Ceng::Rectangle *scrollRect,
	const Ceng::UINT32 scrollOffsetX,
	const Ceng::UINT32 scrollOffsetY)
{
	return CE_OK;
}



const Ceng::CRESULT CR_SwapChain::GetFrameBufferTarget(Ceng::RenderTarget **renderTarget)
{
	CR_RenderTarget *newTarget;

	try
	{
		newTarget = new CR_RenderTarget(backBuffer);
	}
	catch (std::bad_alloc &ba)
	{
		return CE_ERR_OUT_OF_MEMORY;
	}

	*renderTarget = (Ceng::RenderTarget*)newTarget;

	return CE_OK;
}