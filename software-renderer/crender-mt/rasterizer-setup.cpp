/*****************************************************************************
*
* raster-setup.cpp
*
* By Jari Korkala 2/2012
*
* Portable version of the triangle setup
*
*****************************************************************************/

#include "rasterizer-cr.h"

#include "locking-queue.h"

#include "task-rasterizer.h"

#include "shader-link.h"

using namespace Ceng;

CR_Rasterizer::CR_Rasterizer()
{
	//call_tile_CoverageTest = &TILE_COVERAGE_TEST;
	call_tile_CoverageTest = &TILE_COVERAGE_TEST_X86_SSE2;
}

CR_Rasterizer::~CR_Rasterizer()
{
}

const CRESULT CR_Rasterizer::TriangleSetup(std::shared_ptr<TriangleBatch> &batch,
										   LockingStage &outputStage)
{
	CRESULT cresult;
	Ceng::UINT32 k;

	BatchVector outputBatch	= BatchVector(outputStage.Buckets());
	Ceng::UINT32 buckets=0;

	for(k=0;k<batch->primitiveList.size();k++)
	{
		for(Ceng::UINT32 j=0;j<outputBatch.size();j++)
		{
			outputBatch[j] = std::shared_ptr<RasterizerBatch>(new RasterizerBatch(batch->apiCallId,batch->renderState));
		}

		switch(batch->primitiveList[k].primitiveType)
		{
		case PRIMITIVE_TYPE::TRIANGLE_LIST:

			cresult = TriangleSetup(batch->primitiveList[k],
									batch->renderState,outputBatch,buckets);

			break;
		}

		for(Ceng::UINT32 j=0;j<buckets;j++)
		{
			if (outputBatch[j] != nullptr)
			{
				outputStage.AddTask(j,std::shared_ptr<LockingTask>(new Task_Rasterizer(outputBatch[j])));
			}
		}
	}

	return CE_OK;
}

void CR_Rasterizer::TriangleSetup_DiscardBatch(BatchVector &outputBatch)
{
	for(Ceng::UINT32 k=0;k<outputBatch.size();k++)
	{
		outputBatch[k] = nullptr;
	}
}

