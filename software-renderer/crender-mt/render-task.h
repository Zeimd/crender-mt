/*****************************************************************************
*
* render-task.h
*
* Created By Jari Korkala 9/10/2014
*
*****************************************************************************/

#ifndef CRENDER_RENDER_TASK_H
#define CRENDER_RENDER_TASK_H

#include <ceng/datatypes/basic-types.h>
#include <ceng/datatypes/return-val.h>

namespace Ceng
{
	class ConceptRenderer;
	class Pipeline;

	class RenderTask
	{
	public:

		Ceng::UINT32 apiCallId;

	public:
		RenderTask()
		{
		}

		virtual ~RenderTask()
		{
		}

		virtual const CRESULT Prepare(const Ceng::UINT32 threadId,Pipeline *pipeline)=0;

		virtual const CRESULT Execute(const Ceng::UINT32 threadId,Pipeline *pipeline)=0;
	};
}

#endif