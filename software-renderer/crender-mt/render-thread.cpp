/*****************************************************************************
*
* render-thread.cpp
*
* Created By Jari Korkala 10/10/2014
*
*****************************************************************************/

#include "render-thread.h"
#include "pipeline.h"

#include "render-task.h"

#include <ceng/platform.h>
#include <ceng/datatypes/release-deleter.h>

using namespace Ceng;

RenderThread::RenderThread() 
	: exitLoop(0),wakeCrit(nullptr),wakeCondition(nullptr),
	cmdWake(nullptr,ReleaseDeleter<ConditionVariable>())
{
}

RenderThread::RenderThread(const Ceng::UINT32 threadId,ConditionVariable *wakeCondition,
						   std::shared_ptr<ConditionVariable> &cmdWake,
						   std::atomic<Ceng::UINT32> *runningThreadCount,
						   std::atomic<Ceng::UINT32> *minThreads,
						   std::atomic<Ceng::UINT32> *maxThreads,Pipeline *pipeline)
	: threadId(threadId),wakeCondition(wakeCondition),cmdWake(cmdWake),
	runningThreadCount(runningThreadCount),minThreads(minThreads),maxThreads(maxThreads),
	pipeline(pipeline),exitLoop(0)
{
	Ceng_CreateCriticalSection(&wakeCrit);
}

RenderThread::~RenderThread()
{
	if (wakeCrit != nullptr)
	{
		wakeCrit->Release();
	}
}

void RenderThread::Release()
{
	delete this;
}

const CRESULT RenderThread::Exit()
{
	exitLoop = 1;
	return CE_OK;
}

const CRESULT RenderThread::Execute()
{
	wakeCrit->Lock();

	++(*runningThreadCount);

	while(exitLoop == 0)
	{
		std::shared_ptr<RenderTask> task = pipeline->GetTask(threadId);
		
		if (task != nullptr)
		{
			CRESULT cresult = task->Execute(threadId,pipeline);
		}
		else
		{
			if (pipeline->remainingTasks.load() == 0)
			{
				if (pipeline->activeThreads.load() == 0)
				{
					cmdWake->WakeAll();
				}
		
				if (threadId > maxThreads->load())
				{
					if (threadId > minThreads->load())
					{
						--(*runningThreadCount);
						wakeCondition->Wait(wakeCrit);
						++(*runningThreadCount);
					}
				}
			}		
		}
		
	}

	--(*runningThreadCount);

	wakeCrit->Unlock();

	return CE_OK;
}