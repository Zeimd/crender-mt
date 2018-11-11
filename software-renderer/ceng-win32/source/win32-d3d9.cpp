/*****************************************************************************
*
* win32-d3d9.cpp
*
* By Jari Korkala 10/2012
*
*****************************************************************************/

#include "../include/win32-d3d9.h"
#include "../include/d3d9-adapter.h"

#ifdef UNICODE
#define D3D9_DLL L"d3d9.dll"
#else
#define D3D9_DLL "d3d9.dll"
#endif

using namespace Ceng;

Direct3D_Core* Direct3D_Core::GetInstance()
{
	
	DLL_Handle *dll = DLL_Handle::GetInstance(D3D9_DLL);

	if (dll == nullptr)
	{
		return nullptr;
	}

	DynamicCallback call = dll->GetFunction("Direct3DCreate9");

	if (call == NULL)
	{
		dll->Release();
		return nullptr;
	}

	typedef IDirect3D9* (__stdcall *create_d3d9)(::UINT);

	create_d3d9 function = (create_d3d9)call;


	IDirect3D9 *core = (*function)(D3D_SDK_VERSION);

	if (core == nullptr)
	{
		dll->Release();
		return nullptr;
	}

	Direct3D_Core *temp = new Direct3D_Core(core,dll);

	return temp;
}

Direct3D_Core::Direct3D_Core() : d3d_core(nullptr), dll(nullptr)
{
}

Direct3D_Core::Direct3D_Core(LPDIRECT3D9 core,DLL_Handle *dll)
	: d3d_core(core),dll(dll)
{
}
		
Direct3D_Core::~Direct3D_Core()
{
	if (d3d_core != nullptr)
	{
		d3d_core->Release();
	}

	if (dll != nullptr)
	{
		dll->Release();
	}
}

void Direct3D_Core::Release()
{
	delete this;
}

const CRESULT Direct3D_Core::GetRenderDevice(GraphicsAdapter *adapter, 
											 const Ceng::UINT64 deviceOptions, 
											 SwapChainDesc *swapChainDesc, 
											 SwapChain **swapChainPtr,
											 RenderDevice **devicePtr,
											 RenderContext **contextPtr)
{
	if (adapter == nullptr)
	{
		return CE_ERR_INVALID_PARAM;
	}

	if (swapChainDesc == nullptr)
	{
		return CE_ERR_INVALID_PARAM;
	}

	D3DPRESENT_PARAMETERS d3d_params;

	CRESULT cresult = Direct3D_Core::TranslateSwapChainDesc(*swapChainDesc, d3d_params);

	if (cresult != CE_OK) return cresult;

	Direct3D9_Adapter *d3d_adapter = (Direct3D9_Adapter*)adapter;

	IDirect3DDevice9 *d3d_device = nullptr;

	::DWORD behaviorFlags = 0;

	HRESULT hr = d3d_core->CreateDevice(d3d_adapter->index, D3DDEVTYPE_HAL, d3d_params.hDeviceWindow,
		behaviorFlags, &d3d_params, &d3d_device);

	if (hr == D3DERR_NOTAVAILABLE)
	{
		return CE_ERR_NOT_SUPPORTED;
	}
	else if (hr == D3DERR_INVALIDCALL)
	{
		return CE_ERR_INVALID_PARAM;
	}
	else if (hr == D3DERR_OUTOFVIDEOMEMORY)
	{
		return CE_ERR_OUT_OF_VIDEO_MEMORY;
	}
	else if (hr == D3DERR_DEVICELOST)
	{
		// ?
	}


	return CE_OK;
}

LPDIRECT3D9 Direct3D_Core::GetCore()
{
	return d3d_core;
}

const Ceng::UINT32 Direct3D_Core::AdapterCount()
{
	return d3d_core->GetAdapterCount();
}

