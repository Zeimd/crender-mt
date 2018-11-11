/*****************************************************************************
*
* cmd-flip-buffers.cpp
*
* Created By Jari Korkala 4/10/2014
*
*****************************************************************************/

#include "api-command.h"

#include "crender-context.h"

#include "cr-swap-chain.h"

#include "cr-image-buffer.h"

#include <ceng/interfaces/graphics-2d.h>

using namespace Ceng;

Cmd_FlipBuffers::Cmd_FlipBuffers(const Ceng::UINT32 id, CR_SwapChain *swapChain)
	: ApiCommand(id),swapChain(swapChain)
{
}

Cmd_FlipBuffers::~Cmd_FlipBuffers()
{
}

const CRESULT Cmd_FlipBuffers::Execute(CR_RenderContext *renderer)
{
	CRESULT cresult;

	Ceng::CR_ImageBuffer frameBuffer;

	frameBuffer.renderTarget = swapChain->backBuffer.get();

	cresult = swapChain->outputDevice->ShowFrame(&frameBuffer);

	swapChain->presentReady->WakeAll();

	return cresult;
}

const CRESULT Cmd_FlipBuffers::Prepare(CR_RenderContext *renderer)
{
	return CE_OK;

	/*
	Ceng::UINT32 counter = renderer->pipeline.PixelShaderCounter();

	if (counter == id-1)
	{
		return CE_OK;
	}

	return CE_ERR_FAIL;
	*/
}