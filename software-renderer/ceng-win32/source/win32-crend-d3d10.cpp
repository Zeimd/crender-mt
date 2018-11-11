/*****************************************************************************
*
* win32-crend-d3d10.cpp
*
* By Jari Korkala 5/2013
*
*****************************************************************************/

#include "../include/win32-crend-d3d10.h"

#include "../include/graphics-2d-d3d10.h"

#include "../include/win32-window.h"

#include <ceng/datatypes/cpu-info.h>

#ifdef _MSC_VER
	#ifdef _DEBUG
		#ifdef UNICODE
			#define CRENDER_DLL_FILE L"crender-debug.dll"
		#else
			#define CRENDER_DLL_FILE "crender-debug.dll"
		#endif
	#else
		#ifdef UNICODE
			#define CRENDER_DLL_FILE L"crender.dll"
		#else
			#define CRENDER_DLL_FILE "crender.dll"
		#endif
	#endif
#endif

using namespace Ceng;


	

ConceptRenderer_D3D10::ConceptRenderer_D3D10()
{
	dxgiFactory = nullptr;

	displayAdapter = nullptr;
	adapterOutput = nullptr;

	device = nullptr;
	swapChain = nullptr;

	cpuData = nullptr;

	crenderHandle = nullptr;
}

ConceptRenderer_D3D10::ConceptRenderer_D3D10(CPU_Info *sourceCPUdata)
{
	dxgiFactory = nullptr;

	displayAdapter = nullptr;
	adapterOutput = nullptr;

	device = nullptr;
	swapChain = nullptr;

	cpuData = nullptr;

	crenderHandle = nullptr;

	cpuData = sourceCPUdata;
}

ConceptRenderer_D3D10::~ConceptRenderer_D3D10()
{
	Clean();
}

void ConceptRenderer_D3D10::Clean()
{
	if (crenderHandle != nullptr)
	{
		FreeLibrary(crenderHandle);
		crenderHandle = nullptr;
	}

	if (dxgiFactory != nullptr)
	{
		dxgiFactory->Release();
		dxgiFactory = nullptr;
	}

	if (displayAdapter != nullptr)
	{
		displayAdapter->Release();
		displayAdapter = nullptr;
	}

	/*
	if (device != nullptr)
	{
		device->Release();
		device = nullptr;
	}

	if (swapChain != nullptr)
	{
		swapChain->Release();
		swapChain = nullptr;
	}
	*/
	
	if (cpuData != nullptr)
	{
		cpuData->Release();
		cpuData = nullptr;
	}
}

void ConceptRenderer_D3D10::Release()
{
	delete this;
}

const CRESULT ConceptRenderer_D3D10::Initialize()
{
	HRESULT hr;

	/*
	hr = CreateDXGIFactory(__uuidof(IDXGIFactory),(void**)&dxgiFactory);

	if (hr != S_OK)
	{
		Ceng::Log::Print("CreateDXGIFactory : Failed");
		return CE_ERR_FAIL;
	}

	// Get default display adapter

	hr = dxgiFactory->EnumAdapters(0,&displayAdapter);

	if (hr != S_OK)
	{
		Ceng::Log::Print("EnumAdapters : Failed");
		dxgiFactory->Release();
		return CE_ERR_FAIL;
	}

	// Get default adapter output

	hr = displayAdapter->EnumOutputs(0,&adapterOutput);

	if (hr != S_OK)
	{
		Ceng::Log::Print("EnumOutputs : Failed");
		displayAdapter->Release();
		dxgiFactory->Release();
		return CE_ERR_FAIL;
	}
	*/

	return CE_OK;
}

const CRESULT ConceptRenderer_D3D10::GetRenderDevice(GraphicsAdapter *adapter, 
											  const Ceng::UINT64 deviceOptions, 
											  SwapChainDesc *parameters, 
											  SwapChain **swapChainPtr,
											  RenderDevice **devicePtr,
											  RenderContext **contextPtr)
{

	HRESULT hr;

	// Set main window

	*devicePtr = nullptr;

	WinAPI_Window *window = (WinAPI_Window*)parameters->outputWindow;

	DXGI_SWAP_CHAIN_DESC swapDesc;

	memset(&swapDesc,0,sizeof(DXGI_SWAP_CHAIN_DESC));

	swapDesc.BufferDesc.Width = parameters->displayMode.width;
	swapDesc.BufferDesc.Height = parameters->displayMode.height;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	//swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;

	swapDesc.OutputWindow = window->GetWindowHandle();
	swapDesc.Windowed = TRUE;

	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	//swapDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

	swapDesc.BufferCount = 1;
	swapDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;

	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	hr = D3D10CreateDeviceAndSwapChain(NULL,
										D3D10_DRIVER_TYPE_HARDWARE,
										NULL,
										0,
										D3D10_SDK_VERSION,
										&swapDesc,
										&swapChain,
										&device);

	if (hr != S_OK)
	{
		if (hr == E_FAIL)
		{
			//Ceng::Log::Print("CreateDeviceAndSwapChain : Invalid call");
		}
		else if (hr == E_INVALIDARG)
		{
			//Ceng::Log::Print("CreateDeviceAndSwapChain : Invalid parameter");
		}
		else
		{
			//Ceng::Log::Print("CreateDeviceAndSwapChain : Failed");
		}

		Clean();
		return CE_ERR_FAIL;
	}

	Graphics2D_D3D10 *graphics2D;

	try
	{
		graphics2D = new Graphics2D_D3D10();
	}
	catch(std::bad_alloc &ba)
	{
		Clean();
		return CE_ERR_OUT_OF_MEMORY;
	}

	CRESULT cresult;

	cresult = graphics2D->ConfigureOutput(device,swapChain,&swapDesc.BufferDesc);

	if (cresult != CE_OK)
	{
		// Must be deleted separately because ownership will pass to RenderDevice.
		delete graphics2D;

		Clean();
		return CE_ERR_FAIL;
	}

	//*****************************************************'
	// Create concept renderer object

	crenderHandle = LoadLibrary(CRENDER_DLL_FILE);

	if (crenderHandle == nullptr)
	{
		delete graphics2D;
		Clean();		
		return CE_ERR_FAIL;
	}

	FARPROC callBackAddress = GetProcAddress(crenderHandle,"GetRenderDevice");

	if (callBackAddress == nullptr)
	{
		delete graphics2D;
		Clean();
		return CE_ERR_FAIL;
	}

	if (!parameters->windowed)
	{
		swapChain->SetFullscreenState(TRUE,NULL);
	}

	call_GetRenderDevice = (int (*)(UINT32,UINT32,Graphics2D*,SwapChainDesc*,RenderDevice**))callBackAddress;

	RenderDevice *renderer=nullptr;

	int result = call_GetRenderDevice(cpuData->CacheLine(),1,graphics2D,parameters,(RenderDevice**)&renderer);

	if (result != 0)
	{
		delete graphics2D;
		Clean();
		return CE_ERR_FAIL;
	}
	
	// Write output pointer
	*devicePtr = (RenderDevice*)renderer;

	return CE_OK;
}