/*****************************************************************************
*
* win32-d3d9.h
*
* By Jari Korkala 10/2012
*
* class Direct3D_Renderer : Renderer core object for direct3d hardware
*                           acceleration
* 
*
*****************************************************************************/

#ifndef _CONCEPT2_WIN32_D3D9_H
#define _CONCEPT2_WIN32_D3D9_H

#include <d3d9.h>
#include <d3d9types.h>

#include <ceng/interfaces/graphics-system.h>

#include "win32-window.h"

#include "win32-dll.h"

namespace Ceng
{
	class Direct3D_Core : public GraphicsSystem
	{
	protected:
		
		LPDIRECT3D9 d3d_core; // Direct3D v9 object

		DLL_Handle *dll;
	
		Direct3D_Core();

		Direct3D_Core(LPDIRECT3D9 core,DLL_Handle *dll);

	protected:

	public:
		virtual ~Direct3D_Core();

		virtual void Release();

		static Direct3D_Core* GetInstance();

		virtual const Ceng::UINT32 AdapterCount() override;

		virtual const Ceng::CRESULT EnumAdapter(const Ceng::UINT32 index, GraphicsAdapter **out_adapter) override;

		virtual const Ceng::UINT32 CurrentDisplayMode(GraphicsAdapter *adapter,
														DisplayMode &out_mode) override;

		virtual const Ceng::UINT32 DisplayModeCount(GraphicsAdapter *adapter,
														const Ceng::IMAGE_FORMAT::value format) override;

		virtual const Ceng::CRESULT EnumDisplayMode(GraphicsAdapter *adapter, const Ceng::UINT32 index,
														const Ceng::IMAGE_FORMAT::value format,DisplayMode &out_mode) override;

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
														const Ceng::UINT32 options)  override;
		
		virtual const CRESULT CreateSwapChain(RenderDevice *device,
														SwapChainDesc &swapChainDesc,
														SwapChain **chainPtr) override;

		virtual const CRESULT GetRenderDevice(GraphicsAdapter *adapter,
										const Ceng::UINT64 deviceOptions,
										SwapChainDesc *swapChainDesc,
										SwapChain **swapChainPtr,
										RenderDevice **devicePtr,
										RenderContext **contextPtr) override;

		LPDIRECT3D9 GetCore();

		static const ::D3DFORMAT MapFormat(const Ceng::IMAGE_FORMAT::value format);

		static const Ceng::IMAGE_FORMAT::value ReverseMapFormat(::D3DFORMAT format);

		static const Ceng::BOOL IsDepthFormat(const ::D3DFORMAT format);

		static const D3DMULTISAMPLE_TYPE MultisampleType(const Ceng::UINT32 sampleCount);

		static const Ceng::CRESULT TranslateSwapChainDesc(Ceng::SwapChainDesc &swapChainDesc,
			D3DPRESENT_PARAMETERS &out_params);
	};

} // Namespace Ceng

#endif // Include guard