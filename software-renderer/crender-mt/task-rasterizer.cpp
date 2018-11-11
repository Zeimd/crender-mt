/*****************************************************************************
*
* task-rasterizer.cpp
*
* Created By Jari Korkala 2/2015
*
*****************************************************************************/

#include "task-rasterizer.h"

#include "rasterizer-batch.h"
#include "pipeline.h"

#include "rasterizer-cr.h"

using namespace Ceng;

Task_Rasterizer::Task_Rasterizer()
{
}

Task_Rasterizer::~Task_Rasterizer()
{
}


Task_Rasterizer::Task_Rasterizer(std::shared_ptr<RasterizerBatch> &rasterizerBatch)
	: rasterizerBatch(rasterizerBatch)
{
}


const CRESULT Task_Rasterizer::Execute(const Ceng::UINT32 threadId,Pipeline *pipeline)
{
	//CRESULT cresult = CE_OK;

	CRESULT cresult = rasterizerBatch->renderState->rasterizer->
		Rasterize(rasterizerBatch,pipeline->pixelShader,threadId);

	--pipeline->rasterizer.activeThreads;
	--pipeline->activeThreads;

	pipeline->WakeAllThreads();

	return cresult;
}

const CRESULT Task_Rasterizer::Prepare(const Ceng::UINT32 threadId,Pipeline *pipeline)
{
	return CE_OK;
}