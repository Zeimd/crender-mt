/*****************************************************************************
*
* clipper.cpp
*
* By Jari Korkala 10/2013
*
*****************************************************************************/

#include "clipper.h"
#include "clipper-state.h"

#include "shader-link.h"

#include "task-triangle-setup.h"

using namespace Ceng;

CR_Clipper::CR_Clipper()
{
	/*
	xMinClipModifier = FLOAT32(1.0f);
	xMaxClipModifier = FLOAT32(1.0f);

	yMinClipModifier = FLOAT32(1.0f);
	yMaxClipModifier = FLOAT32(1.0f);
	*/
}

CR_Clipper::~CR_Clipper()
{
}

const CRESULT CR_Clipper::CreateClipperState(const FLOAT32 top,const FLOAT32 left,
									 const FLOAT32 bottom,const FLOAT32 right,
									 const FLOAT32 minTop,const FLOAT32 minLeft,
									 const FLOAT32 maxBottom,const FLOAT32 maxRight,
									 ClipperState &out_state)
{
	// Determine device coordinate origin

	FLOAT32 halfWidth = (right-left)*0.5f;
	FLOAT32 halfHeight = (bottom-top)*0.5f;

	FLOAT32 originX = left + halfWidth;
	FLOAT32 originY = top + halfHeight;

	out_state.xMinClipModifier = fabs((minLeft - originX) / halfWidth);
	out_state.xMaxClipModifier = fabs((maxRight - originX) / halfWidth);

	out_state.yMinClipModifier = fabs((maxBottom - originY) / halfHeight);
	out_state.yMaxClipModifier = fabs((minTop - originY) / halfHeight);

	return CE_OK;
}

const CRESULT CR_Clipper::ClipPrimitives(std::shared_ptr<ClipperBatch> &batch,
										 LockingStage &outputQueue)
{
	CRESULT cresult;
	Ceng::UINT32 k;

	Ceng::UINT32 newFragCount = 0;

	const Ceng::UINT32 outputBatchSize = 10*batch->batchSize;

	std::shared_ptr<TriangleBatch> outputBatch = 
		std::shared_ptr<TriangleBatch>(new TriangleBatch(batch->apiCallId,outputBatchSize,batch->renderState,
		batch->fragmentCache,batch->cacheA));

	for(k=0;k<batch->primitiveList.size();k++)
	{
		switch(batch->primitiveList[k].primitiveType)
		{
		case PRIMITIVE_TYPE::TRIANGLE_LIST:
			
			cresult = HomogeneousClipTriangle(batch->primitiveList[k],&newFragCount,outputBatch);
		}
	}

	outputQueue.AddTask(0,std::shared_ptr<LockingTask>(new Task_TriangleSetup(outputBatch)));

	return CE_OK;
}

