/*****************************************************************************
*
* locking-queue.cpp
*
* Created By Jari Korkala 2/2015
*
*****************************************************************************/

#include "locking-queue.h"

#include <ceng/platform.h>

using namespace Ceng;


LockingQueue::LockingQueue() : readExclusion(nullptr),writeLock(nullptr)
{

}

LockingQueue::LockingQueue(const Ceng::UINT32 size,const Ceng::UINT32 alignment)
{
	queue = RingBuffer<std::shared_ptr<LockingTask>>::Allocate(size,alignment);

	CRESULT cresult = Ceng_CreateCriticalSection(&readExclusion);

	cresult = Ceng_CreateCriticalSection(&writeLock);
}

LockingQueue::~LockingQueue()
{
	if (readExclusion != nullptr)
	{
		readExclusion->Release();
	}

	if (writeLock != nullptr)
	{
		writeLock->Release();
	}
}

LockingQueue::LockingQueue(LockingQueue &&source)
{
	queue = std::move(source.queue);
	readExclusion = source.readExclusion;
	writeLock = source.writeLock;

	source.readExclusion = nullptr;
	source.writeLock = nullptr;
}

LockingQueue& LockingQueue::operator=(LockingQueue &&source)
{
	if (this == &source)
	{
		return *this;
	}

	if (readExclusion != nullptr)
	{
		readExclusion->Release();
	}

	if (writeLock != nullptr)
	{
		writeLock->Release();
	}

	queue = std::move(source.queue);
	readExclusion = source.readExclusion;
	writeLock = source.writeLock;

	source.readExclusion = nullptr;
	source.writeLock = nullptr;

	return *this;
}

const Ceng::CRESULT LockingQueue::AddTask(std::shared_ptr<LockingTask> &task)
{
	Ceng::CRESULT cresult;

	writeLock->Lock();

	cresult = queue.PushBack(task);

	writeLock->Unlock();

	return cresult;
}

const Ceng::CRESULT LockingQueue::GetTask(const Ceng::UINT32 threadId,
											  Pipeline *pipeline,std::shared_ptr<RenderTask> &out_task)
{
	CRESULT cresult;

	out_task = nullptr;

	cresult = readExclusion->AttemptLock();

	if (cresult != CE_OK)
	{
		return CE_OK;
	}

	if (queue.IsEmpty())
	{
		readExclusion->Unlock();
		return CE_OK;
	}

	std::shared_ptr<LockingTask> task = queue.Front();

	cresult = task->Prepare(threadId,pipeline);

	if (cresult != CE_OK)
	{
		readExclusion->Unlock();
		return CE_OK;
	}

	queue.PopFront();

	task->lock = readExclusion;
	out_task = task;	

	return CE_OK;
}