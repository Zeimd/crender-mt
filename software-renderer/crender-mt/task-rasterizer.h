/*****************************************************************************
*
* task-rasterizer.h
*
* Created By Jari Korkala 2/2015
*
*****************************************************************************/

#ifndef TASK_RASTERIZER_H
#define TASK_RASTERIZER_H

#include <memory>

#include "locking-task.h"

namespace Ceng
{
	class RasterizerBatch;
	class Pipeline;

	class Task_Rasterizer : public LockingTask
	{
	public:

		std::shared_ptr<RasterizerBatch> rasterizerBatch;

	public:

		Task_Rasterizer();
		virtual ~Task_Rasterizer();

		Task_Rasterizer(std::shared_ptr<RasterizerBatch> &rasterizerBatch);

		virtual const CRESULT Prepare(const Ceng::UINT32 threadId,Pipeline *pipeline) override;
		virtual const CRESULT Execute(const Ceng::UINT32 threadId,Pipeline *pipeline) override;
	};
}

#endif