/*****************************************************************************
*
* win32-thread.h
*
* Created By Jari Korkala 3/9/2014
*
*****************************************************************************/

#ifndef WIN32_THREAD_H
#define WIN32_THREAD_H

#include <Windows.h>

#include <ceng/datatypes/thread.h>
#include <ceng/datatypes/boolean.h>

namespace Ceng
{
	class ThreadTask;

	class WindowsThread_x86 : public Ceng::Thread
	{
	protected:

		::HANDLE processHandle;

		::HANDLE threadHandle;

		::DWORD threadID;

		::BOOL paused;

		ThreadTask *task;
		
		E_ThreadPriority::value priorityLevel;

		Ceng::UINT32 affinityMask;

	protected:

		WindowsThread_x86();

	public:		

		static WindowsThread_x86* GetInstance(ThreadTask *task,const Ceng::BOOL paused);

		WindowsThread_x86(const HANDLE threadHandle,const DWORD threadID,
							const ::BOOL paused,ThreadTask *task,
							const ::HANDLE processHandle);

		virtual ~WindowsThread_x86();

		virtual void Release() override;

		virtual const CRESULT Resume() override;

		virtual const CRESULT Pause() override;

		virtual const CRESULT Exit() override;

		virtual const CRESULT Wait(const Ceng::UINT32 msMaxWaitTime) override;

		virtual const CRESULT Terminate() override;

		virtual const CRESULT SetPriority(const E_ThreadPriority::value level) override;

		virtual const E_ThreadPriority::value GetPriority() override;

		virtual const CRESULT SetProcessorAffinity(const Ceng::UINT32 mask) override;

		virtual const Ceng::UINT32 GetProcessorAffinity() const override;

	public:

		const ::HANDLE GetHandle();
	};
}

DWORD WINAPI ThreadProc(LPVOID lpParameter);

#endif

