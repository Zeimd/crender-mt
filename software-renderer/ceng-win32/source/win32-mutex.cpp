/*****************************************************************************
*
* win32-mutex.cpp
*
* Created By Jari Korkala 4/9/2014
*
*****************************************************************************/

#include "../include/ceng-win32.h"

#include "../include/win32-mutex.h"

using namespace Ceng;

extern "C" _declspec(dllexport) const Ceng::CRESULT Ceng_CreateMutex(Ceng::Mutex **out_mutex)
{
	HANDLE handle;

	handle = ::CreateMutex(NULL,FALSE,NULL);

	if (handle == NULL) return CE_ERR_FAIL;

	*out_mutex = new WindowsMutex_x86(handle);

	return CE_OK;
}

WindowsMutex_x86::WindowsMutex_x86()
{
}

WindowsMutex_x86::WindowsMutex_x86(const HANDLE handle) : mutexHandle(handle)
{

}

WindowsMutex_x86::~WindowsMutex_x86()
{
	DWORD result = ::WaitForSingleObject(mutexHandle,INFINITE);

	::CloseHandle(mutexHandle);
}

void WindowsMutex_x86::Release()
{
	delete this;
}

const CRESULT WindowsMutex_x86::Lock(const Ceng::UINT32 msMaxWaitTime)
{
	DWORD time = msMaxWaitTime;

	if (time == -1) time = INFINITE;

	DWORD result = ::WaitForSingleObject(mutexHandle,time);

	if (result != WAIT_OBJECT_0)
	{
		DWORD error = GetLastError();

		return CE_ERR_FAIL;
	}

	return CE_OK;
}

const CRESULT WindowsMutex_x86::Unlock()
{
	BOOL result = ::ReleaseMutex(mutexHandle);

	if (result == 0)
	{
		DWORD error = GetLastError();

		return CE_ERR_FAIL;
	}

	return CE_OK;
}