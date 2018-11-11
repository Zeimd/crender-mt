/*****************************************************************************
*
* command-thread.cpp
*
* Created By Jari Korkala 30/9/2014
*
*****************************************************************************/


#include "command-thread.h"

#include "api-command.h"
#include "crender-context.h"

#include <ceng/platform.h>
#include <ceng/datatypes/release-deleter.h>

using namespace Ceng;

CommandProcessor::CommandProcessor() 
	: exitThread(0),renderer(nullptr),wakeCondition(nullptr,ReleaseDeleter<ConditionVariable>()),wakeCrit(nullptr)
{
}

CommandProcessor::CommandProcessor(CR_RenderContext *renderer,std::shared_ptr<ConditionVariable> &wakeCondition)
	: renderer(renderer),wakeCondition(wakeCondition),exitThread(0)
{
	Ceng_CreateCriticalSection(&wakeCrit);
}

CommandProcessor::~CommandProcessor()
{
	if (wakeCrit != nullptr)
	{
		wakeCrit->Release();
	}
}

void CommandProcessor::Release()
{
	delete this;
}

CommandProcessor& CommandProcessor::operator = (CommandProcessor &&source)
{
	if (&source == this)
	{
		return *this;
	}

	if (wakeCrit != nullptr)
	{
		wakeCrit->Release();
	}

	wakeCrit = source.wakeCrit;
	source.wakeCrit = nullptr;

	wakeCondition = source.wakeCondition;
	exitThread = source.exitThread;
	renderer = source.renderer;

	return *this;
}

const CRESULT CommandProcessor::Execute()
{
	wakeCrit->Lock();

	while( !(exitThread) )
	{
		if ( !(renderer->commandQueue.IsEmpty()) )
		{
			// TODO: Determine if the first command can be executed

			std::shared_ptr<ApiCommand> command = renderer->commandQueue.Front();

			CRESULT cresult = command->Prepare(renderer);

			if (cresult == CE_OK)
			{
				renderer->commandQueue.PopFront();

				CRESULT cresult = command->Execute(renderer);

				if (cresult != CE_OK)
				{
					// TODO: Find a way to signal error asynchronously
				}
			}
		}
		else
		{
			// TODO: Wait on condition variable
		}
	};

	wakeCrit->Unlock();

	return CE_OK;
}

const CRESULT CommandProcessor::Exit()
{
	exitThread = 1;

	return CE_OK;
}