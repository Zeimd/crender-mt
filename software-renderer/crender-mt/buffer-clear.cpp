/*****************************************************************************
*
* buffer-clear.cpp
*
* By Jari Korkala 11/2012
*
*****************************************************************************/

#include <xmmintrin.h>

#include "buffer-clear.h"

using namespace Ceng;

namespace Ceng
{
	void CLEAR_BUFFER_BLOCK(void *bufferPtr,void *fillVec,POINTER vecAmount)
	{
		UINT32 *depthTarget = (UINT32*)bufferPtr;
		UINT32 *value = (UINT32*)fillVec;
		
		UINT32 k;
		UINT32 index = 0;

		for(k=0;k<vecAmount;k++)
		{
			depthTarget[index] = value[0];
			depthTarget[index+1] = value[1];
			depthTarget[index+2] = value[2];
			depthTarget[index+3] = value[3];
			
			index += 4;
		}
	}

	void CLEAR_DEPTH_STENCIL(void *depthPtr,void *stencilPtr,void *depthVec,UINT32 depthCount,
										void *stencilVec,UINT32 stencilCount)
	{

	}

	//**************************************************
	// x86 SSE

	void CLEAR_BUFFER_BLOCK_X86_SSE(void *bufferPtr,void *fillVec,POINTER vecAmount)
	{
		__m128 fill = _mm_load_ps((float*)fillVec);

		float *writePtr = (float*)bufferPtr;

		Ceng::UINT32 blocks = vecAmount >> 2;
		Ceng::UINT32 remainder = vecAmount & 3;

		for (Ceng::UINT32 k = 0; k < blocks; k++)
		{
			_mm_stream_ps(&writePtr[16 * k], fill);
			_mm_stream_ps(&writePtr[16 * k+4], fill);
			_mm_stream_ps(&writePtr[16 * k+8], fill);
			_mm_stream_ps(&writePtr[16 * k+12], fill);
		}

		for (Ceng::UINT32 k = 0; k < remainder; k++)
		{
			_mm_stream_ps(&writePtr[4 * k], fill);
		}
	}
}