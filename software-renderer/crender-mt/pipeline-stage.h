/*****************************************************************************
*
* pipeline-stage.h
*
* Created By Jari Korkala 2/2015
*
*****************************************************************************/

#ifndef CENG_PIPELINE_STAGE_H
#define CENG_PIPELINE_STAGE_H

#include <atomic>
#include <memory>
#include <ceng/datatypes.h>

namespace Ceng
{
	class Pipeline;
	class RenderTask;

	class PipelineStage
	{
	public:

		std::atomic<Ceng::UINT32> remainingTasks;
		std::atomic<Ceng::UINT32> activeThreads;

	protected:

		void CopyAtomics(const PipelineStage &source);

		PipelineStage& operator = (const PipelineStage &source);

	public:

		PipelineStage();
		virtual ~PipelineStage();

		

		PipelineStage& operator = (PipelineStage &&source);

		const Ceng::UINT32 RemainingTasks() const;
		const Ceng::UINT32 ActiveThreads() const;

		virtual const Ceng::CRESULT GetTask(const Ceng::UINT32 threadId,Pipeline *pipeline,
												std::shared_ptr<RenderTask> &out_task)=0;
	};
}

#endif