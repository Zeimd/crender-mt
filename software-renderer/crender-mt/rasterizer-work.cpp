/*****************************************************************************
*
* cr-raster.cpp
*
* By Jari Korkala 1/2012
*
* Portable version of the half-space rasterizer
*
*****************************************************************************/

#include "rasterizer-cr.h"

#include "triangle-data.h"

#include "rtarget-data.h"

#include "quad-format.h"

#include "shader-link.h"

#include "task-pshader.h"

#include "locking-queue.h"

using namespace Ceng;

const CRESULT CR_Rasterizer::Rasterize(std::shared_ptr<RasterizerBatch> &batch,
										LockingStage &outputStage, const Ceng::UINT32 threadId)
{
	std::vector<Task_PixelShader*> outputs(3);

	for (Ceng::UINT32 k = 0; k < 3; ++k)
	{
		outputs[k] = new Task_PixelShader(batch);
	}

	Ceng::INT32 quadCount=0;

	RasterizeTriangle(outputs,&batch->renderState->activeRect);

	for (Ceng::UINT32 k = 0; k < 3; ++k)
	{
		if (outputs[k]->quadCount > 0)
		{
			outputStage.AddTask(3*batch->bucketId+k, std::shared_ptr<LockingTask>(outputs[k]));
		}
		else
		{
			delete outputs[k];
		}
	}
	
	return CE_OK;
}