const CRESULT CR_Clipper::HomogeneousClipTriangle(CR_PrimitiveData &primitive,
												  Ceng::UINT32 *newFragmentCount,
												  std::shared_ptr<TriangleBatch> &outputBatch)
{
	const ClipperState &clipperState = outputBatch->renderState->clipperState;

	VectorF4 *currentPos;

	VectorF4 delta;
	
	//******************************************
	// Construct input pointer array

	UINT8 *vertexIn[4];
	
	vertexIn[0] = primitive.fragmentPtr[0];
	vertexIn[1] = primitive.fragmentPtr[1];
	vertexIn[2] = primitive.fragmentPtr[2];
	vertexIn[3] = primitive.fragmentPtr[0]; // Cyclic array simplifies loop over triangle edges

	//******************************************
	// Trivial accept/reject

	VectorF4 test;

	UINT32 minRejectMask[3] = {15,15,15};
	UINT32 maxRejectMask[3] = {15,15,15};

	UINT32 acceptMask=0; // Set to non-zero to disable trivial accept

	const FLOAT32 epsilonW = 0.0001f;

	for(Ceng::UINT32 k=0;k<3;k++)
	{
		currentPos = (VectorF4*)vertexIn[k];
		
		// Construct a vector that tests for
		// x < -w , y < -w , z < 0 , w < epsilon. The corresponding component
		// is negative if the condition is true

		test.x = currentPos->x + currentPos->w * clipperState.xMinClipModifier;
		test.y = currentPos->y + currentPos->w * clipperState.yMinClipModifier;
		test.z = currentPos->z;
		test.w = currentPos->w - epsilonW;

		// For trivial reject, check if 
		// (x < -w) or (y < -w) or (z < 0) is true for all
		// vertices. Do this by bitwise OR of sign bits.

		minRejectMask[k] &= test.SignMask();

		// For trivial accept, check that
		// (-w <= x <= w) and (-w <= y <= w) and (0 <= z <= w)
		// for all vertices. Do this by bitwise OR of sign bits.

		acceptMask |= test.SignMask();
		
		// Construct a vector that tests for
		// x > w , y > w , z > w. The corresponding component
		// is negative if the condition is true

		test.x = currentPos->w * clipperState.xMaxClipModifier - currentPos->x;
		test.y = currentPos->w * clipperState.yMaxClipModifier - currentPos->y;
		test.z = currentPos->w - currentPos->z;
		test.w = 0;

		maxRejectMask[k] &= test.SignMask();

		// Trivial accept mask for maximum tests is stored in bits 4-7
		acceptMask |= (test.SignMask() << 4);
	}

	UINT32 trivialRejectMin = minRejectMask[0] & minRejectMask[1] & minRejectMask[2];
	UINT32 trivialRejectMax = maxRejectMask[0] & maxRejectMask[1] & maxRejectMask[2];

	// Trivial reject if minimum test and maximum test have
	// at least one bit set
	if (trivialRejectMin | trivialRejectMax)
	{
		//*outputSize = 0;
		return CE_OK;
	}
	
	// Trivial accept if all coordinates pass both minimum and
	// maximum tests (ie. acceptMask has no bits set)
	if (acceptMask == 0)
	{	
		outputBatch->primitiveList.push_back(primitive);

		return CE_OK;
	}

	// Determine which planes to clip against

	UINT32 clipMin = minRejectMask[0] | minRejectMask[1] | minRejectMask[2];
	UINT32 clipMax = maxRejectMask[0] | maxRejectMask[1] | maxRejectMask[2];

	/**
	 * Two pointer arrays that are alternated between clip planes.
	 */
	UINT8 *pointerOut[2][12];

	UINT32 outputGroup = 1; // Which pointer array is output

	pointerOut[0][0] = vertexIn[0];
	pointerOut[0][1] = vertexIn[1];
	pointerOut[0][2] = vertexIn[2];
	pointerOut[0][3] = vertexIn[0];


	INT32 outputCount = 0;
	UINT32 inputIndex;

	UINT8 **inputIndices; // Array ptr for input fragment pointers
	UINT8 **outputIndices; // Array ptr for output fragment pointers

	UINT8 **finalOutput = pointerOut[0];
	UINT32 finalSize = 3;

	VectorF4 *nextPos;

	outputCount = 0;
	UINT32 cacheIndex = 0;

	UINT32 inputSize;

	CR_ShaderLink *shaderLink = outputBatch->renderState->shaderLink;

	UINT32 fragSizeBytes = shaderLink->fragmentFormat.size;

	UINT8 *outPtr = &(*outputBatch->cacheA)[(*newFragmentCount)*fragSizeBytes];

	//*****************************************
	// Clip against w = epsilonW

	if (clipMin & 8)
	{
		inputIndices = finalOutput;
		inputSize = finalSize;

		outputIndices = pointerOut[outputGroup];		

		outputGroup ^= 1; // Switch output group

		outputCount = 0;

		for(inputIndex=0;inputIndex<inputSize;inputIndex++)
		{
			currentPos = (VectorF4*)&inputIndices[inputIndex][0];

			nextPos = (VectorF4*)&inputIndices[inputIndex+1][0];

			// NOTE: -epsilonW is used as parameter so that the same clipping function
			//       can be used in this case as well.

			
			Ceng::UINT32 temp = ClipEdgeAgainstMinimum(inputIndices[inputIndex],inputIndices[inputIndex+1],
												outPtr,
												&outputIndices[outputCount],
												currentPos->w,
												-epsilonW,
												nextPos->w,
												-epsilonW,
												shaderLink);
												
			*newFragmentCount += temp;
			outputCount += temp;
			outPtr += fragSizeBytes * temp;
		}

		// Add first vertex again to ensure simple looping
		outputIndices[outputCount] = outputIndices[0];

		finalOutput = outputIndices;
		finalSize = outputCount;

		// Catch empty output , because input from previous stage
		// could be trivially rejected

		if (finalSize == 0)
		{
			return CE_OK;
		}
	}

	if (clipMin & 4)
	{
		// Clip against z = 0

		inputIndices = finalOutput;
		inputSize = finalSize;

		outputIndices = pointerOut[outputGroup];
		
		outputGroup ^= 1; // Switch output group

		outputCount = 0;

		
		for(inputIndex=0;inputIndex<inputSize;inputIndex++)
		{
			currentPos = (VectorF4*)&inputIndices[inputIndex][0];

			nextPos = (VectorF4*)&inputIndices[inputIndex+1][0];

			Ceng::UINT32 temp = ClipEdgeAgainstMinimum(inputIndices[inputIndex],inputIndices[inputIndex+1],
												outPtr,
												&outputIndices[outputCount],
												currentPos->z,
												0.0f,
												nextPos->z,
												0.0f,
												shaderLink);

			*newFragmentCount += temp;
			outputCount += temp;
			outPtr += fragSizeBytes * temp;
		}
		

		// Add first vertex again to ensure simple looping
		outputIndices[outputCount] = outputIndices[0];

		finalOutput = outputIndices;
		finalSize = outputCount;

		// Catch empty output , because input from previous stage
		// could be trivially rejected

		if (finalSize == 0)
		{
			return CE_OK;
		}
	}

	if (clipMax & 4)
	{
		// Clip against z = w

		inputIndices = finalOutput;
		inputSize = finalSize;

		outputIndices = pointerOut[outputGroup];
		
		outputGroup ^= 1; // Switch output group

		outputCount = 0;

		
		for(inputIndex=0;inputIndex<inputSize;inputIndex++)
		{
			currentPos = (VectorF4*)&inputIndices[inputIndex][0];

			nextPos = (VectorF4*)&inputIndices[inputIndex+1][0];

			Ceng::UINT32 temp = ClipEdgeAgainstMaximum(inputIndices[inputIndex],inputIndices[inputIndex+1],
												outPtr,
												&outputIndices[outputCount],
												currentPos->z,
												currentPos->w,
												nextPos->z,
												nextPos->w,
												shaderLink);

			*newFragmentCount += temp;
			outputCount += temp;
			outPtr += fragSizeBytes * temp;
		}

		// Add first vertex again to ensure simple looping
		outputIndices[outputCount] = outputIndices[0];

		finalOutput = outputIndices;
		finalSize = outputCount;

		// Catch empty output , because input from previous stage
		// could be trivially rejected

		if (finalSize == 0)
		{
			return CE_OK;
		}
	}
	
	if (clipMin & 1)
	{
		// Clip against x = -w

		inputIndices = finalOutput;
		inputSize = finalSize;

		outputIndices = pointerOut[outputGroup];
		
		outputGroup ^= 1; // Switch output group

		outputCount = 0;

		
		for(inputIndex=0;inputIndex<inputSize;inputIndex++)
		{
			currentPos = (VectorF4*)&inputIndices[inputIndex][0];

			nextPos = (VectorF4*)&inputIndices[inputIndex+1][0];

			Ceng::UINT32 temp = ClipEdgeAgainstMinimum(inputIndices[inputIndex],inputIndices[inputIndex+1],
												outPtr,
												&outputIndices[outputCount],
												currentPos->x,
												currentPos->w * clipperState.xMinClipModifier,
												nextPos->x,
												nextPos->w * clipperState.xMinClipModifier,
												shaderLink);

			*newFragmentCount += temp;
			outputCount += temp;
			outPtr += fragSizeBytes * temp;
		}
		

		// Add first vertex again to ensure simple looping
		outputIndices[outputCount] = outputIndices[0];

		finalOutput = outputIndices;
		finalSize = outputCount;

		// Catch empty output , because input from previous stage
		// could be trivially rejected

		if (finalSize == 0)
		{
			return CE_OK;
		}
	}

	if (clipMax & 1)
	{
		// Clip against x = w

		inputIndices = finalOutput;
		inputSize = finalSize;

		outputIndices = pointerOut[outputGroup];
		
		outputGroup ^= 1; // Switch output group

		outputCount = 0;

		for(inputIndex=0;inputIndex<inputSize;inputIndex++)
		{
			currentPos = (VectorF4*)&inputIndices[inputIndex][0];

			nextPos = (VectorF4*)&inputIndices[inputIndex+1][0];

			Ceng::UINT32 temp = ClipEdgeAgainstMaximum(inputIndices[inputIndex],inputIndices[inputIndex+1],
												outPtr,
												&outputIndices[outputCount],
												currentPos->x,
												currentPos->w * clipperState.xMaxClipModifier,
												nextPos->x,
												nextPos->w * clipperState.xMaxClipModifier,
												shaderLink);

			*newFragmentCount += temp;
			outputCount += temp;
			outPtr += fragSizeBytes * temp;
		}

		// Add first vertex again to ensure simple looping
		outputIndices[outputCount] = outputIndices[0];

		finalOutput = outputIndices;
		finalSize = outputCount;

		// Catch empty output , because input from previous stage
		// could be trivially rejected

		if (finalSize == 0)
		{
			return CE_OK;
		}
	}
	
	if (clipMin & 2)
	{
		// Clip against y = -w

		inputIndices = finalOutput;
		inputSize = finalSize;

		outputIndices = pointerOut[outputGroup];
		
		outputGroup ^= 1; // Switch output group

		outputCount = 0;

		for(inputIndex=0;inputIndex<inputSize;inputIndex++)
		{
			currentPos = (VectorF4*)&inputIndices[inputIndex][0];

			nextPos = (VectorF4*)&inputIndices[inputIndex+1][0];

			Ceng::UINT32 temp = ClipEdgeAgainstMinimum(inputIndices[inputIndex],inputIndices[inputIndex+1],
												outPtr,
												&outputIndices[outputCount],
												currentPos->y,
												currentPos->w * clipperState.yMinClipModifier,
												nextPos->y,
												nextPos->w * clipperState.yMinClipModifier,
												shaderLink);

			*newFragmentCount += temp;
			outputCount += temp;
			outPtr += fragSizeBytes * temp;
		}

		// Add first vertex again to ensure simple looping
		outputIndices[outputCount] = outputIndices[0];

		finalOutput = outputIndices;
		finalSize = outputCount;

		// Catch empty output , because input from previous stage
		// could be trivially rejected

		if (finalSize == 0)
		{
			return CE_OK;
		}
	}	
	

	
	if (clipMax & 2)
	{
		// Clip against y = w

		inputIndices = finalOutput;
		inputSize = finalSize;

		outputIndices = pointerOut[outputGroup];
		
		outputGroup ^= 1; // Switch output group

		outputCount = 0;

		
		for(inputIndex=0;inputIndex<inputSize;inputIndex++)
		{
			currentPos = (VectorF4*)&inputIndices[inputIndex][0];

			nextPos = (VectorF4*)&inputIndices[inputIndex+1][0];

			Ceng::UINT32 temp = ClipEdgeAgainstMaximum(inputIndices[inputIndex],inputIndices[inputIndex+1],
												outPtr,
												&outputIndices[outputCount],
												currentPos->y,
												currentPos->w * clipperState.yMaxClipModifier,
												nextPos->y,
												nextPos->w * clipperState.yMaxClipModifier,
												shaderLink);

			*newFragmentCount += temp;
			outputCount += temp;
			outPtr += fragSizeBytes * temp;
		}
		

		// Add first vertex again to ensure simple looping
		outputIndices[outputCount] = outputIndices[0];

		finalOutput = outputIndices;
		finalSize = outputCount;

		// Catch empty output , because input from previous stage
		// could be trivially rejected

		if (finalSize == 0)
		{
			return CE_OK;
		}
	}
	
	Ceng::UINT32 triangles = finalSize - 2;

	CR_PrimitiveData outPrimitive;

	outPrimitive.primitiveType = PRIMITIVE_TYPE::TRIANGLE_LIST;
	outPrimitive.fragmentPtr[0] = finalOutput[0];

	for(Ceng::UINT32 k=0;k<triangles;k++)
	{
		outPrimitive.fragmentPtr[1] = finalOutput[k+1];
		outPrimitive.fragmentPtr[2] = finalOutput[k+2];

		outputBatch->primitiveList.push_back(outPrimitive);
	}
	
	return CE_OK;
}


