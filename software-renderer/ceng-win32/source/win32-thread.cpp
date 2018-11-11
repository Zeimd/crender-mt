/*****************************************************************************
*
* win32-thread.cpp
*
* Created By Jari Korkala 3/9/2014
*
*****************************************************************************/

#include <vector>

#include <ceng/interfaces/thread-task.h>

#include "../include/ceng-win32.h"

#include "../include/win32-thread.h"

using namespace Ceng;

extern "C" const Ceng::CRESULT Ceng_CreateThread(Ceng::ThreadTask *task,const Ceng::BOOL paused,
												 Ceng::Thread **out_thread)
{
	::HANDLE processHandle = ::GetCurrentProcess();

	if (processHandle == NULL)
	{
		return CE_ERR_FAIL;
	}

	::HANDLE handle = NULL;

	::DWORD threadID;

	::DWORD flags=0;

	::BOOL suspended = FALSE;

	if (paused) 
	{
		flags |= CREATE_SUSPENDED;
		suspended = TRUE;
	}
	
	handle = ::CreateThread(NULL,0,&ThreadProc,(LPVOID)task,flags,&threadID);

	if (handle == NULL) return CE_ERR_FAIL;
	
	WindowsThread_x86 *temp;
	
	temp = new WindowsThread_x86(handle,threadID,suspended,task,processHandle);

	*out_thread = (Ceng::Thread*)temp;

	return CE_OK;
}

extern "C" const CRESULT Ceng_WaitForAll(Ceng::Thread **threads,const Ceng::UINT32 count,const Ceng::UINT32 msMaxWaitTime)
{
	if (count > MAXIMUM_WAIT_OBJECTS)
	{
		return CE_ERR_INVALID_PARAM;
	}

	std::vector<HANDLE> handles;

	Ceng::UINT32 k;

	for(k=0;k<count;k++)
	{
		WindowsThread_x86 *thread = static_cast<WindowsThread_x86*>(threads[k]);

		handles.push_back(thread->GetHandle());
	}

	DWORD result = ::WaitForMultipleObjects(count,&handles[0],TRUE,msMaxWaitTime);

	if (result == WAIT_OBJECT_0)
	{
		return CE_OK;
	}
	else if (result == WAIT_ABANDONED_0)
	{
		return CE_ERR_ABANDONED_OBJECT;
	}
	else if (result == WAIT_TIMEOUT)
	{
		return CE_ERR_WAIT_TIMEOUT;
	}
	else if (result == WAIT_FAILED)
	{
		DWORD error = GetLastError();

		return CE_ERR_FAIL;
	}

	return CE_OK;
}

extern "C" const CRESULT Ceng_WaitForAny(Ceng::Thread **threads,const Ceng::UINT32 count,
										 const Ceng::UINT32 msMaxWaitTime,Ceng::UINT32 *out_index)
{
	if (count > MAXIMUM_WAIT_OBJECTS)
	{
		return CE_ERR_INVALID_PARAM;
	}

	std::vector<HANDLE> handles;

	Ceng::UINT32 k;

	for(k=0;k<count;k++)
	{
		WindowsThread_x86 *thread = static_cast<WindowsThread_x86*>(threads[k]);

		handles.push_back(thread->GetHandle());
	}

	DWORD result = ::WaitForMultipleObjects(count,&handles[0],FALSE,msMaxWaitTime);

	if (result < WAIT_ABANDONED_0)
	{
		*out_index = result - WAIT_OBJECT_0;
		return CE_OK;
	}
	else if (result < WAIT_TIMEOUT)
	{
		*out_index = result - WAIT_ABANDONED_0;
		return CE_ERR_ABANDONED_OBJECT;
	}
	else if (result == WAIT_TIMEOUT)
	{
		*out_index = -1;
		return CE_ERR_WAIT_TIMEOUT;
	}
	else if (result == WAIT_FAILED)
	{
		*out_index = -1;
		DWORD error = GetLastError();

		return CE_ERR_FAIL;
	}

	return CE_OK;
}


WindowsThread_x86::WindowsThread_x86(const HANDLE threadHandle,const DWORD threadID,
									 const ::BOOL paused,ThreadTask *task,
									 const ::HANDLE processHandle)
	: threadHandle(threadHandle),threadID(threadID),paused(paused),task(task),
	priorityLevel(E_ThreadPriority::normal),processHandle(processHandle)
{

}