const Ceng::CRESULT Direct3D_Core::EnumAdapter(const Ceng::UINT32 index, GraphicsAdapter **out_adapter)
{
	D3DADAPTER_IDENTIFIER9 d3d_adapter;

	HRESULT hr = d3d_core->GetAdapterIdentifier(index,0,&d3d_adapter);

	if (hr != D3D_OK)
	{
		return CE_ERR_INVALID_PARAM;
	}

	Direct3D9_Adapter *temp = new Direct3D9_Adapter(index,d3d_adapter);

	*out_adapter = (GraphicsAdapter*)temp;

	return CE_OK;
}

const Ceng::UINT32 Direct3D_Core::CurrentDisplayMode(GraphicsAdapter *adapter, DisplayMode &out_mode)
{
	Direct3D9_Adapter *d3d_adapter = (Direct3D9_Adapter*)adapter;

	D3DDISPLAYMODE d3d_displayMode;

	HRESULT hr = d3d_core->GetAdapterDisplayMode(d3d_adapter->index, &d3d_displayMode);

	if (hr != D3D_OK)
	{
		return CE_ERR_INVALID_PARAM;
	}

	out_mode.format = ReverseMapFormat(d3d_displayMode.Format);
	out_mode.width = d3d_displayMode.Width;
	out_mode.height = d3d_displayMode.Height;
	out_mode.refreshHz = d3d_displayMode.RefreshRate;

	return Ceng::CE_OK;
}

const Ceng::UINT32 Direct3D_Core::DisplayModeCount(GraphicsAdapter *adapter, const Ceng::IMAGE_FORMAT::value format)
{
	Direct3D9_Adapter *d3d_adapter = (Direct3D9_Adapter*)adapter;

	D3DFORMAT d3d_format = MapFormat(format);

	return d3d_core->GetAdapterModeCount(d3d_adapter->index, d3d_format);
}

const Ceng::CRESULT Direct3D_Core::EnumDisplayMode(GraphicsAdapter *adapter, const Ceng::UINT32 modeIndex,
													const Ceng::IMAGE_FORMAT::value format, DisplayMode &out_mode)
{
	Direct3D9_Adapter *d3d_adapter = (Direct3D9_Adapter*)adapter;

	D3DFORMAT d3d_format = MapFormat(format);

	D3DDISPLAYMODE d3d_displayMode;

	HRESULT hr = d3d_core->EnumAdapterModes(d3d_adapter->index, d3d_format, modeIndex, &d3d_displayMode);

	if (hr == D3DERR_INVALIDCALL)
	{
		return CE_ERR_INVALID_PARAM;
	}

	if (hr == D3DERR_NOTAVAILABLE)
	{
		return CE_ERR_NOT_SUPPORTED;
	}

	out_mode.format = ReverseMapFormat(d3d_displayMode.Format);
	out_mode.width = d3d_displayMode.Width;
	out_mode.height = d3d_displayMode.Height;
	out_mode.refreshHz = d3d_displayMode.RefreshRate;

	return CE_OK;
}

const Ceng::CRESULT Direct3D_Core::CheckFramebufferSupport(GraphicsAdapter *adapter,
															const Ceng::IMAGE_FORMAT::value displayFormat,
															const Ceng::IMAGE_FORMAT::value backBufferFormat,
															const Ceng::BOOL windowed)
{
	Direct3D9_Adapter *d3d_adapter = (Direct3D9_Adapter*)adapter;

	::D3DFORMAT d3d_displayFormat = MapFormat(displayFormat);

	if (d3d_displayFormat == ::D3DFMT_UNKNOWN)
	{
		return CE_ERR_INVALID_PARAM;
	}

	::D3DFORMAT d3d_backFormat = MapFormat(backBufferFormat);

	if (d3d_backFormat == ::D3DFMT_UNKNOWN)
	{
		return CE_ERR_INVALID_PARAM;
	}

	::HRESULT hr = d3d_core->CheckDeviceType(d3d_adapter->index, 
		::D3DDEVTYPE_HAL, d3d_displayFormat, d3d_backFormat,::BOOL(windowed));

	if (hr == D3DERR_INVALIDCALL)
	{
		return CE_ERR_INVALID_PARAM;
	}

	if (hr == D3DERR_NOTAVAILABLE)
	{
		return CE_ERR_NOT_SUPPORTED;
	}

	return CE_OK;
}

