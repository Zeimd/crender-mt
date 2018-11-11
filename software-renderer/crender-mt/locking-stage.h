/*****************************************************************************
*
* locking-stage.h
*
* Created By Jari Korkala 2/2015
*
*****************************************************************************/

#ifndef CENG_LOCKING_STAGE_H
#define CENG_LOCKING_STAGE_H

#include "locking-queue.h"
#include "pipeline-stage.h"

namespace Ceng
{
	class Pipeline;

	class LockingStage : public PipelineStage
	{
	protected:

		Pipeline *pipeline;

		std::vector<LockingQueue> queue;

		LockingStage& operator = (const LockingStage &source);

	public:
		LockingStage();
		virtual ~LockingStage();

		LockingStage(const Ceng::UINT32 buckets,const Ceng::UINT32 queueSize,
						Pipeline *pipeline,const Ceng::UINT32 alignment);

		LockingStage& operator = (LockingStage &&source);

		const Ceng::UINT32 Buckets() const;

		const Ceng::CRESULT AddTask(const Ceng::UINT32 bucket,
										std::shared_ptr<LockingTask> &task);

		virtual const Ceng::CRESULT GetTask(const Ceng::UINT32 threadId,Pipeline *pipeline,
												std::shared_ptr<RenderTask> &out_task) override;
	};
}

#endif