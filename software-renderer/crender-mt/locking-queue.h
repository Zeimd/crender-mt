/*****************************************************************************
*
* locking-queue.h
*
* Created By Jari Korkala 2/2015
*
* A queue that allows only one thread to execute from it at the same time.
* The queue is unlocked after a task is finished.
*
*****************************************************************************/

#ifndef CENG_LOCKING_QUEUE_H
#define CENG_LOCKING_QUEUE_H

#include <memory>

#include "locking-task.h"

#include <ceng/datatypes/ring-buffer.h>

namespace Ceng
{
	class LockingQueue
	{
		RingBuffer<std::shared_ptr<LockingTask>> queue;

		Ceng::CriticalSection *readExclusion;

		Ceng::CriticalSection *writeLock;

	protected:

		LockingQueue(const LockingQueue &source);
		LockingQueue& operator = (const LockingQueue &source);

	public:

		LockingQueue();

		virtual ~LockingQueue();

		LockingQueue(const Ceng::UINT32 size,const Ceng::UINT32 alignment);

		
		LockingQueue(LockingQueue &&source);

		
		LockingQueue& operator = (LockingQueue &&source);

		const Ceng::CRESULT AddTask(std::shared_ptr<LockingTask> &task);

		const Ceng::CRESULT GetTask(const Ceng::UINT32 threadId,Pipeline *pipeline,
										std::shared_ptr<RenderTask> &out_task);

	};
}

#endif