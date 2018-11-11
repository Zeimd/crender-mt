/*****************************************************************************
*
* task-clipper.h
*
* Created By Jari Korkala 2/2015
*
*****************************************************************************/

#ifndef CENG_TASK_CLIPPER_H
#define CENG_TASK_CLIPPER_H

#include <memory>
#include "locking-task.h"

namespace Ceng
{
	class ClipperBatch;

	class Task_Clipper : public LockingTask
	{
	protected:

		std::shared_ptr<ClipperBatch> clipperBatch;

		Task_Clipper();
	public:

		Task_Clipper(std::shared_ptr<ClipperBatch> &clipperBatch);

		virtual ~Task_Clipper();

		virtual const CRESULT Prepare(const Ceng::UINT32 threadId,Pipeline *pipeline) override;
		virtual const CRESULT Execute(const Ceng::UINT32 threadId,Pipeline *pipeline) override;
	};
}

#endif