const Ceng::CRESULT Direct3D_Core::CheckBufferSupport(GraphicsAdapter *adapter,
	const Ceng::IMAGE_FORMAT::value displayFormat,
	const Ceng::BufferType::value type,
	const Ceng::IMAGE_FORMAT::value bufferFormat,
	const Ceng::UINT32 binding,
	const Ceng::BufferUsage::value usage,
	const Ceng::UINT32 cpuAccess,
	const Ceng::UINT32 options)
{
	Direct3D9_Adapter *d3d_adapter = (Direct3D9_Adapter*)adapter;

	::D3DFORMAT d3d_displayFormat = MapFormat(displayFormat);

	if (d3d_displayFormat == ::D3DFMT_UNKNOWN)
	{
		return CE_ERR_INVALID_PARAM;
	}

	::D3DFORMAT d3d_bufferFormat = MapFormat(bufferFormat);

	if (d3d_bufferFormat == ::D3DFMT_UNKNOWN)
	{
		return CE_ERR_INVALID_PARAM;
	}

	::DWORD d3d_usage=0;
	::D3DRESOURCETYPE d3d_resType;

	switch (type)
	{
	case Ceng::BufferType::back_buffer:
		d3d_resType = D3DRTYPE_SURFACE;
		break;
	case Ceng::BufferType::texture_2d:
		d3d_resType = D3DRTYPE_TEXTURE;
		break;
	case Ceng::BufferType::cube_map:
		d3d_resType = D3DRTYPE_CUBETEXTURE;
		break;
	default:
		return CE_ERR_NOT_SUPPORTED;
	}

	if (binding & Ceng::BufferBinding::render_target && binding & Ceng::BufferBinding::depth_stencil)
	{
		return CE_ERR_NOT_SUPPORTED;
	}

	if (binding & Ceng::BufferBinding::render_target)
	{
		d3d_usage |= D3DUSAGE_RENDERTARGET;

		Ceng::BOOL test = IsDepthFormat(d3d_bufferFormat);

		if (test) return CE_ERR_NOT_SUPPORTED;
	}

	if (binding & Ceng::BufferBinding::depth_stencil)
	{
		d3d_usage |= D3DUSAGE_DEPTHSTENCIL;

		Ceng::BOOL test = IsDepthFormat(d3d_bufferFormat);

		if (!test) return CE_ERR_NOT_SUPPORTED;
	}

	if (usage & Ceng::BufferUsage::gpu_cpu_write)
	{
		d3d_usage |= D3DUSAGE_DYNAMIC;
	}

	if (options & Ceng::BufferOptions::generate_mip_maps)
	{
		d3d_usage |= D3DUSAGE_AUTOGENMIPMAP;
	}

	::HRESULT hr = d3d_core->CheckDeviceFormat(d3d_adapter->index, ::D3DDEVTYPE_HAL, d3d_displayFormat,
		d3d_usage, d3d_resType, d3d_bufferFormat);
													

	return CE_OK;
}