const CRESULT CR_Rasterizer::TriangleSetup(CR_PrimitiveData &primitive,
										   std::shared_ptr<RenderState> &renderState,
										   BatchVector &outputBatch,
										   Ceng::UINT32 &buckets)
{
	const INT32 TILE_SHIFT = 3;

	/*
	 * Size of largest rasterization tile.
	 */
	const INT32 TILE_SIZE = 1 << TILE_SHIFT;	

	const INT32 TILE_BIT_MASK = TILE_SIZE-1;

	
	/*
	 * Number of fractional bits in vertex screen space coordinates.
	 */
	const INT32 SUBPIXEL_SHIFT = 8;

	const INT32 SUBPIXEL_ONE = 1 << SUBPIXEL_SHIFT;
	
	const FLOAT32 SUBPIXEL_SCALE = FLOAT32(SUBPIXEL_ONE);

	const INT32 SUBPIXEL_FRACTION_MASK = SUBPIXEL_ONE-1;

	const INT32 MIN_STEP_SHIFT = 1; // Default = 1 , must be >= 1

	/*
	 * Minimum number of pixels that will be stepped during rasterization.
	 * All other steps must be an integer multiple of this value.
	 */
	const INT32 MIN_PIXEL_STEP = 1 << MIN_STEP_SHIFT;

	const INT32 MAX_SCREEN_POW2 = ( (32-2-SUBPIXEL_SHIFT+MIN_STEP_SHIFT) / 2);

	const INT32 MAX_SCREEN_WIDTH = 1 << MAX_SCREEN_POW2;

	UINT8* vertexOrder[3];

	ViewportState *viewportState = &renderState->viewportState;

	const INT32 viewWidth = viewportState->rasterizerPort.right - viewportState->rasterizerPort.left;
	const INT32 viewHeight = viewportState->rasterizerPort.bottom - viewportState->rasterizerPort.top;

	_declspec(align(16)) VectorF4 position[3];

	RasterizerState *state = &renderState->rasterizerState;

	if (state->frontClockwise)
	{
		if (state->cullingMode == Ceng::CULLING_MODE::FRONT)
		{
			// Visible backfaces have clockwise ordering

			vertexOrder[0] = primitive.fragmentPtr[0];
			vertexOrder[1] = primitive.fragmentPtr[1];
			vertexOrder[2] = primitive.fragmentPtr[2];
		}
		else
		{
			// Visible faces have counter-clockwise ordering (default)
			
			vertexOrder[0] = primitive.fragmentPtr[0];
			vertexOrder[1] = primitive.fragmentPtr[2];
			vertexOrder[2] = primitive.fragmentPtr[1];			
		}
		
	}
	else
	{
		// Front-faces have counter-clockwise vertex ordering

		if (state->cullingMode == Ceng::CULLING_MODE::FRONT)
		{
			// Visible backfaces have clockwise ordering

			vertexOrder[0] = primitive.fragmentPtr[0];
			vertexOrder[1] = primitive.fragmentPtr[2];
			vertexOrder[2] = primitive.fragmentPtr[1];
		}
		else
		{
			// Visible faces have counter-clockwise ordering (default)
			
			vertexOrder[0] = primitive.fragmentPtr[0];
			vertexOrder[1] = primitive.fragmentPtr[1];
			vertexOrder[2] = primitive.fragmentPtr[2];
		}
	}

	position[0] = *(VectorF4*)&vertexOrder[0][0];
	position[1] = *(VectorF4*)&vertexOrder[1][0];
	position[2] = *(VectorF4*)&vertexOrder[2][0];
	
	//******************************************
	// Store camera space x and y
	FLOAT32 camSpaceX[3],camSpaceY[3];

	camSpaceX[0] = position[0].x;
	camSpaceY[0] = position[0].y;

	camSpaceX[1] = position[1].x;
	camSpaceY[1] = position[1].y;
	
	camSpaceX[2] = position[2].x;
	camSpaceY[2] = position[2].y;

	// Divide each component by 'w', and store
	// 1/w to screen space w-component

	position[0].EuclidianRestore();
	position[1].EuclidianRestore();
	position[2].EuclidianRestore();

	// Adjust to output resolution
	position[0] *= viewportState->screenDilation;
	position[1] *= viewportState->screenDilation;
	position[2] *= viewportState->screenDilation;

	position[0] += viewportState->screenCenter;
	position[1] += viewportState->screenCenter;
	position[2] += viewportState->screenCenter;

	//*****************************************
	// Snap vertex (x,y) pairs to grid of
	// 16*16 values per pixel

	INT32 vx[3],vy[3];
	
	// TODO: Test flooring before float->int for sharper edges
	//
	// NOTE: Also rounds positive and negative values in the same
	//       direction

	vx[0] = floor(position[0].x * SUBPIXEL_SCALE);
	vy[0] = floor(position[0].y * SUBPIXEL_SCALE);
	
	vx[1] = floor(position[1].x * SUBPIXEL_SCALE);
	vy[1] = floor(position[1].y * SUBPIXEL_SCALE);
	
	vx[2] = floor(position[2].x * SUBPIXEL_SCALE);
	vy[2] = floor(position[2].y * SUBPIXEL_SCALE);

	//***************************************************
	// Front-/backface culling

	FLOAT32 gradientDiv;

	// NOTE: 
	// Convert truncated screen coordinates back to float so
	// that variable stepping is only calculated to the precision
	// of 1/SUBPIXEL_SCALE.

	gradientDiv = (FLOAT32(vx[1]-vx[0])*FLOAT32(vy[2]-vy[1]) - FLOAT32(vx[2]-vx[1])*FLOAT32(vy[1]-vy[0]));

	if (state->cullingMode == Ceng::CULLING_MODE::BACK)
	{
		if (gradientDiv > FLOAT32(-1.0f))
		{
			TriangleSetup_DiscardBatch(outputBatch);
			return CE_OK;
		}
		
	}
	else if (state->cullingMode == Ceng::CULLING_MODE::FRONT)
	{
		if (gradientDiv > FLOAT32(-1.0f))
		{
			TriangleSetup_DiscardBatch(outputBatch);
			return CE_OK;
		}
	}
	else
	{
		// No culling

		if (gradientDiv > FLOAT32(-1.0f))
		{
			if (gradientDiv == FLOAT32(0.0))
			{
				TriangleSetup_DiscardBatch(outputBatch);
				return CE_OK;
			}

			// Swap vertex order to display the backside
			
			std::swap(vertexOrder[1],vertexOrder[2]);

			//vertexOrder[0] = vertexOr;
			//vertexOrder[1] = vertexOr;
			//vertexOrder[2] = vertex1;

			VectorF4 v4swap;

			v4swap = position[1];
			position[1] = position[2];
			position[2] = v4swap;

			INT32 swap32;

			swap32 = vx[1];
			vx[1] = vx[2];
			vx[2] = swap32;

			swap32 = vy[1];
			vy[1] = vy[2];
			vy[2] = swap32;

			gradientDiv = (FLOAT32(vx[1]-vx[0])*FLOAT32(vy[2]-vy[1]) 
					- FLOAT32(vx[2]-vx[1])*FLOAT32(vy[1]-vy[0]));		

		}
	}

	gradientDiv = SUBPIXEL_SCALE / gradientDiv;

	std::shared_ptr<CR_TriangleData> triangle = std::make_shared<CR_TriangleData>();

	triangle->gradientDiv = gradientDiv;

	for(Ceng::UINT32 k=0; k < outputBatch.size(); k++)
	{
		outputBatch[k]->triangle = triangle;
	}

	//*****************************************
	// Determine bounding box
		
	// X extrema
	INT32 xMin = vx[0];
	INT32 xMax = vx[0];

	if (vx[1] < xMin) xMin = vx[1];
	if (vx[2] < xMin) xMin = vx[2];

	if (vx[1] > xMax) xMax = vx[1];
	if (vx[2] > xMax) xMax = vx[2];

	// Y extrema
	INT32 yMin = vy[0];
	INT32 yMax = vy[0];

	if (vy[1] < yMin) yMin = vy[1];
	if (vy[2] < yMin) yMin = vy[2];

	if (vy[1] > yMax) yMax = vy[1];
	if (vy[2] > yMax) yMax = vy[2];
	
	xMin = xMin >> SUBPIXEL_SHIFT; // Floor
	xMax = (xMax + SUBPIXEL_FRACTION_MASK) >> SUBPIXEL_SHIFT; // Ceil
	
	yMin = yMin >> SUBPIXEL_SHIFT;
	yMax = (yMax + SUBPIXEL_FRACTION_MASK) >> SUBPIXEL_SHIFT;

	//***********************************************************************
	// Calculate tile-padded bounding box in final coordinate system

	INT32 halfWidth = ceil(FLOAT32(viewWidth) / FLOAT32(2.0f));
	INT32 halfHeight = ceil(FLOAT32(viewHeight) / FLOAT32(2.0f)) ;

	xMin += halfWidth;
	xMax += halfWidth;

	yMin += halfHeight;
	yMax += halfHeight;

	//**********************************************************************
	// Scissor test

	Rectangle &scissorRect = renderState->activeRect;

	// Trivial reject

	if ( ( (xMax-scissorRect.left) | (yMax-scissorRect.top) | (scissorRect.right-xMin) | (scissorRect.bottom-yMin) ) < 0)
	{
		// if (xMax < left || yMax < top || xMin > right || yMin > bottom)

		TriangleSetup_DiscardBatch(outputBatch);
		return CE_OK;
	}

	if (xMin < scissorRect.left) xMin = scissorRect.left;
	if (yMin < scissorRect.top) yMin = scissorRect.top;

	if (xMax > scissorRect.right) xMax = scissorRect.right;
	if (yMax > scissorRect.bottom) yMax = scissorRect.bottom;

	// Floor xMin,yMin down to nearest number divisible by TILE_SIZE

	xMin = xMin & (~TILE_BIT_MASK);
	yMin = yMin & (~TILE_BIT_MASK);

	// Ceil xMax,yMax up to nearest number divisible by 8

	xMax = (xMax + TILE_BIT_MASK) & (~TILE_BIT_MASK);
	yMax = (yMax + TILE_BIT_MASK) & (~TILE_BIT_MASK);

	/*
	triangle->xMin = xMin;
	triangle->yMin = yMin;
	triangle->xMax = xMax;
	triangle->yMax = yMax;
	*/

	// Discard subsections by bounding box

	//std::atomic<Ceng::UINT32> *counter = new std::atomic<Ceng::UINT32>(0);

	CR_NewTargetData *target = renderState->renderTargets[2].get();

	buckets = target->bucketAreas.size();

	for(Ceng::UINT32 k=0; k < buckets; k++)
	{
		target = renderState->renderTargets[2].get();

		Rectangle *rect = &target->bucketAreas[k];

		if ( ( (xMax-rect->left) | (yMax-rect->top) | (rect->right-xMin) | (rect->bottom-yMin) ) < 0)
		{
			outputBatch[k] = nullptr;
			continue;
		}

		outputBatch[k]->xMin = xMin;
		if (xMin < rect->left) outputBatch[k]->xMin = rect->left;

		outputBatch[k]->yMin = yMin;
		if (yMin < rect->top) outputBatch[k]->yMin = rect->top;

		outputBatch[k]->xMax = xMax;
		if (xMax > rect->right) outputBatch[k]->xMax = rect->right;

		outputBatch[k]->yMax = yMax;
		if (yMax > rect->bottom) outputBatch[k]->yMax = rect->bottom;

		outputBatch[k]->bucketId = k;
	}

	//***********************************************************************

	// Convert local extrema back to input coordinates so that
	// calculations are correct

	xMin -= halfWidth;
	xMax -= halfWidth;

	yMin -= halfHeight;
	yMax -= halfHeight;

	//**********************************************************************

	/*
	 * Half-space rasterization algorithm uses the
	 * edge equation
	 *
	 * e = (x1-x0)(y-y0) - (y1-y0)(x-x0)
	 *
	 * to determine whether a pixel center (integer
	 * screen coordinates) is inside the triangle.
	 *
	 * Each edge has its own edge equation. All
	 * three edge equations are negative when a
	 * pixel is inside the triangle.
	 *
	 * NOTE: Requires counter-clockwise vertex order
	 */
	
	INT32 deltaX[3],deltaY[3];
	
	deltaX[0] = vx[1] - vx[0];
	deltaX[1] = vx[2] - vx[1];
	deltaX[2] = vx[0] - vx[2];
	
	deltaY[0] = vy[1] - vy[0];
	deltaY[1] = vy[2] - vy[1];
	deltaY[2] = vy[0] - vy[2];

	// Skip special cases in which an edge has zero length,
	// which imply that the triangle has zero area

	if ( (deltaX[0] | deltaY[0]) == 0)
	{
		// deltaX[0] == 0 && deltaY[0] == 0
		TriangleSetup_DiscardBatch(outputBatch);
		return CE_OK;
	}

	if ( (deltaX[1] | deltaY[1]) == 0)
	{
		TriangleSetup_DiscardBatch(outputBatch);
		return CE_OK;
	}

	if ( (deltaX[2] | deltaY[2]) == 0)
	{
		TriangleSetup_DiscardBatch(outputBatch);
		return CE_OK;
	}



	INT64 edgeFunction[3];

	INT64 pixelStepX[3],pixelStepY[3];

	UINT32 k;

	UINT32 totalShift = SUBPIXEL_SHIFT + MIN_STEP_SHIFT;

	INT32 offsetX = INT32( (0.5f - viewportState->pixelCenterOffsetX) * SUBPIXEL_SCALE);
	INT32 offsetY = INT32( (0.5f - viewportState->pixelCenterOffsetY) * SUBPIXEL_SCALE);

	CR_QuadEdges quadEdgeValue[3];

	for(k=0;k<3;k++)
	{
		INT64 cx,cy;

		// Evaluate edge functions at (xMin,yMin)

		// NOTE: Must use local xMin,yMin here

		cx =  INT64( (xMin << SUBPIXEL_SHIFT) + offsetX - vx[k] ) * deltaY[k];
		cy =  INT64( (yMin << SUBPIXEL_SHIFT) + offsetY - vy[k] ) * deltaX[k];

		// Inside is negative

		edgeFunction[k] = cy-cx;

		// NOTE: These are implicitly multiplied by MIN_PIXEL_STEP.

		pixelStepX[k] = (-deltaY[k]) << SUBPIXEL_SHIFT;
		pixelStepY[k] = (deltaX[k]) << SUBPIXEL_SHIFT;
		
		triangle->minPixelStepX[k] = -deltaY[k];
		triangle->minPixelStepY[k] = deltaX[k];

		// Top-left fill convention

		// NOTE: This works because vertices are given in counter-clockwise
		//       order

		if (deltaY[k] > 0 || (deltaY[k] == 0 && deltaX[k] < 0))
		{
			// deltaY > 0 : Edge goes downward -> must be left edge

			// deltaY = 0 , deltaX < 0 : Edge is horizontal,goes left -> must be top edge
		
			// Shift edge function value so that pixel centers exactly on the edge
			// are considered to be inside the triangle
			edgeFunction[k]--;

			// TODO: this affects variable stepping
		}
	
		// Edge values for per-quad coverage test

		quadEdgeValue[k].pixel[0] = INT32(edgeFunction[k] >> totalShift);
		quadEdgeValue[k].pixel[1] = INT32( (edgeFunction[k] + pixelStepX[k]) >> totalShift);
		quadEdgeValue[k].pixel[2] = INT32( (edgeFunction[k] + pixelStepY[k]) >> totalShift);

		quadEdgeValue[k].pixel[3] = INT32( (edgeFunction[k] + pixelStepX[k] + pixelStepY[k]) >> totalShift);

		triangle->quadEdgeStepX[k] = triangle->minPixelStepX[k];
		
		triangle->quadEdgeStepY[k] = 
			triangle->minPixelStepY[k] - ((TILE_SIZE-2)/MIN_PIXEL_STEP) * triangle->minPixelStepX[k];

		triangle->quadStepTileBack[k] = -(TILE_SIZE / MIN_PIXEL_STEP) * triangle->minPixelStepY[k];
	}

	// For each edge, find the two corners of the tile
	// that are closest and furthest away from the edge.
	// This allows full and partial tile coverage to be tested
	// with just one sample per edge.

	INT32 trivialAccept[4],trivialReject[4];
	
	for(k=0;k<3;k++)
	{
		INT64 tempAccept = edgeFunction[k];
		
		INT64 tempReject = edgeFunction[k] + (TILE_SIZE-1)*(pixelStepX[k] + pixelStepY[k]);

		if (deltaX[k] >= 0) 
		{
			tempAccept += (TILE_SIZE-1)*pixelStepY[k];
			tempReject -= (TILE_SIZE-1)*pixelStepY[k];
		}

		if (deltaY[k] <= 0) 
		{
			tempAccept += (TILE_SIZE-1)*pixelStepX[k];
			tempReject -= (TILE_SIZE-1)*pixelStepX[k];
		}

		trivialAccept[k] = INT32(tempAccept >> totalShift);

		trivialReject[k] = INT32(tempReject >> totalShift);

		triangle->tile.stepX[k] = (TILE_SIZE / MIN_PIXEL_STEP) * triangle->minPixelStepX[k];

		triangle->tile.toNextRow[k] = (TILE_SIZE / MIN_PIXEL_STEP) * triangle->minPixelStepY[k] 
									- ((xMax-xMin) / MIN_PIXEL_STEP) * triangle->minPixelStepX[k];
	}

	trivialAccept[3] = 0;
	trivialReject[3] = 0;
	triangle->tile.stepX[3] = 0;
	triangle->tile.toNextRow[3] = 0;

	// Calculate values per subdivision

	for(k=0; k < outputBatch.size(); k++)
	{
		if (outputBatch[k] != nullptr)
		{
			INT32 destXmin = outputBatch[k]->xMin - halfWidth;
			INT32 destYmin = outputBatch[k]->yMin - halfHeight;

			INT32 dx = destXmin - xMin;
			INT32 dy = destYmin - yMin;

			for(Ceng::UINT32 j=0;j<3;j++)
			{
				for(Ceng::UINT32 i=0;i<4;i++)
				{
					outputBatch[k]->quadEdgeValue[j].pixel[i] =
						quadEdgeValue[j].pixel[i] +
						(dx >> 1)*triangle->minPixelStepX[j] + (dy >> 1) *triangle->minPixelStepY[j];
				}

				outputBatch[k]->tileValues.trivialAccept[j] =
					trivialAccept[j] +
					(dx >> TILE_SHIFT)*(TILE_SIZE / MIN_PIXEL_STEP)*triangle->minPixelStepX[j] + 
					(dy >> TILE_SHIFT)*(TILE_SIZE / MIN_PIXEL_STEP)*triangle->minPixelStepY[j];

				outputBatch[k]->tileValues.trivialReject[j] =
					trivialReject[j] +
					(dx >> TILE_SHIFT)*(TILE_SIZE / MIN_PIXEL_STEP)*triangle->minPixelStepX[j] + 
					(dy >> TILE_SHIFT)*(TILE_SIZE / MIN_PIXEL_STEP)*triangle->minPixelStepY[j];
			}

			outputBatch[k]->tileValues.trivialAccept[3] = 0;
			outputBatch[k]->tileValues.trivialReject[3] = 0;
		}
	}

	//***********************************************
	// Calculate fragment variable steps (constant)

	FLOAT32 paramSub21,paramSub10;

	FLOAT32 vdy21 = FLOAT32(vy[2] - vy[1]);
	FLOAT32 vdy10 = FLOAT32(vy[1] - vy[0]);

	FLOAT32 vdx21 = FLOAT32(vx[2] - vx[1]);
	FLOAT32 vdx10 = FLOAT32(vx[1] - vx[0]);
	
	// Step to screen coordinate (viewport.left,viewport.top) + (pixelCenterOffsetX,pixelCenterOffsetY)

	FLOAT32 initialStepX = FLOAT32(viewportState->rasterizerPort.left) + 
								(0.5f - viewportState->pixelCenterOffsetX) - 
								FLOAT32(1.0f / SUBPIXEL_SCALE) * FLOAT32(vx[0]);

	FLOAT32 initialStepY = FLOAT32(viewportState->rasterizerPort.top) + 
								(0.5f - viewportState->pixelCenterOffsetY) - 
								FLOAT32(1.0f / SUBPIXEL_SCALE) * FLOAT32(vy[0]);
	
	FLOAT32 numerator;

	FLOAT32 float_dx,float_dy,float_start;

	//***************************************************
	// Pseudo-depth (depth buffer value) interpolant

	paramSub21 = position[2].z - position[1].z;
	paramSub10 = position[1].z - position[0].z;

	numerator = (vdy10 * paramSub21) - (vdy21 * paramSub10);
	float_dx = -numerator * gradientDiv;

	numerator = (vdx10 * paramSub21) - (vdx21 * paramSub10);
	float_dy = numerator * gradientDiv;

	// Pseudo-depth value at (xMin,yMin)
	float_start = position[0].z + initialStepX * float_dx +	initialStepY * float_dy;

	triangle->packedZ.x = float_start;
	triangle->packedZ.y = float_start + float_dx;
	triangle->packedZ.z = float_start + float_dy;
	triangle->packedZ.w = float_start + float_dx + float_dy;

	triangle->packedZ_dx.x = float_dx;
	triangle->packedZ_dx.y = float_dx;
	triangle->packedZ_dx.z = float_dx;
	triangle->packedZ_dx.w = float_dx;

	triangle->packedZ_quad_dx.x = 2*float_dx;
	triangle->packedZ_quad_dx.y = 2*float_dx;
	triangle->packedZ_quad_dx.z = 2*float_dx;
	triangle->packedZ_quad_dx.w = 2*float_dx;

	triangle->packedZ_dy.x = float_dy;
	triangle->packedZ_dy.y = float_dy;
	triangle->packedZ_dy.z = float_dy;
	triangle->packedZ_dy.w = float_dy;

	triangle->packedZ_quad_dy.x = -(TILE_SIZE-2)*float_dx + 2*float_dy;
	triangle->packedZ_quad_dy.y = -(TILE_SIZE-2)*float_dx + 2*float_dy;
	triangle->packedZ_quad_dy.z = -(TILE_SIZE-2)*float_dx + 2*float_dy;
	triangle->packedZ_quad_dy.w = -(TILE_SIZE-2)*float_dx + 2*float_dy;

	//***************************************************
	// Perspective correction term (1/w) interpolant

	// TODO: Check from pixel shader whether this is needed

	paramSub21 = position[2].w - position[1].w;
	paramSub10 = position[1].w - position[0].w;

	numerator = (vdy10 * paramSub21) - (vdy21 * paramSub10);
	float_dx = -numerator * gradientDiv;

	numerator = (vdx10 * paramSub21) - (vdx21 * paramSub10);
	float_dy = numerator * gradientDiv;

	float_start = position[0].w + initialStepX * float_dx + initialStepY * float_dy;

	triangle->packedW.x = float_start;
	triangle->packedW.y = float_start + float_dx;
	triangle->packedW.z = float_start + float_dy;
	triangle->packedW.w = float_start + float_dx + float_dy;

	triangle->packedW_dx.x = float_dx;
	triangle->packedW_dx.y = float_dx;
	triangle->packedW_dx.z = float_dx;
	triangle->packedW_dx.w = float_dx;

	triangle->packedW_quad_dx.x = 2*float_dx;
	triangle->packedW_quad_dx.y = 2*float_dx;
	triangle->packedW_quad_dx.z = 2*float_dx;
	triangle->packedW_quad_dx.w = 2*float_dx;

	triangle->packedW_dy.x = float_dy;
	triangle->packedW_dy.y = float_dy;
	triangle->packedW_dy.z = float_dy;
	triangle->packedW_dy.w = float_dy;

	triangle->packedW_quad_dy.x = -(TILE_SIZE-2)*float_dx + 2*float_dy;
	triangle->packedW_quad_dy.y = -(TILE_SIZE-2)*float_dx + 2*float_dy;
	triangle->packedW_quad_dy.z = -(TILE_SIZE-2)*float_dx + 2*float_dy;
	triangle->packedW_quad_dy.w = -(TILE_SIZE-2)*float_dx + 2*float_dy;
	
	//*********************************************
	// Generate proto-quad

	/*
	VectorF4 *floatIn[3];

	Ceng::UINT32 floatStart = outputBatch->renderState->shaderLink->fragmentFormat.floatStart;

	floatIn[0] = (VectorF4*)&vertexOrder[0][floatStart];
	floatIn[1] = (VectorF4*)&vertexOrder[1][floatStart];
	floatIn[2] = (VectorF4*)&vertexOrder[2][floatStart];

	VectorD2 *doubleIn[3];

	Ceng::UINT32 doubleStart = outputBatch->renderState->shaderLink->fragmentFormat.doubleStart;


	doubleIn[0] = (VectorD2*)&vertexOrder[0][doubleStart];
	doubleIn[1] = (VectorD2*)&vertexOrder[1][doubleStart];
	doubleIn[2] = (VectorD2*)&vertexOrder[2][doubleStart];
	*/

	FLOAT32 *positionW[3] = {&position[0].w,&position[1].w,&position[2].w};

	auto floatVariables = std::shared_ptr<Ceng::AlignedBuffer<CR_FloatFragment>>
		(renderState->shaderLink->GetFloatVariableBuffer());

	auto doubleVariables = std::shared_ptr<Ceng::AlignedBuffer<CR_DoubleFragment>>
		(renderState->shaderLink->GetDoubleVariableBuffer());

	auto shaderSteps = std::shared_ptr<Ceng::AlignedBuffer<UINT8>>
		(renderState->shaderLink->GetShaderStepBuffer());
	
	renderState->shaderLink->quadFormat.TranslateFragment(vertexOrder,
												positionW,
												&vdy10,&vdy21,
												&vdx10,&vdx21,
												&gradientDiv,
												&initialStepX,&initialStepY,
												&(*floatVariables)[0],
												&(*doubleVariables)[0],
												&(*shaderSteps)[0]);
											
	triangle->fragment.floatBlock = *floatVariables;
	triangle->fragment.doubleBlock = *doubleVariables;
	triangle->fragment.variableStep = *shaderSteps;
	triangle->fragment.vertexConst = nullptr;

	for(Ceng::UINT32 k=0; k < outputBatch.size(); k++)
	{
		if (outputBatch[k] != nullptr)
		{
			outputBatch[k]->floatVariables = floatVariables;
			outputBatch[k]->doubleVariables = doubleVariables;
			outputBatch[k]->shaderSteps = shaderSteps;
		}
	}

	return CE_OK;
}
