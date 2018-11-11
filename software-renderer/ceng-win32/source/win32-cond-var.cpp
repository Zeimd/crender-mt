/*****************************************************************************
*
* win32-cond-var.cpp
*
* Created By Jari Korkala 16/9/2014
*
*****************************************************************************/

#include "../include/win32-cond-var.h"

#include "../include/win32-crit-section.h"

#include <ceng/datatypes/critical-section.h>

extern "C" _declspec(dllexport) const Ceng::CRESULT Ceng_CreateConditionVar(Ceng::ConditionVariable **out_condVar)
{
	::CONDITION_VARIABLE condVar;

	::InitializeConditionVariable(&condVar);

	Ceng::WindowsConditionVariable_x86 *temp;

	temp = new Ceng::WindowsConditionVariable_x86(condVar);

	*out_condVar = temp;

	return Ceng::CE_OK;
}

using namespace Ceng;

WindowsConditionVariable_x86::WindowsConditionVariable_x86()
{
}


WindowsConditionVariable_x86::WindowsConditionVariable_x86(const ::CONDITION_VARIABLE condVar)
	: condVar(condVar)
{
}


WindowsConditionVariable_x86::~WindowsConditionVariable_x86()
{
}

void WindowsConditionVariable_x86::Release()
{
	delete this;
}

const Ceng::CRESULT WindowsConditionVariable_x86::Wait(CriticalSection *critSection)
{
	WindowsCriticalSection_x86 *crit = (WindowsCriticalSection_x86*)critSection;

	::BOOL result  = ::SleepConditionVariableCS(&condVar,&crit->handle,INFINITE);

	if (result == FALSE)
	{
		::DWORD error = ::GetLastError();
		return CE_ERR_FAIL;
	}

	return CE_OK;
}

const Ceng::CRESULT WindowsConditionVariable_x86::WaitFor(CriticalSection *critSection,
														  const Ceng::UINT32 maxWaitTimeMs)
{
	WindowsCriticalSection_x86 *crit = (WindowsCriticalSection_x86*)critSection;

	::BOOL result  = ::SleepConditionVariableCS(&condVar,&crit->handle,maxWaitTimeMs);

	if (result == FALSE)
	{
		::DWORD error = ::GetLastError();
		return CE_ERR_FAIL;
	}

	return CE_OK;
}

const Ceng::CRESULT WindowsConditionVariable_x86::WakeOne()
{
	::WakeConditionVariable(&condVar);

	return CE_OK;
}

const Ceng::CRESULT WindowsConditionVariable_x86::WakeAll()
{
	::WakeAllConditionVariable(&condVar);

	return CE_OK;
}
