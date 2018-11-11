/*****************************************************************************
*
* win32-crend.cpp
*
* By Jari Korkala 10/2012
*
*****************************************************************************/

#include <sstream>

#include <ceng/enums/device-options.h>
#include <ceng/datatypes/cpu-info.h>

#include "../include/ceng-win32.h"

#include "../include/win32-crend-d3d9.h"

#include "../include/win32-window.h"

#ifdef _MSC_VER
	#ifdef UNICODE
		#define DEBUG_DLL L"crender-mt-debug.dll"
		#define RELEASE_DLL L"crender-mt.dll"
	#else
		#define DEBUG_DLL "crender-mt-debug.dll"
		#define RELEASE_DLL "crender-mt.dll"
	#endif

	#ifdef _DEBUG
		static const ::TCHAR *crender_mt_dll = DEBUG_DLL;
	#else
		static const ::TCHAR *crender_mt_dll = RELEASE_DLL;
	#endif
#endif

using namespace Ceng;

ConceptRenderer_D3D9* ConceptRenderer_D3D9::GetInstance(CPU_Info *cpuData)
{
	CRESULT cresult;

	CriticalSection *crit_crender;

	cresult = Ceng_CreateCriticalSection((CriticalSection**)&crit_crender);

	if (cresult != CE_OK)
	{
		return nullptr;
	}

	CriticalSection *crit_crenderMT;

	cresult = Ceng_CreateCriticalSection((CriticalSection**)&crit_crenderMT);

	if (cresult != CE_OK)
	{
		crit_crender->Release();
		return nullptr;
	}

	Direct3D_Core *core = Direct3D_Core::GetInstance();

	if (core == nullptr)
	{
		crit_crender->Release();
		crit_crenderMT->Release();
		return nullptr;
	}

	ConceptRenderer_D3D9 *temp = new ConceptRenderer_D3D9(core,cpuData,
															crit_crender,
															crit_crenderMT);

	return temp;
}

ConceptRenderer_D3D9::ConceptRenderer_D3D9(Direct3D_Core *core,CPU_Info *cpuData,
										   CriticalSection *crit_crender,
										   CriticalSection *crit_crenderMT)
	: d3d_core(core),cpuData(cpuData),crender(nullptr),crenderMT(nullptr),
	crit_crender(crit_crender),crit_crenderMT(crit_crenderMT)
{
}

ConceptRenderer_D3D9::ConceptRenderer_D3D9()
{
	d3d_core = nullptr;
	cpuData = nullptr;

	crender = nullptr;
	crenderMT = nullptr;

	crit_crender = nullptr;
	crit_crenderMT = nullptr;
}

ConceptRenderer_D3D9::~ConceptRenderer_D3D9()
{
	if (d3d_core != nullptr)
	{
		d3d_core->Release();
		d3d_core = nullptr;
	}

	if (cpuData != nullptr)
	{
		cpuData->Release();
		cpuData = nullptr;
	}

	if (crender != nullptr)
	{
		crender->Release();
		crender = nullptr;
	}

	if (crenderMT != nullptr)
	{
		crenderMT->Release();
		crenderMT = nullptr;
	}

	if (crit_crender != nullptr)
	{
		crit_crender->Release();
	}

	if (crit_crenderMT != nullptr)
	{
		crit_crenderMT->Release();
	}
}

void ConceptRenderer_D3D9::Release()
{
	delete this;
}

