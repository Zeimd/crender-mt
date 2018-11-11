/*****************************************************************************
*
* win32-crend-d3d9.h
*
* By Jari Korkala 10/2012
*
*
* class ConceptRenderer_Core : 
*
* Software renderer core interface that uses Direct3D 9 as output.
*
* Most important difference to Direct3D_Core is that this returns the 
* Concept Engine software rasterizer object.
*
*****************************************************************************/

#ifndef _CONCEPT2_WIN32_CREND_D3D9_H
#define _CONCEPT2_WIN32_CREND_D3D9_H

#include <d3d9.h>

#include <ceng/interfaces/display-mode.h>

#include <ceng/interfaces/graphics-system.h>

#include "win32-d3d9.h"

#include "graphics-2d-d3d9.h"
#include "win32-dll.h"

//#include <ceng/interfaces/display-mode.h>

extern "C" typedef int (*RenderDeviceCall)(Ceng::CPU_Info*,Ceng::DisplayMode*,Ceng::Graphics2D*,
			Ceng::SwapChainDesc*,Ceng::SwapChain**,Ceng::RenderDevice**,Ceng::RenderContext**);
	
namespace Ceng
{
	class CPU_Info;

	class WindowsCriticalSection_x86;

	class ConceptRenderer_D3D9 : public GraphicsSystem
	{
	protected:
		CPU_Info *cpuData;

		Direct3D_Core *d3d_core;
			
		DLL_Handle * volatile crender;
		DLL_Handle * volatile crenderMT;
		

		DynamicCallback call_GetRenderDevice;
		DynamicCallback call_GetRenderDeviceMT;

		CriticalSection *crit_crender;
		CriticalSection *crit_crenderMT;

		ConceptRenderer_D3D9();
		ConceptRenderer_D3D9(Direct3D_Core *core,CPU_Info *cpuData,
								CriticalSection *crit_crender,
								CriticalSection *crit_crenderMT);
	
		virtual ~ConceptRenderer_D3D9();

	public:
				
		virtual void Release() override;

		static ConceptRenderer_D3D9* GetInstance(CPU_Info *cpuData);

		const CRESULT Initialize();

		virtual const Ceng::UINT32 AdapterCount() override;

		virtual const Ceng::CRESULT EnumAdapter(const Ceng::UINT32 index, GraphicsAdapter **out_adapter) override;

		virtual const Ceng::UINT32 CurrentDisplayMode(GraphicsAdapter *adapter,
			DisplayMode &out_mode) override;

		virtual const Ceng::UINT32 DisplayModeCount(GraphicsAdapter *adapter,
			const Ceng::IMAGE_FORMAT::value format) override;

		virtual const Ceng::CRESULT EnumDisplayMode(GraphicsAdapter *adapter, const Ceng::UINT32 index,
			const Ceng::IMAGE_FORMAT::value format, DisplayMode &out_mode) override;

		virtual const Ceng::CRESULT CheckFramebufferSupport(GraphicsAdapter *adapter,
			const Ceng::IMAGE_FORMAT::value displayFormat,
			const Ceng::IMAGE_FORMAT::value backBufferFormat,
			const Ceng::BOOL windowed) override;
	
		virtual const Ceng::CRESULT CheckBufferSupport(GraphicsAdapter *adapter,
			const Ceng::IMAGE_FORMAT::value displayFormat,
			const Ceng::BufferType::value type,
			const Ceng::IMAGE_FORMAT::value bufferFormat,
			const Ceng::UINT32 binding,
			const Ceng::BufferUsage::value usage,
			const Ceng::UINT32 cpuAccess,
			const Ceng::UINT32 options) override;

		virtual const CRESULT CreateSwapChain(RenderDevice *device,
												SwapChainDesc &swapChainDesc,
												SwapChain **chainPtr) override;

		virtual const CRESULT GetRenderDevice(GraphicsAdapter *adapter,
										const Ceng::UINT64 deviceOptions,
										SwapChainDesc *swapChainDesc,
										SwapChain **swapChainPtr,
										RenderDevice **devicePtr,
										RenderContext **contextPtr) override;
		
		
	};

} // Namespace Ceng

#endif // Include guard