/*****************************************************************************
*
* d3d9-swap-chain.cpp
*
* Created By Jari Korkala 3/3/2015
*
*****************************************************************************/

#include "../include/d3d9-swap-chain.h"

using namespace Ceng;

Direct3D9_SwapChain::Direct3D9_SwapChain()
	: d3d_swapChain(nullptr),parentDevice(nullptr),windowed(true)
{
}

Direct3D9_SwapChain::~Direct3D9_SwapChain()
{
	if (d3d_swapChain != nullptr)
	{
		d3d_swapChain->Release();
	}
}

Direct3D9_SwapChain::Direct3D9_SwapChain(IDirect3DDevice9 *parentDevice, IDirect3DSwapChain9 *d3d_swapChain,
	WinAPI_Window *window, const Ceng::BOOL windowed,const DisplayMode &desktopMode)
	: parentDevice(parentDevice),d3d_swapChain(d3d_swapChain),windowed(windowed),window(window),
	desktopMode(desktopMode)
{
}

void Direct3D9_SwapChain::Release()
{
	delete this;
}

const CRESULT Direct3D9_SwapChain::Present()
{
	if (windowed)
	{

	}
	else
	{
		HRESULT hr = parentDevice->Present(NULL, NULL, NULL, NULL);
	}

	return CE_OK;
}

const CRESULT Direct3D9_SwapChain::Present(const Ceng::UINT32 syncInterval, const Ceng::UINT32 options,
	const Ceng::UINT32 dirtyRectCount,
	const Ceng::Rectangle *dirtyRects,
	const Ceng::Rectangle *scrollRect,
	const Ceng::UINT32 scrollOffsetX,
	const Ceng::UINT32 scrollOffsetY)
{
	return CE_OK;
}