const CRESULT Direct3D_Core::CreateSwapChain(RenderDevice *device, SwapChainDesc &swapChainDesc,
	SwapChain **chainPtr)
{
	if (device == nullptr) return CE_ERR_INVALID_PARAM;

	D3DPRESENT_PARAMETERS params;

	CRESULT cresult = TranslateSwapChainDesc(swapChainDesc,params);

	if (cresult != CE_OK) return cresult;

	IDirect3DDevice9 *d3d_device = (IDirect3DDevice9*)device;

	IDirect3DSwapChain9 *d3d_chain = nullptr;

	HRESULT hr = d3d_device->CreateAdditionalSwapChain(&params, &d3d_chain);

	if (hr == D3DERR_OUTOFVIDEOMEMORY)
	{
		return CE_ERR_OUT_OF_VIDEO_MEMORY;
	}

	if (hr == D3DERR_INVALIDCALL)
	{
		return CE_ERR_INVALID_PARAM;
	}

	return CE_OK;
}


const Ceng::CRESULT Direct3D_Core::TranslateSwapChainDesc(Ceng::SwapChainDesc &swapChainDesc,
	D3DPRESENT_PARAMETERS &out_params)
{
	D3DFORMAT format = MapFormat(swapChainDesc.displayMode.format);

	if (format == D3DFMT_UNKNOWN) return CE_ERR_NOT_SUPPORTED;

	out_params.BackBufferWidth = swapChainDesc.displayMode.width;
	out_params.BackBufferHeight = swapChainDesc.displayMode.height;
	out_params.BackBufferFormat = format;
	out_params.FullScreen_RefreshRateInHz = swapChainDesc.displayMode.refreshHz;
	out_params.BackBufferCount = swapChainDesc.bufferCount;

	out_params.MultiSampleType = MultisampleType(swapChainDesc.multisampleDesc.count);

	out_params.EnableAutoDepthStencil = swapChainDesc.autoDepthStencil;
	out_params.AutoDepthStencilFormat = MapFormat(swapChainDesc.autoDepthStencilFormat);

	if (swapChainDesc.autoDepthStencil)
	{
		if (!IsDepthFormat(out_params.AutoDepthStencilFormat))
		{
			return CE_ERR_INVALID_PARAM;
		}
	}

	if (out_params.MultiSampleType == D3DMULTISAMPLE_FORCE_DWORD)
	{
		return CE_ERR_NOT_SUPPORTED;
	}

	out_params.MultiSampleQuality = swapChainDesc.multisampleDesc.quality;

	switch (swapChainDesc.swapEffect)
	{
	case Ceng::SwapEffect::discard:
		out_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
		break;
	case Ceng::SwapEffect::flip:
		out_params.SwapEffect = D3DSWAPEFFECT_FLIP;
		break;
	case Ceng::SwapEffect::copy:
		out_params.SwapEffect = D3DSWAPEFFECT_COPY;
		break;
	default:
		return CE_ERR_NOT_SUPPORTED;
	}


	switch (swapChainDesc.presentInterval)
	{
	case 0:
		out_params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		break;
	case 1:
		out_params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		break;
	case 2:
		out_params.PresentationInterval = D3DPRESENT_INTERVAL_TWO;
		break;
	case 3:
		out_params.PresentationInterval = D3DPRESENT_INTERVAL_THREE;
		break;
	case 4:
		out_params.PresentationInterval = D3DPRESENT_INTERVAL_FOUR;
		break;
	default:
		return CE_ERR_NOT_SUPPORTED;
	}
	

	::DWORD flags = 0;

	if (swapChainDesc.swapOptions & Ceng::SwapOptions::not_prerotated)
	{
		flags |= D3DPRESENTFLAG_NOAUTOROTATE;
	}

	if (swapChainDesc.swapOptions & Ceng::SwapOptions::restricted_content)
	{
		flags |= D3DPRESENTFLAG_RESTRICTED_CONTENT;
	}

	if (swapChainDesc.swapOptions & Ceng::SwapOptions::restrict_shared_resource)
	{
		flags |= D3DPRESENTFLAG_RESTRICT_SHARED_RESOURCE_DRIVER;
	}

	out_params.Flags = flags;

	WinAPI_Window *window = (WinAPI_Window*)swapChainDesc.outputWindow;

	out_params.Windowed = swapChainDesc.windowed;

	out_params.hDeviceWindow = window->GetWindowHandle();
	return CE_OK;
}