const Ceng::UINT32 CR_Clipper::ClipEdgeAgainstMinimum(Ceng::UINT8 *startVertex,
													  Ceng::UINT8 *endVertex,
													  Ceng::UINT8 *generatedVertex,
													  Ceng::UINT8 **indexOut,
													  Ceng::FLOAT32 startCoord,
													  Ceng::FLOAT32 startW,
													  Ceng::FLOAT32 endCoord,
													  Ceng::FLOAT32 endW,
													  CR_ShaderLink *shaderLink)
{
	UINT32 outputIndex = 0;

	if ( (startCoord >= -startW) )
	{
		// startCoord is inside

		// Add starting vertex since it's already inside

		indexOut[outputIndex] = startVertex;
		outputIndex++;

		// Don't clip if the entire edge is inside

		if (endCoord >= -endW)
		{
			return outputIndex;
		}
	}
	else
	{	
		// startCoord is outside

		// Don't clip if the entire edge is outside
		if (endCoord < -endW)
		{
			return outputIndex;
		}

		// We always clip from inside to outside, so swap endpoints

		std::swap(startVertex,endVertex);
		std::swap(startCoord,endCoord);
		std::swap(startW,endW);
	}

	FLOAT32 transportFactor;

	// The clipping plane's location is given by equation
	// startCoord + transport*(endCoord-startCoord) = -(startW + transport*(endW-startW))

	transportFactor = (startCoord+startW) / ( -(endW-startW) - (endCoord- startCoord));

	indexOut[outputIndex] = generatedVertex;
	outputIndex++;

	// Transport all FLOAT-block variables to clipping plane
	// intersection
	VectorF4 *startVariable = (VectorF4*)startVertex;
	VectorF4 *endVariable = (VectorF4*)endVertex;
	VectorF4 *result = (VectorF4*)generatedVertex;

	VectorF4 delta;

	for(Ceng::UINT32 k=0;k<shaderLink->fragmentFormat.floatBlocks;k++)
	{
		delta = endVariable[k] - startVariable[k];
		delta *= transportFactor;

		result[k] = startVariable[k] + delta;
	}
	
	return outputIndex;
}

