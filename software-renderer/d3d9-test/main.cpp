/*****************************************************************************
*
* main.cpp
*
* Created By Jari Korkala 3/3/2015
*
* - For purposes of testing Direct3D 9 component.
*
*****************************************************************************/

#include <ceng.h>

#include <ceng/lib/liblog.h>

#include <timerlib.h>

#include <Windows.h>

const Ceng::String ImageFormatToString(const Ceng::IMAGE_FORMAT::value format);


// Debug information
#ifdef _MSC_VER

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <crtdbg.h>

#endif

#ifdef _WIN32

#include <windows.h>
#include <sstream>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{

#ifdef _DEBUG
	// Detect memory leaks after main-function ends
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	int k;

	Ceng::CRESULT cresult;

	Ceng::FileLog engineLog;
	Ceng::Log::SetLog(&engineLog);

	std::stringstream writer;

	cresult = engineLog.OpenFile("d3d9-test-log.txt");
	if (cresult != Ceng::CE_OK)
	{
		Ceng::Log::SetLog(NULL);
		return 0;
	}

	Ceng::Log::Print("Program started");

	Ceng::UINT32 resX = 640;
	Ceng::UINT32 resY = 480;

	Ceng::BOOL fullScreenMode = false;

	Ceng::String programName;
	Ceng::String captionInfo;

	programName = "Ceng-d3d9 v ";
	programName += CONCEPT_VERSION;
	programName += ".";
	programName += CONCEPT_MINOR;
	programName += " Build ";
	programName += CONCEPT_BUILD;

	Ceng::PlatformWindow *window;

	Ceng::UINT32 windowOptions = 0;

	if (fullScreenMode)
	{
		windowOptions = Ceng::E_WindowOptions::popup | Ceng::E_WindowOptions::topmost;
	}
	else
	{
		windowOptions = Ceng::E_WindowOptions::resizable
			| Ceng::E_WindowOptions::center_x
			| Ceng::E_WindowOptions::center_y;
	}

	Ceng_CreateWindow(programName, 0, 0, resX, resY,
		windowOptions, nullptr,
		&window);

	if (window == nullptr)
	{
		return 0;
	}

	Ceng::CPU_Info *cpuData;

	Ceng_GetCpuInfo(&cpuData);

	Ceng::E_CpuFeatureInfo::value testResult;

	cpuData->CheckFeature("sse_2", &testResult);

	if (testResult != Ceng::E_CpuFeatureInfo::ok)
	{
		window->Show();
		Ceng::String title = "Error";
		Ceng::String message = "CPU doesn't support SSE2";

		cresult = Ceng_MessageWindow(window, title, message);

		cpuData->Release();
		return 0;
	}

	cpuData->Release();

	Ceng::GraphicsSystem *renderCore = nullptr;

	//Ceng_CreateGraphics3D(Ceng::RENDERER_TYPE::SOFTWARE, &renderCore);
	Ceng_CreateGraphics3D(Ceng::RENDERER_TYPE::D3D_9,&renderCore);

	if (renderCore == nullptr)
	{
		Ceng::Log::Print("Error:  GetRenderer() failed");

		window->Release();
		return 0;
	}

	//**************************************************************************

	// Enumerate display adapters

	Ceng::GraphicsAdapter *adapter;
	Ceng::GraphicsAdapterDesc adapterDesc;

	Ceng::UINT32 adapterCount = renderCore->AdapterCount();

	Ceng::String out = "display adapters = ";
	out += adapterCount;

	Ceng::Log::Print(out);

	renderCore->EnumAdapter(0, &adapter);

	out = "Display adapter:";

	adapter->GetDesc(adapterDesc);

	Ceng::Log::Print(out);
	Ceng::Log::Print(adapterDesc.description);
	Ceng::Log::Print("\n");

	//**************************************************************************
	// Enumerate display modes for C32_ARGB

	Ceng::IMAGE_FORMAT::value format = Ceng::IMAGE_FORMAT::C32_XRGB;

	out = "Checking back buffer format : ";
	out += ImageFormatToString(format);
	out += "\n";

	Ceng::UINT32 modeCount = renderCore->DisplayModeCount(adapter, format);

	out += "modeCount = ";
	out += modeCount;
	out += "\n";
	Ceng::Log::Print(out);

	std::vector<Ceng::DisplayMode> modeList;

	for (Ceng::UINT32 k = 0; k < modeCount; k++)
	{
		Ceng::DisplayMode displayMode;

		renderCore->EnumDisplayMode(adapter, k, format, displayMode);

		if (displayMode.refreshHz < 59)
		{
			continue;
		}

		modeList.push_back(displayMode);

		out = "display mode ";
		out += modeList.size() - 1;
		out += " :\n";
		out += "width = ";
		out += displayMode.width;
		out += "\n";
		out += "height = ";
		out += displayMode.height;
		out += "\n";
		out += "hz = ";
		out += displayMode.refreshHz;
		Ceng::Log::Print(out);
		Ceng::Log::Print("\n");
	}

	//**************************************************************************
	// Test framebuffer format support

	Ceng::IMAGE_FORMAT::value displayFormat = Ceng::IMAGE_FORMAT::C32_XRGB;
	Ceng::IMAGE_FORMAT::value backBufferFormat = Ceng::IMAGE_FORMAT::C32_ARGB;

	cresult = renderCore->CheckFramebufferSupport(adapter, displayFormat, backBufferFormat, true);

	out = "Testing frame buffer format with:\n";
	out += "display format = ";
	out += ImageFormatToString(displayFormat);
	out += "\nback buffer format = ";
	out += ImageFormatToString(backBufferFormat);
	out += "\nresult = ";

	if (cresult == Ceng::CE_OK)
	{
		out += "OK";
	}
	else
	{
		out += "FAIL";
	}

	out += "\n";

	Ceng::Log::Print(out);

	//**************************************************************************
	// Test dxt5 support

	cresult = renderCore->CheckBufferSupport(adapter, displayFormat,
		Ceng::BufferType::texture_2d,
		Ceng::IMAGE_FORMAT::dxt5,
		Ceng::BufferBinding::shader_resource,
		Ceng::BufferUsage::gpu_read_only,
		0,
		0);

	Ceng::Log::Print("Testing dxt5 support with C32_ARGB display format");

	out = "result = ";

	if (cresult == Ceng::CE_OK)
	{
		out += "OK";
	}
	else
	{
		out += "FAIL";
	}

	out += "\n";

	Ceng::Log::Print(out);

	//**************************************************************************
	// Test depth buffer support

	cresult = renderCore->CheckBufferSupport(adapter, displayFormat,
		Ceng::BufferType::back_buffer,
		Ceng::IMAGE_FORMAT::D32F_LOCKABLE,
		Ceng::BufferBinding::shader_resource,
		Ceng::BufferUsage::gpu_read_only,
		0,
		0);

	Ceng::Log::Print("Testing D24_S8 support with C32_ARGB display format");

	out = "result = ";

	if (cresult == Ceng::CE_OK)
	{
		out += "OK";
	}
	else
	{
		out += "FAIL";
	}

	out += "\n";

	Ceng::Log::Print(out);
	adapter->Release();
	
	

	renderCore->Release();

	window->Release();

	Ceng::Log::Print("Program finished");

	return 0;
}

#endif

#endif