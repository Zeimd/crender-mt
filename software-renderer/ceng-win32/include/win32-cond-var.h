/*****************************************************************************
*
* win32-cond-var.h
*
* Created By Jari Korkala 16/9/2014
*
*****************************************************************************/

#ifndef CENG_WIN32_COND_VAR_H
#define CENG_WIN32_COND_VAR_H

#include <Windows.h>

#include <ceng/datatypes/condition-variable.h>

namespace Ceng
{
	class CriticalSection;

	class WindowsConditionVariable_x86 : public ConditionVariable
	{
	protected:

		::CONDITION_VARIABLE condVar;

		WindowsConditionVariable_x86();

	public:

		WindowsConditionVariable_x86(const ::CONDITION_VARIABLE condVar);

		virtual ~WindowsConditionVariable_x86();

		virtual void Release() override;

		virtual const CRESULT Wait(CriticalSection *critSection) override;

		virtual const Ceng::CRESULT WaitFor(CriticalSection *critSection,
												const Ceng::UINT32 maxWaitTimeMs) override;

		virtual const Ceng::CRESULT WakeOne() override;
		virtual const Ceng::CRESULT WakeAll() override;
	};
}

#endif