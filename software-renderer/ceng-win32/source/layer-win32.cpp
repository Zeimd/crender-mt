/*****************************************************************************
*
* layer-win32.cpp
*
* By Jari Korkala 1/6/2011
*
* Platform abstraction for 32-bit Windows on x86.
*
*****************************************************************************/

/*
#include "../include/layer-win32.h"
#include "../include/win32-dll.h"

#ifdef _CE_PLATFORM_WIN32

#define _CE_PLATFORM_DIRECTX
#define _CE_PLATFORM_OPENGL

using namespace Ceng;

WindowsLayer_x86::WindowsLayer_x86()
{
	processHandle = GetCurrentProcess();
}

WindowsLayer_x86::~WindowsLayer_x86()
{
	
}

#ifdef _MSC_VER

#include <intrin.h>
#include <immintrin.h>

// GatherData() determines CPU capabilities such as MMX,SSE support and
// cache line length. Must be compiler specific 

const CRESULT WindowsLayer_x86::Configure()
{
	processHandle = GetCurrentProcess();

	DLL_Handle *kernel32 = DLL_Handle::GetInstance("kernel32.dll");

	if (kernel32 == nullptr)
	{
		// Serious problems
		return CE_ERR_FAIL;
	}

	// Check if we are running under 64-bit version of Windows

	isWOW64 = FALSE;

	DynamicCallback dynamicPtr = kernel32->GetFunction("IsWow64Process");

	if (dynamicPtr != NULL)
	{
		typedef ::BOOL (WINAPI *Call_IsWow64Process)(HANDLE,PBOOL);

		Call_IsWow64Process call_IsWow64Process = (Call_IsWow64Process)dynamicPtr;

		::BOOL result = (*call_IsWow64Process)(processHandle,&isWOW64);

		kernel32->Release();

		if (result == 0)
		{
			::DWORD error = GetLastError();
			return CE_ERR_FAIL;
		}
	}

	if (isWOW64 == TRUE)
	{
		GetNativeSystemInfo(&systemInfo);
	}
	else
	{
		GetSystemInfo(&systemInfo);
	}

	return CE_OK;
}

#endif


#endif // _CE_PLATFORM_WIN32
*/