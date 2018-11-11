/*****************************************************************************
*
* task-clipper.cpp
*
* Created By Jari Korkala 2/2015
*
*****************************************************************************/

#include "task-clipper.h"

#include "clipper-batch.h"
#include "clipper.h"
#include "pipeline.h"

using namespace Ceng;

Task_Clipper::Task_Clipper()
{
}

Task_Clipper::~Task_Clipper()
{
}

Task_Clipper::Task_Clipper(std::shared_ptr<ClipperBatch> &clipperBatch)
	: clipperBatch(clipperBatch)
{
}

const CRESULT Task_Clipper::Execute(const Ceng::UINT32 threadId,Pipeline *pipeline)
{
	CRESULT cresult = CE_OK;

	cresult = clipperBatch->renderState->clipper->ClipPrimitives(clipperBatch,pipeline->triangleSetup);

	--pipeline->clipper.activeThreads;
	--pipeline->activeThreads;

	pipeline->WakeAllThreads();

	return cresult;
}

const CRESULT Task_Clipper::Prepare(const Ceng::UINT32 threadId,Pipeline *pipeline)
{
	return CE_OK;
}