const Ceng::UINT32 CR_Clipper::ClipEdgeAgainstMaximum(Ceng::UINT8 *startVertex,
													  Ceng::UINT8 *endVertex,
													  Ceng::UINT8 *generatedVertex,
													  Ceng::UINT8 **indexOut,
													  Ceng::FLOAT32 startCoord,
													  Ceng::FLOAT32 startW,
													  Ceng::FLOAT32 endCoord,
													  Ceng::FLOAT32 endW,
													  CR_ShaderLink *shaderLink)
{
	UINT32 outputIndex = 0;

	if ( (startCoord <= startW) )
	{
		// startCoord is inside

		// Add starting vertex since it's already inside

		indexOut[outputIndex] = startVertex;
		outputIndex++;

		// Don't clip if the entire edge is inside

		if (endCoord <= endW)
		{
			return outputIndex;
		}
	}
	else
	{	
		// startCoord is outside

		// Don't clip if the entire edge is outside
		if (endCoord > endW)
		{
			return outputIndex;
		}

		// We always clip from inside to outside, so swap endpoints

		std::swap(startVertex,endVertex);
		std::swap(startCoord,endCoord);
		std::swap(startW,endW);
	}

	FLOAT32 transportFactor;

	// The clipping plane's location is given by equation
	// startCoord + transport*(endCoord-startCoord) = (startW + transport*(endW-startW))

	transportFactor = (startCoord-startW) / ( (endW-startW) - (endCoord- startCoord));
	
	indexOut[outputIndex] = generatedVertex;
	outputIndex++;

	// Transport all FLOAT-block variables to clipping plane
	// intersection
	VectorF4 *startVariable = (VectorF4*)startVertex;
	VectorF4 *endVariable = (VectorF4*)endVertex;
	VectorF4 *result = (VectorF4*)generatedVertex;

	VectorF4 delta;

	for(Ceng::UINT32 k=0;k<shaderLink->fragmentFormat.floatBlocks;k++)
	{
		delta = endVariable[k] - startVariable[k];
		delta *= transportFactor;

		result[k] = startVariable[k] + delta;
	}
	
	return outputIndex;
}