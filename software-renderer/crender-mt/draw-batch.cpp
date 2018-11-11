/*****************************************************************************
*
* draw-batch.cpp
*
* By Jari Korkala 8/2014
*
*****************************************************************************/

#include "draw-batch.h"

#include "render-state.h"

#include "shader-link.h"

using namespace Ceng;

DrawBatch::DrawBatch(const Ceng::UINT32 apiCallId,const Ceng::UINT32 batchSize,
					 std::shared_ptr<RenderState> &renderState)
					 : PipelineBatch(apiCallId,batchSize,renderState),
					 vertexCount(0)
{	
//	callBuffer.reserve(batchSize);

	fragmentIndex = std::vector<FragmentCacheTag>(batchSize,FragmentCacheTag());

	fragmentCache = std::shared_ptr<AlignedBuffer<UINT8>>
		(renderState->shaderLink->GetFragmentCache(batchSize));

	fragSizeBytes = renderState->shaderLink->fragmentFormat.size;
}

DrawBatch::DrawBatch()
{
	vertexCount = 0;
}

DrawBatch::~DrawBatch()
{
}

const Ceng::INT32 DrawBatch::CacheAlloc(const Ceng::UINT32 tag)
{
	Ceng::UINT32 k;

	for(k=0;k<fragmentIndex.size();k++)
	{
		if (fragmentIndex[k].tag == tag)
		{
			return fragmentIndex[k].cacheIndex;
		}

		if (fragmentIndex[k].tag == -1)
		{
			fragmentIndex[k].tag = tag;
			fragmentIndex[k].cacheIndex = vertexCount;
			
			fragmentIndex[k].address = &( (*fragmentCache)[vertexCount*fragSizeBytes]);

			++vertexCount;

			return fragmentIndex[k].cacheIndex;
		}
	}

	// Cache is full

	return -1;
}

Ceng::UINT8* DrawBatch::CacheSearch(const Ceng::UINT32 tag)
{
	Ceng::UINT32 k;

	for(k=0;k<fragmentIndex.size();k++)
	{
		if (fragmentIndex[k].tag == tag)
		{
			return fragmentIndex[k].address;
		}
	}

	return nullptr;
}

Ceng::UINT8* DrawBatch::CacheAbsoluteAddress(const Ceng::UINT32 index)
{
	return fragmentIndex[index].address;
}

const Ceng::INT32 DrawBatch::GetEmptySpace() const
{
	return batchSize-vertexCount;
}