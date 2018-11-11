/*****************************************************************************
*
* rasterizer-batch.h
*
* Created By Jari Korkala 1/9/2014
*
*****************************************************************************/

#ifndef CRENDER_RASTERIZER_BATCH_H_T
#define CRENDER_RASTERIZER_BATCH_H_T

#include <memory>

#include "render-state.h"
#include "triangle-data.h"
#include "pipeline-batch.h"

namespace Ceng
{
	class RasterizerBatch : public PipelineBatch
	{
	public:

		Ceng::UINT32 bucketId;

		// Total number of tasks sent to rasterizer from this api call
		// NOTE: -1 indicates this is not the last task
		//Ceng::UINT32 totalCount;

		// Number of tasks completed by rasterizer from this api call
		//std::atomic<Ceng::UINT32> *counter; 

		std::shared_ptr<CR_TriangleData> triangle;

		CR_TileValues tileValues;

		// NOTE: Initial edge equation values are 
		//       evaluated at (xmin,ymin).

		/**
		 * Edge equation values for 2x2 pixels.
		 * Must be 16-byte aligned.
		 */
		CR_QuadEdges quadEdgeValue[3]; 

		Ceng::INT32 xMin,yMin;
		Ceng::INT32 xMax,yMax;

		std::shared_ptr<AlignedBuffer<CR_FloatFragment>> floatVariables;
		std::shared_ptr<AlignedBuffer<CR_DoubleFragment>> doubleVariables;
		std::shared_ptr<AlignedBuffer<Ceng::UINT8>> shaderSteps;

	public:

		RasterizerBatch();

		~RasterizerBatch();

		RasterizerBatch(const Ceng::UINT32 apiCallId,
							std::shared_ptr<RenderState> &renderState);


	};
}

#endif