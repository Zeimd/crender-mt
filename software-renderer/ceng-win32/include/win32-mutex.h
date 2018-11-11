/*****************************************************************************
*
* win32-mutex.h
*
* Created By Jari Korkala 4/9/2014
*
*****************************************************************************/

#ifndef WIN32_MUTEX_H
#define WIN32_MUTEX_H

#include <Windows.h>

#include <ceng/datatypes/mutex.h>

namespace Ceng
{
	class WindowsMutex_x86: public Mutex
	{
	protected:

		HANDLE mutexHandle;

	protected:

		WindowsMutex_x86();

		virtual ~WindowsMutex_x86();
		
	public:

		WindowsMutex_x86(const HANDLE handle);

		virtual void Release() override;

		virtual const CRESULT Lock(const Ceng::UINT32 msMaxWaitTime) override;

		virtual const CRESULT Unlock() override;
	};
}

#endif