/*****************************************************************************
*
* rasterizer-batch.h
*
* Created By Jari Korkala 1/9/2014
*
*****************************************************************************/

#ifndef CRENDER_RASTERIZER_BATCH_H
#define CRENDER_RASTERIZER_BATCH_H

#include <memory>

#include "clipper-batch.h"

namespace Ceng
{
	class TriangleBatch : public ClipperBatch
	{
	protected:

		TriangleBatch();
	public:

		~TriangleBatch();

		TriangleBatch(const Ceng::UINT32 apiCallId,const Ceng::UINT32 batchSize,
						std::shared_ptr<RenderState> &renderState,
						std::shared_ptr<AlignedBuffer<Ceng::UINT8>> &fragmentCache,
						std::shared_ptr<AlignedBuffer<Ceng::UINT8>> &clipperCache);
	};
}

#endif