/*****************************************************************************
*
* win32-crit-section.h
*
* Created By Jari Korkala 15/9/2014
*
*****************************************************************************/

#ifndef CENG_WIN32_CRIT_SECTION_H
#define CENG_WIN32_CRIT_SECTION_H

#include <Windows.h>

#include <ceng/datatypes/critical-section.h>

namespace Ceng
{
	class WindowsCriticalSection_x86 : public CriticalSection
	{
	public:

		::CRITICAL_SECTION handle;

		WindowsCriticalSection_x86();

	public:

		WindowsCriticalSection_x86(::CRITICAL_SECTION handle);		

		virtual ~WindowsCriticalSection_x86();

		virtual void Release() override;

		virtual const CRESULT Lock() override;

		virtual const CRESULT AttemptLock() override;

		virtual const CRESULT Unlock() override;

	};
}

#endif
