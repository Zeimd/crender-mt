/*****************************************************************************
*
* win32-crend-d3d10.h
*
* By Jari Korkala 10/2012
*
*
* class ConceptRenderer_Core : 
*
* Software renderer core interface that uses Direct3D 10 as output.
*
* Most important difference to Direct3D_Core is that this returns the 
* Concept Engine software rasterizer object.
*
*****************************************************************************/

#ifndef _CONCEPT2_WIN32_CREND_D3D10_H
#define _CONCEPT2_WIN32_CREND_D3D10_H

#include <d3d10.h>

#include <ceng/interfaces/graphics-system.h>

#include "win32-dll.h"

namespace Ceng
{
	class CPU_Info;

	class Graphics2D;

	class ConceptRenderer_D3D10 : public GraphicsSystem
	{
	protected:
		CPU_Info *cpuData;

		IDXGIFactory *dxgiFactory;

		IDXGIAdapter *displayAdapter;
		IDXGIOutput *adapterOutput;

		IDXGISwapChain *swapChain;

		ID3D10Device *device;

		HMODULE crenderHandle;
	
		int (*call_GetRenderDevice)(UINT32 cacheLine,UINT32 threads,
									Graphics2D *displayPtr,
									SwapChainDesc *parameters,
									Ceng::RenderDevice **device);

	public:
		ConceptRenderer_D3D10();
		ConceptRenderer_D3D10(CPU_Info *sourceCPUdata);
		
		virtual ~ConceptRenderer_D3D10();

		virtual void Release() override;

		const CRESULT Initialize();
		
		virtual const CRESULT GetRenderDevice(GraphicsAdapter *adapter,
										const Ceng::UINT64 deviceOptions,
										SwapChainDesc *parameters,
										SwapChain **swapChainPtr,
										RenderDevice **devicePtr,
										RenderContext **contextPtr) override;

	protected:

		/**
		 * Return the object to construct-time status after a critical error
		 * is encountered.
		 */
		void Clean();
		
		
	};

} // Namespace Ceng

#endif // Include guard