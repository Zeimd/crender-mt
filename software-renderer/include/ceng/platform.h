/*****************************************************************************
*
* ceng-platform.h
*
* Created By Jari Korkala 3/9/2014
*
*****************************************************************************/

#ifndef CENG_PLATFORM_H
#define CENG_PLATFORM_H

#include "datatypes/cpu-info.h"

#include "datatypes/mutex.h"
#include "datatypes/critical-section.h"
#include "datatypes/condition-variable.h"

#include "datatypes/thread.h"
#include "interfaces/thread-task.h"

#include "interfaces/dynamic-library.h"

#include "datatypes/window.h"
#include "datatypes/keyboard.h"
#include "interfaces/graphics-system.h"

#ifdef _MSC_VER
#if (_WIN32 || _WIN64)

#ifdef CENG_WIN32_DLL
	#define DLL_FUNCTION _declspec(dllexport)
#else
	#define DLL_FUNCTION _declspec(dllimport)
#endif

extern "C"
{
	DLL_FUNCTION
		const Ceng::CRESULT Ceng_LoadLibrary(const Ceng::String &name,Ceng::DynamicLibrary **out_library);

	DLL_FUNCTION
		const Ceng::CRESULT Ceng_CreateWindow(Ceng::String &windowTitle,
									const Ceng::UINT32 locX,const Ceng::UINT32 locY,
									const Ceng::UINT32 width,const Ceng::UINT32 height,
									const Ceng::UINT32 options,
									Ceng::PlatformWindow *parent,
									Ceng::PlatformWindow **out_window);
	
	DLL_FUNCTION
		const Ceng::CRESULT Ceng_MessageWindow(Ceng::PlatformWindow *parent,
												const Ceng::String &windowTitle,
												const Ceng::String &message);

	DLL_FUNCTION
		const Ceng::CRESULT Ceng_GetKeyboard(Ceng::PlatformWindow *focusWindow,
												Ceng::KeyboardDevice **out_device);

	DLL_FUNCTION
		const Ceng::CRESULT Ceng_CreateGraphics3D(Ceng::RENDERER_TYPE::value type,
													Ceng::GraphicsSystem **out_renderer);
		
	DLL_FUNCTION const Ceng::CRESULT Ceng_GetCpuInfo(Ceng::CPU_Info **out_info);		
	
	DLL_FUNCTION const Ceng::CRESULT Ceng_CreateMutex(Ceng::Mutex **out_mutex);

	DLL_FUNCTION const Ceng::CRESULT Ceng_CreateThread(Ceng::ThreadTask *task,const Ceng::BOOL paused,
														Ceng::Thread **out_thread);

	DLL_FUNCTION const Ceng::CRESULT Ceng_CreateCriticalSection(Ceng::CriticalSection **out_section);

	DLL_FUNCTION const Ceng::CRESULT Ceng_CreateConditionVar(Ceng::ConditionVariable **out_condVar);

	DLL_FUNCTION const Ceng::CRESULT Ceng_WaitForAll(Ceng::Thread **threads,const Ceng::UINT32 count,
												const Ceng::UINT32 msMaxWaitTime);

	DLL_FUNCTION const Ceng::CRESULT Ceng_WaitForAny(Ceng::Thread **threads,
										const Ceng::UINT32 count,
										const Ceng::UINT32 msMaxWaitTime,
										Ceng::UINT32 *out_index);
	
}

#undef DLL_FUNCTION

#endif
#endif

#endif