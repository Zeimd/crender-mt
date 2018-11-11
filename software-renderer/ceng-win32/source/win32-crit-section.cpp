/*****************************************************************************
*
* win32-crit-section.cpp
*
* Created By Jari Korkala 15/9/2014
*
*****************************************************************************/

#include "../include/ceng-win32.h"

#include "../include/win32-crit-section.h"

using namespace Ceng;

extern "C" _declspec(dllexport) const Ceng::CRESULT Ceng_CreateCriticalSection(Ceng::CriticalSection **out_section)
{
	::CRITICAL_SECTION handle;

	::InitializeCriticalSection(&handle);

	WindowsCriticalSection_x86 *temp;

	temp = new WindowsCriticalSection_x86(handle);

	*out_section = temp;

	return CE_OK;
}

WindowsCriticalSection_x86::WindowsCriticalSection_x86(::CRITICAL_SECTION handle)
	: handle(handle)
{
}

WindowsCriticalSection_x86::WindowsCriticalSection_x86()
{

}

WindowsCriticalSection_x86::~WindowsCriticalSection_x86()
{
	::DeleteCriticalSection(&handle);
}

void WindowsCriticalSection_x86::Release()
{
	delete this;
}

const Ceng::CRESULT WindowsCriticalSection_x86::Lock()
{
	::EnterCriticalSection(&handle);
	return CE_OK;
}

const Ceng::CRESULT WindowsCriticalSection_x86::AttemptLock()
{
	::BOOL result = ::TryEnterCriticalSection(&handle);

	if (result == FALSE)
	{
		return CE_ERR_WAIT_TIMEOUT;
	}

	return CE_OK;
}

const Ceng::CRESULT WindowsCriticalSection_x86::Unlock()
{
	::LeaveCriticalSection(&handle);
	return CE_OK;
}