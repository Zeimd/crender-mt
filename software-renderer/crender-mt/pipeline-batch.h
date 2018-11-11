/*****************************************************************************
*
* pipeline-batch.h
*
* Created By Jari Korkala 31/8/2014
*
*****************************************************************************/

#ifndef CRENDER_RENDER_BATCH_H
#define CRENDER_RENDER_BATCH_H

#include <memory>

#include "render-state.h"

namespace Ceng
{
	class PipelineBatch
	{
	public:

		Ceng::UINT32 apiCallId;

		Ceng::UINT32 batchSize;

		std::shared_ptr<RenderState> renderState;

		PipelineBatch();

	public:

		PipelineBatch(const Ceng::UINT32 apiCallId,const Ceng::UINT32 batchSize,
			std::shared_ptr<RenderState> &renderState);

		virtual ~PipelineBatch();
	};
}

#endif