WindowsThread_x86::WindowsThread_x86()
{
}

WindowsThread_x86::~WindowsThread_x86()
{
	Exit();

	// wait for thread to finish

	::DWORD result = ::WaitForSingleObject(threadHandle,10000);

	if (result == WAIT_TIMEOUT)
	{
		Terminate();
	}

	::CloseHandle(threadHandle);
}

void WindowsThread_x86::Release()
{
	delete this;
}

const ::HANDLE WindowsThread_x86::GetHandle()
{
	return threadHandle;
}

const CRESULT WindowsThread_x86::Resume()
{
	if (paused == TRUE)
	{
		DWORD result = ResumeThread(threadHandle);

		if (result == -1)
		{
			result = GetLastError();

			return CE_ERR_FAIL;
		}

		paused = FALSE;
	}

	return CE_OK;
}

const CRESULT WindowsThread_x86::Pause()
{
	if (paused == FALSE)
	{
		DWORD result = SuspendThread(threadHandle);

		if (result == -1)
		{
			result = GetLastError();

			return CE_ERR_FAIL;
		}

		paused = TRUE;
	}

	return CE_OK;
}

const CRESULT WindowsThread_x86::Exit()
{
	return task->Exit();
}

const CRESULT WindowsThread_x86::Terminate()
{
	::BOOL result = ::TerminateThread(threadHandle,0);

	if (result == FALSE)
	{
		DWORD error = GetLastError();

		return CE_ERR_FAIL;
	}

	return CE_OK;
}

const CRESULT WindowsThread_x86::Wait(const Ceng::UINT32 msMaxWaitTime)
{
	::DWORD result = ::WaitForSingleObject(threadHandle,msMaxWaitTime);

	if (result != WAIT_OBJECT_0)
	{
		return CE_ERR_FAIL;
	}

	return CE_OK;
}

const CRESULT WindowsThread_x86::SetPriority(const E_ThreadPriority::value level)
{
	int priority = 0;

	switch(level)
	{
	case E_ThreadPriority::idle:
		priority = THREAD_PRIORITY_IDLE;
		break;
	case E_ThreadPriority::lowest:
		priority = THREAD_PRIORITY_LOWEST;
		break;
	case E_ThreadPriority::below_normal:
		priority = THREAD_PRIORITY_BELOW_NORMAL;
		break;
	case E_ThreadPriority::normal:
		priority = THREAD_PRIORITY_NORMAL;
		break;
	case E_ThreadPriority::above_normal:
		priority = THREAD_PRIORITY_ABOVE_NORMAL;
		break;
	case E_ThreadPriority::highest:
		priority = THREAD_PRIORITY_HIGHEST;
		break;
	case E_ThreadPriority::real_time:
		priority = THREAD_PRIORITY_TIME_CRITICAL;
		break;
	case E_ThreadPriority::background_start:
		priority = THREAD_MODE_BACKGROUND_BEGIN;
		break;
	case E_ThreadPriority::background_end:
		priority = THREAD_MODE_BACKGROUND_END;
		break;
	default:
		return CE_ERR_INVALID_PARAM;
	}

	::BOOL result = ::SetThreadPriority(threadHandle,priority);

	if (result == 0)
	{
		DWORD error = GetLastError();

		return CE_ERR_FAIL;
	}

	priorityLevel = level;

	return CE_OK;
}

const E_ThreadPriority::value WindowsThread_x86::GetPriority()
{
	return priorityLevel;
}

const Ceng::UINT32 WindowsThread_x86::GetProcessorAffinity() const
{
	return affinityMask;
}

const CRESULT WindowsThread_x86::SetProcessorAffinity(const Ceng::UINT32 mask)
{
	::DWORD processMask,systemMask;

	::BOOL result = GetProcessAffinityMask(processHandle,&processMask,&systemMask);

	if (result == FALSE)
	{
		::DWORD error = GetLastError();
		return CE_ERR_FAIL;
	}

	// TODO: Return with error if *mask* has bits set where processMask doesn't

	::DWORD affinityMask = mask & processMask;

	::DWORD out = SetThreadAffinityMask(threadHandle,affinityMask);

	if (out == 0)
	{
		::DWORD error = GetLastError();
		return CE_ERR_FAIL;
	}

	return CE_OK;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	ThreadTask *ptr = (ThreadTask*)lpParameter;

	CRESULT cresult = ptr->Execute();

	return 0;
}