/*****************************************************************************
*
* clipper-batch.h
*
* Created By Jari Korkala 31/8/2014
*
*****************************************************************************/

#ifndef CRENDER_CLIPPER_BATCH_H
#define CRENDER_CLIPPER_BATCH_H

#include <memory>
#include <ceng/datatypes/aligned-buffer.h>

#include "pipeline-batch.h"
#include "primitive-data.h"

namespace Ceng
{
	class ClipperBatch : public PipelineBatch
	{
	public:

		/**
		 * Reference to the fragment cache this primitive comes from.
		 * Prevents the cache from being released before this primitive
		 * is rasterized.
		 */
		std::shared_ptr<AlignedBuffer<Ceng::UINT8>> fragmentCache;

		std::vector<CR_PrimitiveData> primitiveList;

		/**
		 * First clipper cache. Takes turns with cacheB as in/out when
		 * clipping against different planes in sequence.
		 */
		std::shared_ptr<AlignedBuffer<Ceng::UINT8>> cacheA;

		ClipperBatch();

	public:

		ClipperBatch(std::shared_ptr<DrawBatch> &batch);

		ClipperBatch(const Ceng::UINT32 apiCallId,const Ceng::UINT32 batchSize,
						std::shared_ptr<RenderState> &renderState,
						std::vector<CR_PrimitiveData> &&primitiveList,
						std::shared_ptr<AlignedBuffer<Ceng::UINT8>> &fragmentCache);

		virtual ~ClipperBatch();
	};
}

#endif