const Ceng::INT32 CR_Rasterizer::RasterizeTriangle(std::vector<Task_PixelShader*> &outputs,
											 Ceng::Rectangle *scissorRect)
{
	//******************************************************
	// Create local copies of variables

	_declspec(align(16)) INT32 tileTrivialAccept[4];
	_declspec(align(16)) INT32 tileTrivialReject[4];

	CR_TriangleData *triangle = outputs[0]->rasterizerBatch->triangle.get();
	
	memcpy(tileTrivialAccept,outputs[0]->rasterizerBatch->tileValues.trivialAccept,4*sizeof(INT32));
	memcpy(tileTrivialReject,outputs[0]->rasterizerBatch->tileValues.trivialReject,4*sizeof(INT32));
	
	_declspec(align(16)) CR_QuadEdges quadEdgeValue[3];
	
	memcpy(quadEdgeValue,outputs[0]->rasterizerBatch->quadEdgeValue,3*sizeof(CR_QuadEdges));

	_declspec(align(16)) CR_QuadEdges quadStep_X[3];

	quadStep_X[0].pixel[0] = triangle->quadEdgeStepX[0];
	quadStep_X[0].pixel[1] = triangle->quadEdgeStepX[0];
	quadStep_X[0].pixel[2] = triangle->quadEdgeStepX[0];
	quadStep_X[0].pixel[3] = triangle->quadEdgeStepX[0];

	quadStep_X[1].pixel[0] = triangle->quadEdgeStepX[1];
	quadStep_X[1].pixel[1] = triangle->quadEdgeStepX[1];
	quadStep_X[1].pixel[2] = triangle->quadEdgeStepX[1];
	quadStep_X[1].pixel[3] = triangle->quadEdgeStepX[1];

	quadStep_X[2].pixel[0] = triangle->quadEdgeStepX[2];
	quadStep_X[2].pixel[1] = triangle->quadEdgeStepX[2];
	quadStep_X[2].pixel[2] = triangle->quadEdgeStepX[2];
	quadStep_X[2].pixel[3] = triangle->quadEdgeStepX[2];

	_declspec(align(16)) CR_QuadEdges quadStep_Y[3];

	quadStep_Y[0].pixel[0] = triangle->quadEdgeStepY[0];
	quadStep_Y[0].pixel[1] = triangle->quadEdgeStepY[0];
	quadStep_Y[0].pixel[2] = triangle->quadEdgeStepY[0];
	quadStep_Y[0].pixel[3] = triangle->quadEdgeStepY[0];

	quadStep_Y[1].pixel[0] = triangle->quadEdgeStepY[1];
	quadStep_Y[1].pixel[1] = triangle->quadEdgeStepY[1];
	quadStep_Y[1].pixel[2] = triangle->quadEdgeStepY[1];
	quadStep_Y[1].pixel[3] = triangle->quadEdgeStepY[1];

	quadStep_Y[2].pixel[0] = triangle->quadEdgeStepY[2];
	quadStep_Y[2].pixel[1] = triangle->quadEdgeStepY[2];
	quadStep_Y[2].pixel[2] = triangle->quadEdgeStepY[2];
	quadStep_Y[2].pixel[3] = triangle->quadEdgeStepY[2];

	_declspec(align(16)) CR_QuadEdges quadStep_tileBack[3];

	quadStep_tileBack[0].pixel[0] = triangle->quadStepTileBack[0];
	quadStep_tileBack[0].pixel[1] = triangle->quadStepTileBack[0];
	quadStep_tileBack[0].pixel[2] = triangle->quadStepTileBack[0];
	quadStep_tileBack[0].pixel[3] = triangle->quadStepTileBack[0];

	quadStep_tileBack[1].pixel[0] = triangle->quadStepTileBack[1];
	quadStep_tileBack[1].pixel[1] = triangle->quadStepTileBack[1];
	quadStep_tileBack[1].pixel[2] = triangle->quadStepTileBack[1];
	quadStep_tileBack[1].pixel[3] = triangle->quadStepTileBack[1];

	quadStep_tileBack[2].pixel[0] = triangle->quadStepTileBack[2];
	quadStep_tileBack[2].pixel[1] = triangle->quadStepTileBack[2];
	quadStep_tileBack[2].pixel[2] = triangle->quadStepTileBack[2];
	quadStep_tileBack[2].pixel[3] = triangle->quadStepTileBack[2];

	_declspec(align(16)) VectorF4 temp_packedZ,temp_packedW;

	_declspec(align(16)) VectorF4 temp_packedZ_dx,
									temp_packedZ_dy,
									temp_packedZ_quad_dx,
									temp_packedZ_quad_dy;

	temp_packedZ_dx = triangle->packedZ_dx;
	temp_packedZ_dy = triangle->packedZ_dy;

	temp_packedZ_quad_dx = temp_packedZ_dx*2;
	temp_packedZ_quad_dy = triangle->packedZ_quad_dy;

	UINT64 depthMask;
	UINT64 tileMask = 0;

	const INT32 FULL_CHAIN = 2;
	const INT32 PARTIAL_CHAIN = 1;

	INT32 chainStart[4];
	INT32 chainEnd[4];
	INT32 chainType[4];
	
	INT32 coverageIndex[4];

	UINT32 chainCoverage[4][4];

	//****************************************************************
	// Build scissor mask

	// TODO: Create these when a new scissor rect is set

	INT64 scissorMask[9] = {-1LL,-1LL,-1LL,-1LL,-1LL,-1LL,-1LL,-1LL,-1LL};

	//UINT64 scissorMask[9] = {0,0,0,0,-1,0,0,0,0};
	
	if ( (scissorRect->top & 7) || (scissorRect->left & 7) )
	{
			scissorMask[0] = 0;

			for(Ceng::INT32 k=(scissorRect->top & 7); k<8;k++)
			{
				for(Ceng::INT32 j=(scissorRect->left & 7); j<8;j++)
				{
					INT32 index = 16*(k>>1) + 4*(j>>1) + (j&1) + 2*(k&1);

					scissorMask[0] |= UINT64(1) << index;
				}
			}
	}

	if (scissorRect->top & 7)
	{
		scissorMask[1] = 0;

		for(Ceng::INT32 k=(scissorRect->top & 7); k<8;k++)
		{
			for(Ceng::INT32 j=0; j<8;j++)
			{
				INT32 index = 16*(k>>1) + 4*(j>>1) + (j&1) + 2*(k&1);

				scissorMask[1] |= UINT64(1) << index;
			}
		}
	}

	if ( (scissorRect->top & 7) || (scissorRect->right & 7))
	{
		scissorMask[2] = 0;

		INT32 hMax = scissorRect->right & 7;

		if (hMax == 0) hMax = 8;

		for(Ceng::INT32 k=(scissorRect->top & 7); k<8;k++)
		{
			for(Ceng::INT32 j=0; j < hMax; j++)
			{
				INT32 index = 16*(k>>1) + 4*(j>>1) + (j&1) + 2*(k&1);

				scissorMask[2] |= UINT64(1) << index;
			}
		}
	}
	
	if (scissorRect->left & 7)
	{
		scissorMask[3] = 0;

		for(Ceng::INT32 k=0; k<8;k++)
		{
			for(Ceng::INT32 j=(scissorRect->left & 7);j<8;j++)
			{
				INT32 index = 16*(k>>1) + 4*(j>>1) + (j&1) + 2*(k&1);

				scissorMask[3] |= UINT64(1) << index;
			}
		}
	}

	if (scissorRect->right & 7)
	{
		scissorMask[5] = 0;

		for(Ceng::INT32 k=0;k<8;k++)
		{
			for(Ceng::INT32 j=0;j<(scissorRect->right & 7);j++)
			{
				INT32 index = 16*(k>>1) + 4*(j>>1) + (j&1) + 2*(k&1);

				scissorMask[5] |= UINT64(1) << index;
			}
		}
	}

	if ( (scissorRect->bottom & 7) || (scissorRect->left & 7) )
	{
		scissorMask[6] = 0;

		INT32 hMax = scissorRect->bottom & 7;

		if (hMax == 0) hMax = 8;

		for(Ceng::INT32 k=0; k < hMax; k++)
		{
			for(Ceng::INT32 j=(scissorRect->left & 7); j<8;j++)
			{
				INT32 index = 16*(k>>1) + 4*(j>>1) + (j&1) + 2*(k&1);

				scissorMask[6] |= UINT64(1) << index;
			}
		}
	}

	if (scissorRect->bottom & 7)
	{
		scissorMask[7] = 0;

		for(Ceng::INT32 k=0;k<(scissorRect->bottom & 7);k++)
		{
			for(Ceng::INT32 j=0; j<8;j++)
			{
				INT32 index = 16*(k>>1) + 4*(j>>1) + (j&1) + 2*(k&1);

				scissorMask[7] |= UINT64(1) << index;
			}
		}
	}

	if ( (scissorRect->bottom & 7) || (scissorRect->right & 7) )
	{
		scissorMask[8] = 0;

		INT32 vMax = scissorRect->bottom & 7;
		if (vMax == 0) vMax = 8;

		INT32 hMax = scissorRect->right & 7;
		if (hMax == 0) hMax = 8;

		for(Ceng::INT32 k=0; k < vMax; k++)
		{
			for(Ceng::INT32 j=0; j < hMax; j++)
			{
				INT32 index = 16*(k>>1) + 4*(j>>1) + (j&1) + 2*(k&1);

				scissorMask[8] |= UINT64(1) << index;
			}
		}
	}

	CR_NewTargetData *depthBuffer;
	
	depthBuffer = outputs[0]->rasterizerBatch->renderState->depthBuffer.get();

	Ceng::INT32 xMin = outputs[0]->rasterizerBatch->xMin;
	Ceng::INT32 yMin = outputs[0]->rasterizerBatch->yMin;
	
	Ceng::INT32 xMax = outputs[0]->rasterizerBatch->xMax;
	Ceng::INT32 yMax = outputs[0]->rasterizerBatch->yMax;
	
	// Test tiles within the bounding box for triangle overlap

	for(Ceng::INT32 k=yMin;k<yMax;k+=8)
	{
		for(Ceng::UINT32 i=0;i<4;i++)
		{
			chainStart[i] = -1;			
			chainEnd[i] = -1;
			chainType[i] = -1;
			coverageIndex[i] = 0;

			chainCoverage[i][0] = 0;
			chainCoverage[i][1] = 0;
			chainCoverage[i][2] = 0;
			chainCoverage[i][3] = 0;
		}

		for(Ceng::INT32 j=xMin;j<xMax;j+=8)
		{
			// Trivial reject test

			// NOTE: Means that the triangle isn't over this tile,
			//       so doesn't include depth-stencil test result

			if ((tileTrivialReject[0] & tileTrivialReject[1] & tileTrivialReject[2]) >= 0)
			{
				// Flush active chains
				
				for(Ceng::UINT32 i=0;i<4;i++)
				{
					if (chainStart[i] != -1)
					{
						GenerateChain(outputs[i%3],chainStart[i],chainEnd[i],
										chainCoverage[i],k+2*i);
					}
				
					chainStart[i] = -1;
					chainEnd[i] = -1;
					chainType[i] = -1;
				}
				

				goto label_tileSkip;
			}

			// Scissor mask selection

			INT32 scissorIndex = 4; // Fully inside (always -1)

			if (k <= scissorRect->top)
			{
				scissorIndex = 1; // Top-middle

				if (j <= scissorRect->left)
				{
					scissorIndex = 0; // Top-left
				}
				else if (j >= scissorRect->right-8)
				{
					scissorIndex = 2; // Top-right
				}
			}
			else if (k >= scissorRect->bottom-8)
			{
				scissorIndex = 7; // Bottom-middle

				if (j <= scissorRect->left)
				{
					scissorIndex = 6; // Bottom-left
				}
				else if (j >= scissorRect->right-8)
				{
					scissorIndex = 8; // Bottom-right
				}
			}
			else
			{
				if (j <= scissorRect->left)
				{
					scissorIndex = 3;
				}
				else if (j >= scissorRect->right-8)
				{
					scissorIndex = 5;
				}
			}

			// Pixel coverage test
			if ((tileTrivialAccept[0] & tileTrivialAccept[1] & tileTrivialAccept[2]) < 0)
			{
				tileMask = -1; // Set all bits
			}
			else
			{
				// Tile is partially covered, do per-pixel tests

				tileMask = 0;

				tileMask = (*call_tile_CoverageTest)(quadEdgeValue,
														quadStep_X,
														quadStep_Y,
														quadStep_tileBack);
			}

			tileMask &= scissorMask[scissorIndex];

			// Step z , 1/w to top-left corner of the tile

			temp_packedZ = triangle->packedZ;
			temp_packedZ += temp_packedZ_dx * FLOAT32(j);
			temp_packedZ += temp_packedZ_dy * FLOAT32(k);

			//*********************************************
			// Depth-stencil test
			
			if (depthBuffer != nullptr)
			{

				depthBuffer->DepthStencilTestTile(8,j,k,(void*)&temp_packedZ,(void*)&temp_packedZ_quad_dx,
													(void*)&temp_packedZ_quad_dy,
													tileMask,&depthMask,true);
			}

			// Debug: Override depth test result with coverage mask									
			depthMask = tileMask; 

			if (depthMask == 0)
			{
				goto label_tileSkip;
			}

			tileMask = depthMask;

			if (tileMask == -1)
			{
				//***************************************
				// Tile is completely covered by triangle

				for(Ceng::UINT32 i=0;i<4;i++)
				{
					// Flush any partial chains
					
					if (chainType[i] == PARTIAL_CHAIN)
					{
						GenerateChain(outputs[i%3],chainStart[i],chainEnd[i],chainCoverage[i],
																k+2*i);

						chainStart[i] = -1;
						chainEnd[i] = -1;
						chainType[i] = -1;
					}
									
					if (chainStart[i] == -1)
					{
						chainStart[i] = j;
						chainType[i] = FULL_CHAIN;

						coverageIndex[i] = 0;

						chainCoverage[i][0] = 0; // Signal a full chain
						chainCoverage[i][1] = 0;
						chainCoverage[i][2] = 0;
						chainCoverage[i][3] = 0;
					}

					chainEnd[i] = j+8;

					// Test: end all chains at end of tile	
					/*
					GenerateChain(outputBatch, chainStart[i], chainEnd[i], chainCoverage[i],
						k + 2 * i);

					chainStart[i] = -1;
					chainEnd[i] = -1;
					chainType[i] = -1;
					*/
					
				}
				
			}
			else 
			{
				// ***********************************
				// Partially covered tile

				// Translate coverage mask

				UINT32 line[4];

				line[0] = UINT32(tileMask & 65535);
				line[1] = UINT32( (tileMask >> 16) & 65535);
				line[2] = UINT32( (tileMask >> 32) & 65535);
				line[3] = UINT32( (tileMask >> 48) & 65535);

				// Examine each row of quads within the tile

				for(Ceng::UINT32 i=0;i<4;i++)
				{
				
					INT32 start=0;
					INT32 quad;

					// Check if we can continue a full chain from the
					// previous tile

					if (chainType[i] == FULL_CHAIN)
					{
						for(quad=0;quad<4;quad++)
						{
							if ( ((line[i] >> (4*quad)) & 15) == 15)
							{
								start++;
							}
							else
							{
								break;
							}
						}

						chainEnd[i] = j+2*start;

						if (start == 4)
						{
							continue;
						}
						
							
						// Flush the chain if the entire line isn't fully covered

						GenerateChain(outputs[i%3],chainStart[i],
													chainEnd[i],chainCoverage[i],
													k+2*i);

						chainStart[i] = -1;
						chainEnd[i] = -1;
						chainType[i] = -1;						
					}
						
					// Check for trailing full chain

					INT32 trailingStart=4;
					
					for(quad=3;quad>=start;quad--)
					{
						if ( ((line[i] >> (4*quad)) & 15) == 15)
						{
							trailingStart = quad;
						}
						else
						{
							break;
						}
					}			

					// Flush any preceding chain

					GenerateChain(outputs[i%3],chainStart[i],
									chainEnd[i],chainCoverage[i],
									k+2*i);

					chainStart[i] = -1;
					chainEnd[i] = -1;
					chainType[i] = -1;		
					
					// Find partial chains

					for(quad=start;quad<trailingStart;quad++)
					{
						UINT32 mask = (line[i] >> (4*quad)) & 15;

						if (mask > 0)
						{
							// Found covered quad

							if (chainStart[i] == -1)
							{
								chainType[i] = PARTIAL_CHAIN;
								chainStart[i] = j+2*quad;
										
								chainCoverage[i][0] = 0;
								chainCoverage[i][1] = 0;
								chainCoverage[i][2] = 0;
								chainCoverage[i][3] = 0;

								coverageIndex[i] = 0;
							}

							chainEnd[i] = j+2*quad+2;

							chainCoverage[i][ coverageIndex[i] >> 3] |= mask << (4*(coverageIndex[i] & 7));

							coverageIndex[i]++;

							if (coverageIndex[i] == 32)
							{
								GenerateChain(outputs[i%3],chainStart[i],
												chainEnd[i],chainCoverage[i],
												k+2*i);

								chainStart[i] = -1;
								chainEnd[i] = -1;
								chainType[i] = -1;
							}
						}
						else
						{
							// Found a non-covered quad

							if (chainType[i] == PARTIAL_CHAIN)
							{
								GenerateChain(outputs[i%3],chainStart[i],
														chainEnd[i],chainCoverage[i],
														k+2*i);

								chainStart[i] = -1;
								chainEnd[i] = -1;
								chainType[i] = -1;
							}
						}
					}	

					if (trailingStart < 4)
					{
						// Flush partial chain if present
						if (chainType[i] == PARTIAL_CHAIN)
						{
							GenerateChain(outputs[i%3],chainStart[i],
													chainEnd[i],chainCoverage[i],
													k+2*i);

							chainStart[i] = -1;
							chainEnd[i] = -1;
							chainType[i] = -1;
						}

						// Start a full chain

						chainType[i] = FULL_CHAIN;

						coverageIndex[i] = 0;
						chainCoverage[i][0] = 0;

						chainStart[i] = j + 2*trailingStart;
						chainEnd[i] = j + 8;
					}					

				} // for i

			} // if (tileMask)

			label_tileSkip: // if (tileMask == -1)

			// Step tile coverage test values
		
			tileTrivialAccept[0] += triangle->tile.stepX[0];
			tileTrivialAccept[1] += triangle->tile.stepX[1];
			tileTrivialAccept[2] += triangle->tile.stepX[2];

			tileTrivialReject[0] += triangle->tile.stepX[0];
			tileTrivialReject[1] += triangle->tile.stepX[1];
			tileTrivialReject[2] += triangle->tile.stepX[2];

			// Step quad coverage test values to next
			// tile on the right
	
			quadEdgeValue[0].pixel[0] += triangle->tile.stepX[0];
			quadEdgeValue[0].pixel[1] += triangle->tile.stepX[0];
			quadEdgeValue[0].pixel[2] += triangle->tile.stepX[0];
			quadEdgeValue[0].pixel[3] += triangle->tile.stepX[0];

			quadEdgeValue[1].pixel[0] += triangle->tile.stepX[1];
			quadEdgeValue[1].pixel[1] += triangle->tile.stepX[1];
			quadEdgeValue[1].pixel[2] += triangle->tile.stepX[1];
			quadEdgeValue[1].pixel[3] += triangle->tile.stepX[1];

			quadEdgeValue[2].pixel[0] += triangle->tile.stepX[2];
			quadEdgeValue[2].pixel[1] += triangle->tile.stepX[2];
			quadEdgeValue[2].pixel[2] += triangle->tile.stepX[2];
			quadEdgeValue[2].pixel[3] += triangle->tile.stepX[2];	

		} // for j
			
		// Step tile coverage test values
		
		tileTrivialAccept[0] += triangle->tile.toNextRow[0];
		tileTrivialAccept[1] += triangle->tile.toNextRow[1];
		tileTrivialAccept[2] += triangle->tile.toNextRow[2];

		tileTrivialReject[0] += triangle->tile.toNextRow[0];
		tileTrivialReject[1] += triangle->tile.toNextRow[1];
		tileTrivialReject[2] += triangle->tile.toNextRow[2];
		

		// Step quad coverage test values to next row
		// of tiles
	
		quadEdgeValue[0].pixel[0] += triangle->tile.toNextRow[0];
		quadEdgeValue[0].pixel[1] += triangle->tile.toNextRow[0];
		quadEdgeValue[0].pixel[2] += triangle->tile.toNextRow[0];
		quadEdgeValue[0].pixel[3] += triangle->tile.toNextRow[0];

		quadEdgeValue[1].pixel[0] += triangle->tile.toNextRow[1];
		quadEdgeValue[1].pixel[1] += triangle->tile.toNextRow[1];
		quadEdgeValue[1].pixel[2] += triangle->tile.toNextRow[1];
		quadEdgeValue[1].pixel[3] += triangle->tile.toNextRow[1];

		quadEdgeValue[2].pixel[0] += triangle->tile.toNextRow[2];
		quadEdgeValue[2].pixel[1] += triangle->tile.toNextRow[2];
		quadEdgeValue[2].pixel[2] += triangle->tile.toNextRow[2];
		quadEdgeValue[2].pixel[3] += triangle->tile.toNextRow[2];

		// Generate current chains
		for(Ceng::UINT32 i=0;i<4;i++)
		{
			if (chainType[i] != -1)
			{
				GenerateChain(outputs[i%3],chainStart[i],chainEnd[i],
											chainCoverage[i],k+2*i);
			}

			chainStart[i] = -1;
			chainEnd[i] = -1;
			chainType[i] = -1;
		}		
			
	} // for k


	return 1;
}