const D3DFORMAT Direct3D_Core::MapFormat(const Ceng::IMAGE_FORMAT::value format)
{
	switch (format)
	{
	case Ceng::IMAGE_FORMAT::C32_ARGB:
		return ::D3DFMT_A8R8G8B8;
	case Ceng::IMAGE_FORMAT::C32_XRGB:
		return ::D3DFMT_X8R8G8B8;

	case Ceng::IMAGE_FORMAT::C32_ABGR:
		return ::D3DFMT_A8B8G8R8;
	case Ceng::IMAGE_FORMAT::C32_XBGR:
		return ::D3DFMT_X8B8G8R8;

	case Ceng::IMAGE_FORMAT::C24_RGB:
		return ::D3DFMT_R8G8B8;

	case Ceng::IMAGE_FORMAT::C32_A2_BGR:
		return ::D3DFMT_A2B10G10R10;
	case Ceng::IMAGE_FORMAT::C32_A2_RGB:
		return ::D3DFMT_A2R10G10B10;

	case Ceng::IMAGE_FORMAT::C32_GR_16:
		return ::D3DFMT_G16R16;
		

	case Ceng::IMAGE_FORMAT::C16_ARGB:
		return ::D3DFMT_A4R4G4B4;
	case Ceng::IMAGE_FORMAT::C16_XRGB:
		return ::D3DFMT_X4R4G4B4;

	case Ceng::IMAGE_FORMAT::C16:
		return ::D3DFMT_R5G6B5;
	case Ceng::IMAGE_FORMAT::C16_A1:
		return ::D3DFMT_A1R5G5B5;
	case Ceng::IMAGE_FORMAT::C16_X1:
		return ::D3DFMT_X1R5G5B5;
	
	case Ceng::IMAGE_FORMAT::C64_ABGR:
		return ::D3DFMT_A16B16G16R16;

	case Ceng::IMAGE_FORMAT::CF32_ABGR:
		return ::D3DFMT_A32B32G32R32F;
	case Ceng::IMAGE_FORMAT::CF32_R:
		return ::D3DFMT_R32F;
	case Ceng::IMAGE_FORMAT::CF32_GR:
		return ::D3DFMT_G32R32F;

	case Ceng::IMAGE_FORMAT::CF16_ABGR:
		return ::D3DFMT_A16B16G16R16F;
	case Ceng::IMAGE_FORMAT::CF16_R:
		return ::D3DFMT_R16F;
	case Ceng::IMAGE_FORMAT::CF16_GR:
		return ::D3DFMT_G16R16F;

	case Ceng::IMAGE_FORMAT::ALPHA_8:
		return ::D3DFMT_A8;
	case Ceng::IMAGE_FORMAT::C16_A8RGB:
		return ::D3DFMT_A8R3G3B2;
	case Ceng::IMAGE_FORMAT::C8_RGB:
		return ::D3DFMT_R3G3B2;

	case Ceng::IMAGE_FORMAT::D24_S8:
		return ::D3DFMT_D24S8;
	case Ceng::IMAGE_FORMAT::D24_X8:
		return ::D3DFMT_D24X8;
	case Ceng::IMAGE_FORMAT::D24F_S8:
		return ::D3DFMT_D24FS8;

	case Ceng::IMAGE_FORMAT::D32F_LOCKABLE:
		return ::D3DFMT_D32F_LOCKABLE;

	case Ceng::IMAGE_FORMAT::D32_LOCKABLE:
		return ::D3DFMT_D32_LOCKABLE;

	case Ceng::IMAGE_FORMAT::D16_LOCKABLE:
		return ::D3DFMT_D16_LOCKABLE;

	case Ceng::IMAGE_FORMAT::D16:
		return ::D3DFMT_D16;
	case Ceng::IMAGE_FORMAT::D32:
		return ::D3DFMT_D32;
	case Ceng::IMAGE_FORMAT::D24_X4_S4:
		return ::D3DFMT_D24X4S4;

	case Ceng::IMAGE_FORMAT::D15_S1:
		return ::D3DFMT_D15S1;

	case Ceng::IMAGE_FORMAT::dxt1:
		return ::D3DFMT_DXT1;
	case Ceng::IMAGE_FORMAT::dxt2:
		return ::D3DFMT_DXT2;
	case Ceng::IMAGE_FORMAT::dxt3:
		return ::D3DFMT_DXT3;
	case Ceng::IMAGE_FORMAT::dxt4:
		return ::D3DFMT_DXT4;
	case Ceng::IMAGE_FORMAT::dxt5:
		return ::D3DFMT_DXT5;
	}

	return ::D3DFMT_UNKNOWN;
}

