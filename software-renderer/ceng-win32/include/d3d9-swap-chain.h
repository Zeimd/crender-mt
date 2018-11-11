/*****************************************************************************
*
* d3d9-swap-chain.h
*
* Created By Jari Korkala 3/3/2015
*
*****************************************************************************/

#ifndef CENG_D3D9_SWAP_CHAIN_H
#define CENG_D3D9_SWAP_CHAIN_H

#include <d3d9.h>
#include <d3d9types.h>

#include <ceng/datatypes/boolean.h>
#include <ceng/interfaces/swap-chain.h>

#include <ceng/interfaces/display-mode.h>

namespace Ceng
{
	class WinAPI_Window;

	class Direct3D9_SwapChain : public SwapChain
	{
	protected:

		IDirect3DDevice9 *parentDevice;
		IDirect3DSwapChain9 *d3d_swapChain;

		Ceng::BOOL windowed;
		WinAPI_Window *window;

		DisplayMode desktopMode;

	public:

		Direct3D9_SwapChain();
		
		Direct3D9_SwapChain(IDirect3DDevice9 *parentDevice,IDirect3DSwapChain9 *d3d_swapChain,
								WinAPI_Window *window,const Ceng::BOOL windowed,
								const DisplayMode &desktopMode);

		virtual ~Direct3D9_SwapChain();

		virtual void Release() override;

		virtual const CRESULT Present() override;

		virtual const CRESULT Present(const Ceng::UINT32 syncInterval, const Ceng::UINT32 options,
			const Ceng::UINT32 dirtyRectCount,
			const Ceng::Rectangle *dirtyRects,
			const Ceng::Rectangle *scrollRect,
			const Ceng::UINT32 scrollOffsetX,
			const Ceng::UINT32 scrollOffsetY) override;
	};
}

#endif