const CRESULT CR_Rasterizer::GenerateChain(Task_PixelShader *batch,
										   const Ceng::INT32 startQuadX,const Ceng::INT32 endQuadX,
										   const Ceng::UINT32 *coverageMask,
										   const Ceng::INT32 chainY)
{
	if ( (startQuadX == -1) || (endQuadX == -1)) return CE_OK;

	if (startQuadX == endQuadX) return CE_OK;

	CR_QuadFormat *quadFormat = &batch->rasterizerBatch->renderState->shaderLink->quadFormat;	

	// Write quad header

	batch->quadList.PushBack(CR_QuadHeader());
	
	CR_QuadHeader *headerPtr = &(batch->quadList)[batch->quadCount];

	// Triangle index (used to read correct
	// per-primitive constants)
	headerPtr->triangle = batch->rasterizerBatch->triangle.get();

	headerPtr->chainLength = (endQuadX-startQuadX) >> 1;

	headerPtr->screenX = startQuadX;
	headerPtr->screenY = chainY;

	// Entrire chain is fully covered 
	// NOTE: doesn't make sense to send coverage mask of 0 to shader, so use it to signal this
	headerPtr->coverageMask[0] = coverageMask[0]; 
	headerPtr->coverageMask[1] = coverageMask[1];
	headerPtr->coverageMask[2] = coverageMask[2];
	headerPtr->coverageMask[3] = coverageMask[3];

	// Increase quad count
	++batch->quadCount;

	return CE_OK;
}