const Ceng::IMAGE_FORMAT::value Direct3D_Core::ReverseMapFormat(::D3DFORMAT format)
{
	switch (format)
	{
	case ::D3DFMT_A8R8G8B8:
		return Ceng::IMAGE_FORMAT::C32_ARGB;
	case ::D3DFMT_X8R8G8B8:
		return Ceng::IMAGE_FORMAT::C32_XRGB;

	case ::D3DFMT_A8B8G8R8:
		return Ceng::IMAGE_FORMAT::C32_ABGR;
	case ::D3DFMT_X8B8G8R8:
		return Ceng::IMAGE_FORMAT::C32_XBGR;

	case ::D3DFMT_R8G8B8:
		return Ceng::IMAGE_FORMAT::C24_RGB;

	case ::D3DFMT_A2B10G10R10:
		return Ceng::IMAGE_FORMAT::C32_A2_BGR;
	case ::D3DFMT_A2R10G10B10:
		return Ceng::IMAGE_FORMAT::C32_A2_RGB;

	case ::D3DFMT_G16R16:
		return Ceng::IMAGE_FORMAT::C32_GR_16;

	case ::D3DFMT_R5G6B5:
		return Ceng::IMAGE_FORMAT::C16;

	case ::D3DFMT_A4R4G4B4:
		return Ceng::IMAGE_FORMAT::C16_ARGB;
	case ::D3DFMT_X4R4G4B4:
		return Ceng::IMAGE_FORMAT::C16_XRGB;

	case ::D3DFMT_A1R5G5B5:
		return Ceng::IMAGE_FORMAT::C16_A1;

	case ::D3DFMT_X1R5G5B5:
		return Ceng::IMAGE_FORMAT::C16_X1;

	case ::D3DFMT_A16B16G16R16:
		return Ceng::IMAGE_FORMAT::C64_ABGR;

	case ::D3DFMT_A32B32G32R32F:
		return Ceng::IMAGE_FORMAT::CF32_ABGR;
	case ::D3DFMT_G32R32F:
		return Ceng::IMAGE_FORMAT::CF32_GR;
	case ::D3DFMT_R32F:
		return Ceng::IMAGE_FORMAT::CF32_R;

	case ::D3DFMT_A16B16G16R16F:
		return Ceng::IMAGE_FORMAT::CF16_ABGR;
	case ::D3DFMT_G16R16F:
		return Ceng::IMAGE_FORMAT::CF16_GR;
	case ::D3DFMT_R16F:
		return Ceng::IMAGE_FORMAT::CF16_R;

	case ::D3DFMT_A8:
		return Ceng::IMAGE_FORMAT::ALPHA_8;
	case ::D3DFMT_A8R3G3B2:
		return Ceng::IMAGE_FORMAT::C16_A8RGB;
	case ::D3DFMT_R3G3B2:
		return Ceng::IMAGE_FORMAT::C8_RGB;

	case ::D3DFMT_D24S8:
		return Ceng::IMAGE_FORMAT::D24_S8;
	case ::D3DFMT_D24X8:
		return Ceng::IMAGE_FORMAT::D24_X8;
	case ::D3DFMT_D24FS8:
		return Ceng::IMAGE_FORMAT::D24F_S8;

	case ::D3DFMT_D32F_LOCKABLE:
		return Ceng::IMAGE_FORMAT::D32F_LOCKABLE;
	case ::D3DFMT_D32_LOCKABLE:
		return Ceng::IMAGE_FORMAT::D32_LOCKABLE;
	case ::D3DFMT_D16_LOCKABLE:
		return Ceng::IMAGE_FORMAT::D16_LOCKABLE;

	case ::D3DFMT_D16:
		return Ceng::IMAGE_FORMAT::D16;

	case ::D3DFMT_D32:
		return Ceng::IMAGE_FORMAT::D32;

	case ::D3DFMT_D24X4S4:
		return Ceng::IMAGE_FORMAT::D24_X4_S4;
		
	case ::D3DFMT_D15S1:
		return Ceng::IMAGE_FORMAT::D15_S1;

	case ::D3DFMT_DXT1:
		return Ceng::IMAGE_FORMAT::dxt1;
	case ::D3DFMT_DXT2:
		return Ceng::IMAGE_FORMAT::dxt2;
	case ::D3DFMT_DXT3:
		return Ceng::IMAGE_FORMAT::dxt3;
	case ::D3DFMT_DXT4:
		return Ceng::IMAGE_FORMAT::dxt4;
	case ::D3DFMT_DXT5:
		return Ceng::IMAGE_FORMAT::dxt5;
	}

	return Ceng::IMAGE_FORMAT::UNKNOWN;
}