const CRESULT ConceptRenderer_D3D9::GetRenderDevice(GraphicsAdapter *adapter, 
													const Ceng::UINT64 deviceOptions, 
													SwapChainDesc *parameters, 
													SwapChain **swapChainPtr,
													RenderDevice **devicePtr,
													RenderContext **contextPtr)
{
	//Ceng::Log::Print("call : ConceptRenderer_Core::GetRenderDevice\n");

	if (parameters == nullptr)
	{
		//Ceng::Log::Print("\tError : parameters = nullptr\n");
		return CE_ERR_INVALID_PARAM;
	}

	if (parameters->outputWindow == nullptr)
	{
		//Ceng::Log::Print("\tError : parameters->displayWindow = nullptr\n");
		return CE_ERR_INVALID_PARAM;
	}

	if (devicePtr == nullptr)
	{
		//Ceng::Log::Print("\tError : devicePtr = nullptr\n");
		return CE_ERR_NULL_PTR;
	}

	// Convert API parameters to Direct3D 9 format

	D3DFORMAT depthFormat;
	D3DFORMAT outputFormat;

	// Find desktop mode

	HRESULT hr;
	D3DDISPLAYMODE desktopMode;

	DisplayMode crender_desktopMode;

	d3d_core->CurrentDisplayMode(adapter,crender_desktopMode);

	LPDIRECT3D9 corePtr = d3d_core->GetCore();

	hr = corePtr->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&desktopMode);
	if (hr == D3DERR_INVALIDCALL)
	{
		//Ceng::Log::Print("\tError : Failed to get desktop mode\n");
		return CE_ERR_FAIL;
	}

	std::stringstream writer;

	/*
	Ceng::Log::Print("\tDesktop mode:\n");

	writer << "\twidth = " << desktopMode.Width << std::endl;
	writer << "\theight = " << desktopMode.Height << std::endl;
	writer << "\tformat = " << desktopMode.Format << std::endl;
	writer << "\trefresh rate = " << desktopMode.RefreshRate << std::endl;

	Ceng::Log::Print(writer.str().c_str());	
	*/

	D3DPRESENT_PARAMETERS d3d_Params;
	memset(&d3d_Params,0,sizeof(D3DPRESENT_PARAMETERS));

	if (!parameters->windowed)
	{
		switch(parameters->displayMode.format)
		{
		case Ceng::IMAGE_FORMAT::C16:
			outputFormat = D3DFMT_R5G6B5;
			d3d_Params.BackBufferFormat = D3DFMT_R5G6B5;
			break;
		case Ceng::IMAGE_FORMAT::C16_A1:
			outputFormat = D3DFMT_X1R5G5B5;
			d3d_Params.BackBufferFormat = D3DFMT_A1R5G5B5;
			break;
		case Ceng::IMAGE_FORMAT::C32_ARGB:
			outputFormat = D3DFMT_X8R8G8B8;
			d3d_Params.BackBufferFormat = D3DFMT_A8R8G8B8;
			break;
		case Ceng::IMAGE_FORMAT::C32_A2_RGB:
			outputFormat = D3DFMT_A2R10G10B10;
			d3d_Params.BackBufferFormat = D3DFMT_A2R10G10B10;
			break;
		default:
			return CE_ERR_NOT_SUPPORTED;
			break;
		}
	}
	else
	{
		// Windowed mode

		outputFormat = desktopMode.Format;
		d3d_Params.BackBufferFormat = desktopMode.Format;
	}

	// Since we use Direct3D only to display the output of a 
	// software renderer, find any depth format compatible with
	// back buffer format.

	D3DFORMAT depthTest[6] = {D3DFMT_D16, D3DFMT_D24X8, D3DFMT_D24S8,
								D3DFMT_D24X4S4, D3DFMT_D32, D3DFMT_D24FS8 };

	int formatIndex;
	bool foundDepthFormat = false;

	for(formatIndex=0;formatIndex<6;formatIndex++)
	{
		hr = corePtr->CheckDeviceFormat(D3DADAPTER_DEFAULT,
											D3DDEVTYPE_HAL,outputFormat,
											D3DUSAGE_DEPTHSTENCIL,
											D3DRTYPE_SURFACE,
											depthTest[formatIndex]);

		if (hr == D3D_OK)
		{
			depthFormat = depthTest[formatIndex];
			foundDepthFormat = true;
			break;
		}
	}

	if (foundDepthFormat == false)
	{
		//Ceng::Log::Print("\tError : Couldn't find compatible depth format\n");
		return CE_ERR_NOT_SUPPORTED;
	}

	// Set up parameters

	//D3DPRESENT_PARAMETERS d3d_params;

	Ceng::IMAGE_FORMAT::value crender_depthFormat;
	Ceng::BOOL autoDepth = false;
	if (parameters->autoDepthStencil)
	{
		autoDepth = true;

		crender_depthFormat = parameters->autoDepthStencilFormat;

		parameters->autoDepthStencilFormat = Direct3D_Core::ReverseMapFormat(depthFormat);
	}

	Direct3D_Core::TranslateSwapChainDesc(*parameters,d3d_Params);

	d3d_Params.EnableAutoDepthStencil = TRUE;
	d3d_Params.AutoDepthStencilFormat = depthFormat;

	d3d_Params.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3d_Params.MultiSampleQuality = 0;

	if (parameters->windowed)
	{
		// Windowed mode

		//Ceng::Log::Print("\tSetting up windowed mode:\n");

		// Clip back buffer to window client area
		d3d_Params.Flags |= D3DPRESENTFLAG_DEVICECLIP; 

		d3d_Params.BackBufferWidth = desktopMode.Width;
		d3d_Params.BackBufferHeight = desktopMode.Height;

		// Copy specified back buffer area without stretching to
		// target window
		
		d3d_Params.SwapEffect = D3DSWAPEFFECT_COPY;
	

		// Swap effect "copy" required exactly 1 back buffer
		d3d_Params.BackBufferCount = 1;

		// Required when not using swap effect "discard"
		d3d_Params.MultiSampleType = D3DMULTISAMPLE_NONE;
		d3d_Params.MultiSampleQuality = 0;

		// Must be zero for windowed mode
		d3d_Params.FullScreen_RefreshRateInHz = 0; 
	}

	DWORD d3d_DeviceFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;

	if (deviceOptions & Ceng::E_DeviceOption::disable_print_screen)
	{
		d3d_DeviceFlags |= D3DCREATE_DISABLE_PRINTSCREEN;
	}

	if (deviceOptions & Ceng::E_DeviceOption::multi_threaded)
	{
		d3d_DeviceFlags |= D3DCREATE_MULTITHREADED;
	}

	if (deviceOptions & Ceng::E_DeviceOption::no_window_changes)
	{
		d3d_DeviceFlags |= D3DCREATE_NOWINDOWCHANGES;
	}

	if (deviceOptions & Ceng::E_DeviceOption::allow_screen_saver)
	{
		d3d_DeviceFlags |= D3DCREATE_SCREENSAVER;
	}

	if (autoDepth)
	{
		parameters->autoDepthStencilFormat = crender_depthFormat;
	}

	// Create Direct3D 9 device

	IDirect3DDevice9 *d3d_Device;

	hr = corePtr->CreateDevice(D3DADAPTER_DEFAULT,
								D3DDEVTYPE_HAL,d3d_Params.hDeviceWindow,
								d3d_DeviceFlags,&d3d_Params,&d3d_Device);
								
	if (hr != D3D_OK)
	{
		//Ceng::Log::Print("\tError : Failed to create device\n");
		return CE_ERR_FAIL;
	}

	// Create concept engine output interface

	Graphics2D_D3D9 *graphics2D;

	try
	{
		graphics2D = new Graphics2D_D3D9();
	}
	catch(std::bad_alloc &ba)
	{
		//Ceng::Log::Print("\tError : Failed to create output object\n");

		d3d_Device->Release();
		return CE_ERR_OUT_OF_MEMORY;
	}

	CRESULT cresult;

	// NOTE: control of d3d_Device is transfered to outputDevice

	cresult = graphics2D->ConfigureOutput(d3d_Device,&d3d_Params);

	if (cresult != CE_OK)
	{
		//Ceng::Log::Print("\tError : d3d_Output configure failed\n");

		graphics2D->Release();
		return cresult;
	}

	// Load crender library

	
	RenderDeviceCall local_GetRenderDevice;


	if (deviceOptions & E_DeviceOption::multi_threaded)
	{
		crit_crenderMT->Lock();

		if (crenderMT == nullptr)
		{

			crenderMT = DLL_Handle::GetInstance(crender_mt_dll);

			if (crenderMT == nullptr)
			{
				graphics2D->Release();
				return CE_ERR_FAIL;
			}

			call_GetRenderDeviceMT = crenderMT->GetFunction("GetRenderDevice");

			if (call_GetRenderDeviceMT == nullptr)
			{
				graphics2D->Release();
				return CE_ERR_FAIL;
			}
		}

		crit_crenderMT->Unlock();

		local_GetRenderDevice = (RenderDeviceCall)call_GetRenderDeviceMT;
	}
	else
	{
		/*
		crit_crender->Lock();

		if (crender == nullptr)
		{
			crender = DLL_Handle::GetInstance(crender_dll);

			if (crender == nullptr)
			{
				graphics2D->Release();
				return CE_ERR_FAIL;
			}

			call_GetRenderDevice = crender->GetFunction("GetRenderDevice");

			if (call_GetRenderDevice == nullptr)
			{
				graphics2D->Release();
				return CE_ERR_FAIL;
			}
		}

		crit_crender->Unlock();

		local_GetRenderDevice = (RenderDeviceCall)call_GetRenderDevice;
		*/
	}

	// Create renderer object

	RenderDevice *renderer=nullptr;
	RenderContext *context = nullptr;
	SwapChain *swapChain = nullptr;

	int result = local_GetRenderDevice(cpuData,
										&crender_desktopMode,
										graphics2D, parameters, &swapChain,
										&renderer,
										&context);

	if (result != 0)
	{
		graphics2D->Release();
		return CE_ERR_FAIL;
	}
	
	// Write output pointer
	*devicePtr = (RenderDevice*)renderer;
	*contextPtr = context;
	*swapChainPtr = swapChain;

	return CE_OK;
}

