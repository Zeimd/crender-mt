/*****************************************************************************
*
* draw-batch.h
*
* By Jari Korkala 8/2014
*
*****************************************************************************/

#ifndef CRENDER_DRAW_BATCH_H
#define CRENDER_DRAW_BATCH_H

#include <memory>
#include <ceng/datatypes/aligned-buffer.h>

#include "draw-call.h"

#include "render-state.h"
#include "pipeline-batch.h"

#include "primitive-data.h"

#include "cache-tag.h"

namespace Ceng
{
	class DrawBatch : public PipelineBatch
	{
	public:

		Ceng::UINT32 fragSizeBytes;

		Ceng::UINT32 vertexCount;

		std::vector<FragmentCacheTag> fragmentIndex;

		std::vector<CR_PrimitiveData> primitiveList;

		std::shared_ptr<AlignedBuffer<Ceng::UINT8>> fragmentCache;

	protected:

		DrawBatch();

	public:

		DrawBatch(const Ceng::UINT32 apiCallId,const Ceng::UINT32 batchSize,
			std::shared_ptr<RenderState> &renderState); 
		
		~DrawBatch();

		/**
		 * Add a vertex index (tag) to the cache if it isn't there
		 * already, and there is free space in the cache.
		 */
		const Ceng::INT32 CacheAlloc(const Ceng::UINT32 tag);

		/**
		 * If a vertex with given index value (tag) is in the cache,
		 * returns address of that fragment.
		 */
		Ceng::UINT8* CacheSearch(const Ceng::UINT32 tag);

		/**
		 * Returns the address of given index at raw cache data structure.
		 */
		Ceng::UINT8* CacheAbsoluteAddress(const Ceng::UINT32 index);

		/**
		 * Returns the number of vertices that can be added
		 * into this batch.
		 */
		const Ceng::INT32 GetEmptySpace() const;

	};
}

#endif