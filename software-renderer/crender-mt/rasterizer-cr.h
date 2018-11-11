/*****************************************************************************
*
* rasterizer-cr.h
*
* By Jari Korkala 2/2012
*
* Rasterizer functionality of Concept Renderer moved to a separate
* class.
*
*****************************************************************************/

#ifndef _CONCEPT2_RASTERIZER_CR_H
#define _CONCEPT2_RASTERIZER_CR_H

#include <memory>

#include <ceng/interfaces/rasterizer-state.h>

#include <ceng/datatypes/rectangle.h>

#include <ceng/datatypes/ring-buffer.h>

#include "crender-base.h"

#include "cr-pixel-shader.h"

#include "triangle-batch.h"
#include "rasterizer-batch.h"

#include "locking-stage.h"

namespace Ceng
{
	
	struct CR_PackedTargetStep
	{
		UINT16 stepX;
		UINT16 stepY;
	};
	
	struct CR_FastTargetData
	{
		CR_PackedTargetStep stepping[4]; // Size 16 bytes
		POINTER baseAddress[4]; // Size 16 or 32 bytes
	};

	class CR_ShaderLink;

	struct CR_TriangleData;

	class CR_NewTargetData;
	
	//****************************************************************************
	// Rasterizer interface

	typedef RingBuffer<std::shared_ptr<RasterizerBatch>> RasterizerQueue;

	typedef std::vector<std::shared_ptr<RasterizerBatch>> BatchVector;
	
	class PixelShaderQueue;

	class Task_PixelShader;

	class CR_Rasterizer
	{
	public:
		/**
		 * Size of processor's L1 cache line in bytes.
		 */
		UINT32 cacheLineSize;

	public:
		CR_Rasterizer();
		virtual ~CR_Rasterizer();

		const CRESULT TriangleSetup(std::shared_ptr<TriangleBatch> &batch,
								LockingStage &outputStage);

		const CRESULT TriangleSetup(CR_PrimitiveData &primitive,
								std::shared_ptr<RenderState> &renderState,
								BatchVector &outputBatch,
								Ceng::UINT32 &buckets);

		void TriangleSetup_DiscardBatch(BatchVector &outputBatch);

		const CRESULT Rasterize(std::shared_ptr<RasterizerBatch> &batch,LockingStage &outputStage,
			const Ceng::UINT32 threadId);
	
		const Ceng::INT32 RasterizeTriangle(std::vector<Task_PixelShader*> &outputBatch,
												Ceng::Rectangle *scissorRect);

		Ceng::INT32 RasterizeTriangle_ChainWithinTile(CR_TriangleData *triangle,
														CR_NewTargetData *depthBuffer,
														Ceng::Rectangle *scissorRect);

	protected:
		CRESULT ConfigureDepthTest();
		
		UINT64 (*call_tile_CoverageTest)(CR_QuadEdges quadEdges[3],
										const CR_QuadEdges quadEdgeStepX[3],
										const CR_QuadEdges quadEdgeStepY[3],
										const CR_QuadEdges quadStepTileBack[3]);

		const CRESULT GenerateChain(Task_PixelShader *batch,
									const INT32 startQuadX,const INT32 endQuadX,
									const UINT32 *coverageMask,
									const INT32 chainY);
		
											

	};

	//************************************************************************
	// Tile coverage test functions

	extern UINT64 TILE_COVERAGE_TEST(CR_QuadEdges quadEdges[3],
								const CR_QuadEdges quadEdgeStepX[3],
								const CR_QuadEdges quadEdgeStepY[3],
								const CR_QuadEdges quadStepTileBack[3]);

	extern UINT64 TILE_COVERAGE_TEST_X86_SSE2(CR_QuadEdges quadEdges[3],
										const CR_QuadEdges quadEdgeStepX[3],
										const CR_QuadEdges quadEdgeStepY[3],
										const CR_QuadEdges quadStepTileBack[3]);

} // namespace Ceng

#endif // Include guard

