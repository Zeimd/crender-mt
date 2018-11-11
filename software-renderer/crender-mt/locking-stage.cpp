/*****************************************************************************
*
* locking-stage.cpp
*
* Created By Jari Korkala 2/2015
*
*****************************************************************************/

#include "locking-stage.h"

#include "pipeline.h"

using namespace Ceng;

LockingStage::LockingStage()
{
}

LockingStage::~LockingStage()
{
}

LockingStage::LockingStage(const Ceng::UINT32 buckets,const Ceng::UINT32 queueSize,
						   Pipeline *pipeline,const Ceng::UINT32 alignment)
	: pipeline(pipeline)
{

	queue = std::vector<LockingQueue>(buckets);

	for(Ceng::UINT32 k=0;k<buckets;k++)
	{
		queue[k] = std::move(LockingQueue(queueSize,alignment));
	}
}

LockingStage& LockingStage::operator = (LockingStage &&source)
{
	if (this == &source)
	{
		return *this;
	}

	pipeline = source.pipeline;

	CopyAtomics(source);

	queue = std::move(source.queue);

	return *this;
}

const Ceng::UINT32 LockingStage::Buckets() const
{
	return queue.size();
}

const Ceng::CRESULT LockingStage::AddTask(const Ceng::UINT32 bucket,std::shared_ptr<LockingTask> &task)
{
	CRESULT cresult = queue[bucket].AddTask(task);

	if (cresult == CE_OK)
	{
		++remainingTasks;
		++pipeline->remainingTasks;
	}

	return cresult;
}

const Ceng::CRESULT LockingStage::GetTask(const Ceng::UINT32 threadId,Pipeline *pipeline,
										  std::shared_ptr<RenderTask> &out_task)
{
	if (remainingTasks.load() == 0)
	{
		out_task = nullptr;
		return CE_OK;
	}

	CRESULT cresult;

	for(Ceng::UINT32 k=0;k<queue.size();k++)
	{
		cresult = queue[k].GetTask(threadId,pipeline,out_task);

		if (out_task != nullptr)
		{
			++activeThreads;
			--remainingTasks;

			return CE_OK;
		}
	}

	return CE_OK;
}