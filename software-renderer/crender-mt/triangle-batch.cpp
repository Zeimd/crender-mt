/*****************************************************************************
*
* rasterizer-batch.cpp
*
* Created By Jari Korkala 1/9/2014
*
*****************************************************************************/

#include "triangle-batch.h"

using namespace Ceng;

TriangleBatch::TriangleBatch()
{
}

TriangleBatch::~TriangleBatch()
{
}

TriangleBatch::TriangleBatch(const Ceng::UINT32 apiCallId,const Ceng::UINT32 batchSize,
							 std::shared_ptr<RenderState> &renderState,
								 std::shared_ptr<AlignedBuffer<Ceng::UINT8>> &fragmentCache,
								 std::shared_ptr<AlignedBuffer<Ceng::UINT8>> &clipperCache) 
{
	this->apiCallId = apiCallId;
	this->batchSize = batchSize;
	this->renderState = renderState;

	this->fragmentCache = fragmentCache;
	this->cacheA = clipperCache;

	primitiveList.reserve(batchSize);
}