const CRESULT ConceptRenderer_D3D9::CreateSwapChain(RenderDevice *device,
	SwapChainDesc &swapChainDesc,
	SwapChain **chainPtr)
{
	return CE_OK;
}

const Ceng::UINT32 ConceptRenderer_D3D9::AdapterCount()
{
	return d3d_core->AdapterCount();
}

const Ceng::CRESULT ConceptRenderer_D3D9::EnumAdapter(const Ceng::UINT32 index, GraphicsAdapter **out_adapter)
{
	return d3d_core->EnumAdapter(index, out_adapter);
}

const Ceng::UINT32 ConceptRenderer_D3D9::CurrentDisplayMode(GraphicsAdapter *adapter, DisplayMode &out_mode)
{
	return d3d_core->CurrentDisplayMode(adapter, out_mode);
}

const Ceng::UINT32 ConceptRenderer_D3D9::DisplayModeCount(GraphicsAdapter *adapter, const Ceng::IMAGE_FORMAT::value format)
{
	return d3d_core->DisplayModeCount(adapter, format);
}

const Ceng::CRESULT ConceptRenderer_D3D9::EnumDisplayMode(GraphicsAdapter *adapter, const Ceng::UINT32 index,
	const Ceng::IMAGE_FORMAT::value format, DisplayMode &out_mode)
{
	return d3d_core->EnumDisplayMode(adapter, index, format, out_mode);
}

const Ceng::CRESULT ConceptRenderer_D3D9::CheckFramebufferSupport(GraphicsAdapter *adapter,
	const Ceng::IMAGE_FORMAT::value displayFormat,
	const Ceng::IMAGE_FORMAT::value backBufferFormat,
	const Ceng::BOOL windowed)
{
	return d3d_core->CheckFramebufferSupport(adapter, displayFormat, backBufferFormat, windowed);
}

const Ceng::CRESULT ConceptRenderer_D3D9::CheckBufferSupport(GraphicsAdapter *out_adapter,
	const Ceng::IMAGE_FORMAT::value displayFormat,
	const Ceng::BufferType::value type,
	const Ceng::IMAGE_FORMAT::value bufferFormat,
	const Ceng::UINT32 binding,
	const Ceng::BufferUsage::value usage,
	const Ceng::UINT32 cpuAccess,
	const Ceng::UINT32 options)
{
	return CE_OK;
}