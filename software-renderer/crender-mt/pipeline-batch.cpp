/*****************************************************************************
*
* pipeline-batch.cpp
*
* Created By Jari Korkala 31/8/2014
*
*****************************************************************************/

#include "pipeline-batch.h"

using namespace Ceng;

PipelineBatch::PipelineBatch()
{
}

PipelineBatch::~PipelineBatch()
{
}

PipelineBatch::PipelineBatch(const Ceng::UINT32 apiCallId,const Ceng::UINT32 batchSize,
							 std::shared_ptr<RenderState> &renderState)
							 : apiCallId(apiCallId),batchSize(batchSize),renderState(renderState)
{
}