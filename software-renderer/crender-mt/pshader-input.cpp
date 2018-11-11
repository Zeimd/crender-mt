/*****************************************************************************
*
* pshader-input.cpp
*
* By Jari Korkala 2012-2014
*
*
*
*****************************************************************************/

#include <xmmintrin.h>
#include <emmintrin.h>

#include "pshader-input.h"

using namespace Ceng;

void (*CR_PixelShaderInput::call_to_float[32])(void*,void*,void*,void*) =
	{
		nullptr , // UNKNWON

		&PSIN_FLOAT_FLOAT_X86_SSE, // FLOAT
		nullptr, // FLOAT2
		nullptr, // FLOAT3
		nullptr, // FLOAT4
	};

void(*CR_PixelShaderInput::call_to_float2[32])(void*, void*, void*, void*) =
{
	nullptr, // UNKNWON

	nullptr, // FLOAT
	&PSIN_FLOAT2_FLOAT2_X86_SSE, // FLOAT2
	nullptr, // FLOAT3
	nullptr, // FLOAT4
};

void (*CR_PixelShaderInput::call_to_float4[32])(void*,void*,void*,void*) = 
	{
		// By input format

		nullptr , // UNKNOWN

		nullptr, // FLOAT
		nullptr, // FLOAT2
		nullptr, // FLOAT3
		&PSIN_FLOAT4_FLOAT4_X86_SSE , // FLOAT4
	};


namespace Ceng
{
	void PSIN_FLOAT_FLOAT_X86_SSE(void *dest,void *source,void *perspective,void *steps)
	{
		__m128 sourceVec = _mm_load_ps((float*)source);

		__m128 stepVec = _mm_load_ps((float*)steps);

		__m128 sourceOut = _mm_add_ps(sourceVec, stepVec);

		_mm_store_ps((float*)source, sourceOut);

		__m128 perspectiveVec = _mm_load_ps((float*)perspective);

		sourceVec = _mm_mul_ps(sourceVec, perspectiveVec);

		_mm_store_ps((float*)dest, sourceVec);

		/*
		// Horizontal -> vertical
		__asm
		{
			mov ecx,steps;
			mov eax,perspective;
			
			mov esi,source;
			mov edi,dest;

			movaps xmm0,[esi];
			
			movaps xmm1,xmm0;

			addps xmm0,[ecx]; // Step to next quad
			
			mulps xmm1,[eax]; // Perspective correction

			movaps [esi],xmm0;
			movaps [edi],xmm1;
		}
		*/
	}

	void PSIN_FLOAT2_FLOAT2_X86_SSE(void *dest, void *source, void *perspective, void *steps)
	{
		float *stepPtr = (float*)steps;

		__m128 topRow = _mm_load_ps((float*)source);

		__m128 stepVectorA = _mm_load_ps(&stepPtr[0]);

		__m128 bottomRow = _mm_add_ps(topRow, stepVectorA);

		__m128 stepVectorB = _mm_load_ps(&stepPtr[4]);

		__m128 sourceOut = _mm_add_ps(bottomRow, stepVectorB);

		_mm_store_ps((float*)source, sourceOut);

		__m128d *topRowD = (__m128d*)&topRow;
		__m128d *bottomRowD = (__m128d*)&bottomRow;

		__m128d finalA = _mm_unpacklo_pd(*topRowD, *bottomRowD);
		__m128d finalB = _mm_unpackhi_pd(*topRowD, *bottomRowD);

		__m128 perpectiveVec = _mm_load_ps((float*)perspective);

		__m128 *finalA_float = (__m128*)&finalA;
		__m128 *finalB_float = (__m128*)&finalB;

		*finalA_float = _mm_mul_ps(*finalA_float, perpectiveVec);
		*finalB_float = _mm_mul_ps(*finalB_float, perpectiveVec);

		float *destPtr = (float*)dest;

		_mm_store_ps(&destPtr[0], *finalA_float);
		_mm_store_ps(&destPtr[4], *finalB_float);
	}

	void PSIN_FLOAT4_FLOAT4_X86_SSE(void *dest,void *source,void *perspective,void *steps)
	{
		// Rearranged horizontal -> vertical

		float *stepPtr = (float*)steps;

		__m128 topLeftPixel = _mm_load_ps((float*)source);
		
		__m128 stepVectorA = _mm_load_ps(&stepPtr[0]);
		__m128 topRightPixel = _mm_add_ps(topLeftPixel, stepVectorA);

		__m128 stepVectorB = _mm_load_ps(&stepPtr[4]);
		__m128 bottomLeftPixel = _mm_add_ps(topRightPixel, stepVectorB);

		__m128 bottomRightPixel = _mm_add_ps(bottomLeftPixel, stepVectorA);
		
		__m128 sourceOut = _mm_sub_ps(bottomRightPixel, stepVectorB);
		
		_mm_store_ps((float*)source, sourceOut);

		__m128 xy_Top = _mm_unpacklo_ps(topLeftPixel, topRightPixel);
		__m128 zw_Top = _mm_unpackhi_ps(topLeftPixel, topRightPixel);

		__m128 xy_Bottom = _mm_unpacklo_ps(bottomLeftPixel, bottomRightPixel);
		__m128 zw_Bottom = _mm_unpackhi_ps(bottomLeftPixel, bottomRightPixel);

		__m128d *xy_TopD = (__m128d*)&xy_Top;
		__m128d *xy_BottomD = (__m128d*)&xy_Bottom;

		__m128d xVecD = _mm_unpacklo_pd(*xy_TopD, *xy_BottomD);
		__m128d yVecD = _mm_unpackhi_pd(*xy_TopD, *xy_BottomD);

		__m128d *zw_TopD = (__m128d*)&zw_Top;
		__m128d *zw_BottomD = (__m128d*)&zw_Bottom;

		__m128d zVecD = _mm_unpacklo_pd(*zw_TopD, *zw_BottomD);
		__m128d wVecD = _mm_unpackhi_pd(*zw_TopD, *zw_BottomD);

		__m128 perspectiveVec = _mm_load_ps((float*)perspective);

		__m128 *xVec = (__m128*)&xVecD;
		__m128 *yVec = (__m128*)&yVecD;
		__m128 *zVec = (__m128*)&zVecD;
		__m128 *wVec = (__m128*)&wVecD;

		*xVec = _mm_mul_ps(*xVec, perspectiveVec);
		*yVec = _mm_mul_ps(*yVec, perspectiveVec);
		*zVec = _mm_mul_ps(*zVec, perspectiveVec);
		*wVec = _mm_mul_ps(*wVec, perspectiveVec);

		float *outPtr = (float*)dest;

		_mm_store_ps(&outPtr[0], *xVec);
		_mm_store_ps(&outPtr[4], *yVec);
		_mm_store_ps(&outPtr[8], *zVec);
		_mm_store_ps(&outPtr[12], *wVec);
	}
};