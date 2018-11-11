/*****************************************************************************
*
* thread-test.cpp
*
* Created By Jari Korkala 4/9/2014
*
*****************************************************************************/

// Debug information
#ifdef _MSC_VER

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <crtdbg.h>

#endif

#endif

#include <vector>

#include <ceng.h>


#include <ceng/datatypes/sliding-window.h>
#include <ceng/datatypes/ring-buffer.h>

#include <memory>

using namespace Ceng;

/*
class Task : public Ceng::ThreadTask
{
public:
	
public:
	Task()
	{
	}

	virtual ~Task()
	{
	}

	virtual void Release()
	{
		delete this;
	}


	virtual const CRESULT Execute()
	{
	

		return CE_OK;
	}

	virtual const CRESULT Exit()
	{
		return CE_OK;
	}

};

namespace GroupStatus
{
	enum value
	{
		DISCARD = 1 ,
		FINAL = 2 ,
		
		FORCE_32B = 1 << 30 ,
	};
}

class RenderTask
{
public:

public:

	RenderTask()
	{
	}

	virtual ~RenderTask()
	{
	}

	virtual const CRESULT Prepare()=0;

	virtual const CRESULT Execute()=0;
};


template<class t_ElemType>
class TaskWindow
{
public:

	SlidingWindow<std::shared_ptr<t_ElemType>> taskWindow;

	Ceng::UINT32 status;
	Ceng::BOOL ready;

public:

	DrawTaskWindow()
	{
	}

	virtual ~DrawTaskWindow()
	{
	}

	const Ceng::BOOL IsReady() const;
	const Ceng::BOOL IsDiscard() const;
	const Ceng::BOOL IsFinal() const;

	const Ceng::BOOL IsValidId(const Ceng::UINT32 taskId)
	{
		return taskWindow.IsValidId(taskId);
	}

	std::shared_ptr<t_ElemType> GetTask(const Ceng::UINT32 taskId)
	{
		return taskWindow.GetSlot(taskId);
	}

	const CRESULT WriteTask(const Ceng::UINT32 taskId,std::shared_ptr<t_ElemType> task)
	{
		return taskWindow.WriteSlot(taskId,task);
	}
};
*/

#include <atomic>

int main()
{
	
#ifdef _DEBUG
	// Detect memory leaks after main-function ends
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
#endif

	//TaskWindow<TaskWindow<RenderTask>> taskManager;

	Ceng::CriticalSection *crit;

	Ceng_CreateCriticalSection(&crit);

	crit->Lock();

	crit->Unlock();

	crit->Release();



	return 0;
}