const Ceng::BOOL Direct3D_Core::IsDepthFormat(const ::D3DFORMAT format)
{
	switch (format)
	{
	case ::D3DFMT_D24S8:
	case ::D3DFMT_D24X8:
	case ::D3DFMT_D24FS8:
	case ::D3DFMT_D32F_LOCKABLE:
	case ::D3DFMT_D32_LOCKABLE:
	case ::D3DFMT_D16_LOCKABLE:
	case ::D3DFMT_D16:
	case ::D3DFMT_D32:
	case ::D3DFMT_D24X4S4:
	case ::D3DFMT_D15S1:
		return true;
	}

	return false;
}

const D3DMULTISAMPLE_TYPE Direct3D_Core::MultisampleType(const Ceng::UINT32 sampleCount)
{
	switch (sampleCount)
	{
	case 0:
		return D3DMULTISAMPLE_NONE;
	case 1:
		return D3DMULTISAMPLE_NONMASKABLE;
	case 2:
		return D3DMULTISAMPLE_2_SAMPLES;
	case 3:
		return D3DMULTISAMPLE_3_SAMPLES;
	case 4:
		return D3DMULTISAMPLE_4_SAMPLES;
	case 5:
		return D3DMULTISAMPLE_5_SAMPLES;
	case 6:
		return D3DMULTISAMPLE_6_SAMPLES;
	case 7:
		return D3DMULTISAMPLE_7_SAMPLES;
	case 8:
		return D3DMULTISAMPLE_8_SAMPLES;
	case 9:
		return D3DMULTISAMPLE_9_SAMPLES;
	case 10:
		return D3DMULTISAMPLE_10_SAMPLES;
	case 11:
		return D3DMULTISAMPLE_11_SAMPLES;
	case 12:
		return D3DMULTISAMPLE_12_SAMPLES;
	case 13:
		return D3DMULTISAMPLE_13_SAMPLES;
	case 14:
		return D3DMULTISAMPLE_14_SAMPLES;
	case 15:
		return D3DMULTISAMPLE_15_SAMPLES;
	case 16:
		return D3DMULTISAMPLE_16_SAMPLES;
	}

	return D3DMULTISAMPLE_FORCE_DWORD;
}