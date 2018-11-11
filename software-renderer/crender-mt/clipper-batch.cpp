/*****************************************************************************
*
* clipper-batch.cpp
*
* Created By Jari Korkala 31/8/2014
*
*****************************************************************************/

#include "clipper-batch.h"

#include "render-state.h"
#include "shader-link.h"

#include "draw-batch.h"

using namespace Ceng;

ClipperBatch::ClipperBatch()
{
}

ClipperBatch::~ClipperBatch()
{
}

ClipperBatch::ClipperBatch(const Ceng::UINT32 apiCallId,const Ceng::UINT32 batchSize,
						   std::shared_ptr<RenderState> &renderState,
						   std::vector<CR_PrimitiveData> &&primitiveList,
						   std::shared_ptr<AlignedBuffer<Ceng::UINT8>> &fragmentCache) :
	PipelineBatch(apiCallId,batchSize,renderState),fragmentCache(fragmentCache),primitiveList(primitiveList)
{
	cacheA = std::shared_ptr<AlignedBuffer<Ceng::UINT8>>(renderState->shaderLink->GetClipperCache(batchSize));
}

ClipperBatch::ClipperBatch(std::shared_ptr<DrawBatch> &batch)
	: PipelineBatch(batch->apiCallId,batch->batchSize,batch->renderState),
	fragmentCache(batch->fragmentCache),primitiveList(std::move(batch->primitiveList))
{
	cacheA = std::shared_ptr<AlignedBuffer<Ceng::UINT8>>(renderState->shaderLink->GetClipperCache(batchSize));
}