/*
Ceng::INT32 CR_Rasterizer::RasterizeTriangle_ChainWithinTile(CR_TriangleData *triangle,
														CR_NewTargetData *depthBuffer,
														Ceng::Rectangle *scissorRect)
{	
	//******************************************************
	// Create local copies of variables

	_declspec(align(16)) INT32 tileTrivialAccept[4];
	_declspec(align(16)) INT32 tileTrivialReject[4];
	
	memcpy(tileTrivialAccept,triangle->tile.trivialAccept,4*sizeof(INT32));
	memcpy(tileTrivialReject,triangle->tile.trivialReject,4*sizeof(INT32));

	_declspec(align(16)) CR_QuadEdges quadEdgeValue[3];

	memcpy(quadEdgeValue,triangle->quadEdgeValue,3*sizeof(CR_QuadEdges));

	_declspec(align(16)) CR_QuadEdges quadStep_X[3];

	quadStep_X[0].pixel[0] = triangle->quadEdgeStepX[0];
	quadStep_X[0].pixel[1] = triangle->quadEdgeStepX[0];
	quadStep_X[0].pixel[2] = triangle->quadEdgeStepX[0];
	quadStep_X[0].pixel[3] = triangle->quadEdgeStepX[0];

	quadStep_X[1].pixel[0] = triangle->quadEdgeStepX[1];
	quadStep_X[1].pixel[1] = triangle->quadEdgeStepX[1];
	quadStep_X[1].pixel[2] = triangle->quadEdgeStepX[1];
	quadStep_X[1].pixel[3] = triangle->quadEdgeStepX[1];

	quadStep_X[2].pixel[0] = triangle->quadEdgeStepX[2];
	quadStep_X[2].pixel[1] = triangle->quadEdgeStepX[2];
	quadStep_X[2].pixel[2] = triangle->quadEdgeStepX[2];
	quadStep_X[2].pixel[3] = triangle->quadEdgeStepX[2];

	_declspec(align(16)) CR_QuadEdges quadStep_Y[3];

	quadStep_Y[0].pixel[0] = triangle->quadEdgeStepY[0];
	quadStep_Y[0].pixel[1] = triangle->quadEdgeStepY[0];
	quadStep_Y[0].pixel[2] = triangle->quadEdgeStepY[0];
	quadStep_Y[0].pixel[3] = triangle->quadEdgeStepY[0];

	quadStep_Y[1].pixel[0] = triangle->quadEdgeStepY[1];
	quadStep_Y[1].pixel[1] = triangle->quadEdgeStepY[1];
	quadStep_Y[1].pixel[2] = triangle->quadEdgeStepY[1];
	quadStep_Y[1].pixel[3] = triangle->quadEdgeStepY[1];

	quadStep_Y[2].pixel[0] = triangle->quadEdgeStepY[2];
	quadStep_Y[2].pixel[1] = triangle->quadEdgeStepY[2];
	quadStep_Y[2].pixel[2] = triangle->quadEdgeStepY[2];
	quadStep_Y[2].pixel[3] = triangle->quadEdgeStepY[2];

	_declspec(align(16)) CR_QuadEdges quadStep_tileBack[3];

	quadStep_tileBack[0].pixel[0] = triangle->quadStepTileBack[0];
	quadStep_tileBack[0].pixel[1] = triangle->quadStepTileBack[0];
	quadStep_tileBack[0].pixel[2] = triangle->quadStepTileBack[0];
	quadStep_tileBack[0].pixel[3] = triangle->quadStepTileBack[0];

	quadStep_tileBack[1].pixel[0] = triangle->quadStepTileBack[1];
	quadStep_tileBack[1].pixel[1] = triangle->quadStepTileBack[1];
	quadStep_tileBack[1].pixel[2] = triangle->quadStepTileBack[1];
	quadStep_tileBack[1].pixel[3] = triangle->quadStepTileBack[1];

	quadStep_tileBack[2].pixel[0] = triangle->quadStepTileBack[2];
	quadStep_tileBack[2].pixel[1] = triangle->quadStepTileBack[2];
	quadStep_tileBack[2].pixel[2] = triangle->quadStepTileBack[2];
	quadStep_tileBack[2].pixel[3] = triangle->quadStepTileBack[2];

	INT32 k,j;

	_declspec(align(16)) VectorF4 temp_packedZ,temp_packedW;

	_declspec(align(16)) VectorF4 temp_packedZ_dx,
									temp_packedZ_dy,
									temp_packedZ_quad_dx,
									temp_packedZ_quad_dy;

	temp_packedZ_dx = triangle->packedZ_dx;
	temp_packedZ_dy = triangle->packedZ_dy;

	temp_packedZ_quad_dx = temp_packedZ_dx*2;
	temp_packedZ_quad_dy = triangle->packedZ_quad_dy;

	UINT64 depthMask;
	UINT64 tileMask = 0;

	const INT32 FULL_CHAIN = 2;
	const INT32 PARTIAL_CHAIN = 1;

	UINT32 chainCoverage[4][4];

	UINT32 i;
	
	// Test tiles within the bounding box for triangle overlap

	for(k=triangle->yMin;k<triangle->yMax;k+=8)
	{
		for(j=triangle->xMin;j<triangle->xMax;j+=8)
		{
			// Trivial reject test

			// NOTE: Means that the triangle isn't over this tile,
			//       so doesn't include depth-stencil test result

			
			if ((tileTrivialReject[0] & tileTrivialReject[1] & tileTrivialReject[2]) >= 0)
			{
				// if( any(tileEdgeValue[1][i]) >= 0)
				goto label_tileSkip;
			}
			
			
			// Pixel coverage test

			if ((tileTrivialAccept[0] & tileTrivialAccept[1] & tileTrivialAccept[2]) < 0)
			{
				// if( all(tileEdgeValue[0][i]) < 0)

				tileMask = -1; // Set all bits
			}
			else
			{
				// Tile is partially covered, do per-pixel tests

				tileMask = 0;

				tileMask = (*call_tile_CoverageTest)(quadEdgeValue,
														quadStep_X,
														quadStep_Y,
														quadStep_tileBack);
			}

			// Step z , 1/w to top-left corner of the tile

			temp_packedZ = triangle->packedZ;
			temp_packedZ += temp_packedZ_dx * FLOAT32(j);
			temp_packedZ += temp_packedZ_dy * FLOAT32(k);

			//*********************************************
			// Depth-stencil test
			
			if (depthBuffer != NULL)
			{
				depthBuffer->DepthStencilTestTile(8,j,k,(void*)&temp_packedZ,(void*)&temp_packedZ_quad_dx,
													(void*)&temp_packedZ_quad_dy,
													tileMask,&depthMask,true);
													
				// Debug: Override depth test result with coverage mask									
				depthMask = tileMask; 
			}
			

			depthMask = tileMask;

			if (depthMask == 0)
			{
				goto label_tileSkip;
			}

			tileMask = depthMask;

			
			if (tileMask == -1)
			{
				//***************************************
				// Tile is completely covered by triangle

				for(i=0;i<4;i++)
				{
					chainCoverage[i][0] = 0;

					quadGenerator->GenerateChain(triangle,j,j+8,chainCoverage[i],
															k+2*i,quadCounter,shaderLink);
				}				
			}
			else 
			{	
				// ***********************************
				// Partially covered tile

				// Translate coverage mask

				UINT32 line[4];

				line[0] = UINT32(tileMask & 65535);
				line[1] = UINT32( (tileMask >> 16) & 65535);
				line[2] = UINT32( (tileMask >> 32) & 65535);
				line[3] = UINT32( (tileMask >> 48) & 65535);

				UINT32 n;

				/*
				for(i=0;i<4;i++)
				{
					for(n=0;n<4;n++)
					{
						UINT32 mask = (line[i] >> (4*n)) & 15;

						if (mask)
						{
							chainCoverage[i][0] = mask;

							quadGenerator->GenerateChain(triangle,j+2*n,j+2*n+2,chainCoverage[i],
															k+2*i,quadCounter,shaderLink);
						}
					}
				}
				*/
