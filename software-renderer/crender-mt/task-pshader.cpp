/*****************************************************************************
*
* task-pshader.cpp
*
* Created By Jari Korkala 9/10/2014
*
*****************************************************************************/

#include "task-pshader.h"
#include "pipeline.h"

#include "rasterizer-batch.h"

using namespace Ceng;

Task_PixelShader::Task_PixelShader()
{
}

Task_PixelShader::~Task_PixelShader()
{
}

Task_PixelShader::Task_PixelShader(std::shared_ptr<RasterizerBatch> &rasterizerBatch)
	: rasterizerBatch(rasterizerBatch)
{
	quadList = LeafVector<CR_QuadHeader>(128,64);

	quadCount = 0;
}

const CRESULT Task_PixelShader::Execute(const Ceng::UINT32 threadId,Pipeline *pipeline)
{
	CRESULT cresult = CE_OK;

	cresult = rasterizerBatch->renderState->pshaderInstance[threadId]->ProcessQuads(this,threadId);

	--pipeline->pixelShader.activeThreads;
	--pipeline->activeThreads;

	pipeline->WakeAllThreads();

	return cresult;
}

const CRESULT Task_PixelShader::Prepare(const Ceng::UINT32 threadId,Pipeline *pipeline)
{
	return CE_OK;
}