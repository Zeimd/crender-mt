/*****************************************************************************
*
* command-thread.h
*
* Created By Jari Korkala 30/9/2014
*
*****************************************************************************/

#ifndef CENG_COMMAND_THREAD_H
#define CENG_COMMAND_THREAD_H

#include <memory>


#include <ceng/interfaces/thread-task.h>

#include <ceng/datatypes/boolean.h>

namespace Ceng
{
	class CR_RenderContext;
	class ConditionVariable;
	class CriticalSection;

	class CommandProcessor : public ThreadTask
	{
	public:

		CR_RenderContext *renderer;

		volatile Ceng::UINT32 exitThread;

		std::shared_ptr<ConditionVariable> wakeCondition;
		CriticalSection *wakeCrit;

		CommandProcessor();

	public:
		
		CommandProcessor(CR_RenderContext *renderer,std::shared_ptr<ConditionVariable> &wakeCondition);

		virtual ~CommandProcessor();

		CommandProcessor& operator = (CommandProcessor &&source);

		virtual void Release() override;

		virtual const CRESULT Execute() override;
		virtual const CRESULT Exit() override;

	};
}

#endif