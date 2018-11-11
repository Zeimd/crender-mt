/*****************************************************************************
*
* rasterizer-batch.cpp
*
* Created By Jari Korkala 1/9/2014
*
*****************************************************************************/

#include "rasterizer-batch.h"

#include "shader-link.h"

using namespace Ceng;

RasterizerBatch::RasterizerBatch()
{
	
}

RasterizerBatch::~RasterizerBatch()
{
}

RasterizerBatch::RasterizerBatch(const Ceng::UINT32 apiCallId,
								 std::shared_ptr<RenderState> &renderState)
	: PipelineBatch(apiCallId,1,renderState)
{
}