/*					
				for(i=0;i<4;i++)
				{
					if (line[i])
					{
						chainCoverage[i][0] = line[i];
					
						quadGenerator->GenerateChain(triangle,j,j+8,chainCoverage[i],
															k+2*i,quadCounter,shaderLink);
					}

				}
				
				

			} // if (tileMask)

			label_tileSkip: // if (tileMask == -1)

			// Step tile coverage test values
		
			tileTrivialAccept[0] += triangle->tile.stepX[0];
			tileTrivialAccept[1] += triangle->tile.stepX[1];
			tileTrivialAccept[2] += triangle->tile.stepX[2];

			tileTrivialReject[0] += triangle->tile.stepX[0];
			tileTrivialReject[1] += triangle->tile.stepX[1];
			tileTrivialReject[2] += triangle->tile.stepX[2];

			// Step quad coverage test values to next
			// tile on the right
	
			quadEdgeValue[0].pixel[0] += triangle->tile.stepX[0];
			quadEdgeValue[0].pixel[1] += triangle->tile.stepX[0];
			quadEdgeValue[0].pixel[2] += triangle->tile.stepX[0];
			quadEdgeValue[0].pixel[3] += triangle->tile.stepX[0];

			quadEdgeValue[1].pixel[0] += triangle->tile.stepX[1];
			quadEdgeValue[1].pixel[1] += triangle->tile.stepX[1];
			quadEdgeValue[1].pixel[2] += triangle->tile.stepX[1];
			quadEdgeValue[1].pixel[3] += triangle->tile.stepX[1];

			quadEdgeValue[2].pixel[0] += triangle->tile.stepX[2];
			quadEdgeValue[2].pixel[1] += triangle->tile.stepX[2];
			quadEdgeValue[2].pixel[2] += triangle->tile.stepX[2];
			quadEdgeValue[2].pixel[3] += triangle->tile.stepX[2];	

		} // for j
			
		// Step tile coverage test values
		
		tileTrivialAccept[0] += triangle->tile.toNextRow[0];
		tileTrivialAccept[1] += triangle->tile.toNextRow[1];
		tileTrivialAccept[2] += triangle->tile.toNextRow[2];

		tileTrivialReject[0] += triangle->tile.toNextRow[0];
		tileTrivialReject[1] += triangle->tile.toNextRow[1];
		tileTrivialReject[2] += triangle->tile.toNextRow[2];
		

		// Step quad coverage test values to next row
		// of tiles
	
		quadEdgeValue[0].pixel[0] += triangle->tile.toNextRow[0];
		quadEdgeValue[0].pixel[1] += triangle->tile.toNextRow[0];
		quadEdgeValue[0].pixel[2] += triangle->tile.toNextRow[0];
		quadEdgeValue[0].pixel[3] += triangle->tile.toNextRow[0];

		quadEdgeValue[1].pixel[0] += triangle->tile.toNextRow[1];
		quadEdgeValue[1].pixel[1] += triangle->tile.toNextRow[1];
		quadEdgeValue[1].pixel[2] += triangle->tile.toNextRow[1];
		quadEdgeValue[1].pixel[3] += triangle->tile.toNextRow[1];

		quadEdgeValue[2].pixel[0] += triangle->tile.toNextRow[2];
		quadEdgeValue[2].pixel[1] += triangle->tile.toNextRow[2];
		quadEdgeValue[2].pixel[2] += triangle->tile.toNextRow[2];
		quadEdgeValue[2].pixel[3] += triangle->tile.toNextRow[2];
			
	} // for k

	return 1;
}
*/