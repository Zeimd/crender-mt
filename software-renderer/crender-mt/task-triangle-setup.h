/*****************************************************************************
*
* task-triangle-setup.h
*
* Created By Jari Korkala 2/2015
*
*****************************************************************************/

#ifndef CENG_TASK_TRIANGLE_SETUP_H
#define CENG_TASK_TRIANGLE_SETUP_H

#include <memory>

#include "locking-task.h"

namespace Ceng
{
	class TriangleBatch;
	class Pipeline;

	class Task_TriangleSetup : public LockingTask
	{
	protected:

		std::shared_ptr<TriangleBatch> triangleBatch;

	protected:

		Task_TriangleSetup();
	
	public:

		Task_TriangleSetup(std::shared_ptr<TriangleBatch> &triangleBatch);

		virtual ~Task_TriangleSetup();

		virtual const CRESULT Prepare(const Ceng::UINT32 threadId,Pipeline *pipeline) override;
		virtual const CRESULT Execute(const Ceng::UINT32 threadId,Pipeline *pipeline) override;
	};
}

#endif