/****************************************************************************
*
* pshader-output.cpp
*
* By Jari Korkala 1/2012
*
* Pixel Shader write callbacks to render targets.
*
* ---------------------------------------------------------
*
* Jari Korkala 8/2014
*
* - Renamed to pshader-output.cpp
*
****************************************************************************/

#include <xmmintrin.h>
#include <emmintrin.h>

#include "pshader-output.h"

using namespace Ceng;

void (*CR_psOutputRegister::call_from_Float[128])(void*,void*,void*) =
	{
		&PSOUT_NULL , // Target is unknown

		&PSOUT_NULL , // Target is D16
		&PSOUT_NULL , // Target is D16_S8
		&PSOUT_NULL , // Target is D16_S16

		&PSOUT_NULL , // Target is D24_S8
		&PSOUT_NULL , // Target is D24_S16
	
		&PSOUT_NULL , // Target is DF32
		&PSOUT_NULL , // Target is DF32_S8
		&PSOUT_NULL , // Target is DF32_S16

		&PSOUT_NULL , // Target is DF32_IW
		&PSOUT_NULL , // Target is DF32_IW_S8
		&PSOUT_NULL , // Target is DF32_IW_S16

		&PSOUT_NULL , // Target is DF32_W
		&PSOUT_NULL , // Target is DF32_W_S8
		&PSOUT_NULL , // Target is DF32_W_S16

		&PSOUT_NULL , // Target is DF64
		&PSOUT_NULL , // Target is DF64_S8
		&PSOUT_NULL , // Target is DF64_S16

		&PSOUT_NULL , // Target is DF64_IW
		&PSOUT_NULL , // Target is DF64_IW_S8
		&PSOUT_NULL , // Target is DF64_IW_S16

		&PSOUT_NULL , // Target is DF64_W
		&PSOUT_NULL , // Target is DF64_W_S8
		&PSOUT_NULL , // Target is DF64_W_S16

		&PSOUT_NULL , // Target is C16
		&PSOUT_NULL , // Target is C16_A1
	
		&PSOUT_C32_ARGB_FLOAT_X86_SSE2 , // Target is C32_ARGB
		&PSOUT_NULL , // Target is C32_RGBA
		&PSOUT_NULL , // Target is C32_A2

		&PSOUT_NULL , // Target is C64

		&PSOUT_NULL , // Target is CF32_ARGB
		&PSOUT_NULL , // Target is CF32_R
		&PSOUT_NULL , // Target is CF32_RG

		&PSOUT_NULL , // Target is GRAY8
		&PSOUT_NULL , // Target is C24_RGB

		&PSOUT_NULL , // Target is STENCIL8
		&PSOUT_NULL , // Target is STENCIL16
	};

void (*CR_psOutputRegister::call_from_Float4[128])(void*,void*,void*) =
	{
		&PSOUT_NULL , // Target is unknown

		&PSOUT_NULL , // Target is D16
		&PSOUT_NULL , // Target is D16_S8
		&PSOUT_NULL , // Target is D16_S16

		&PSOUT_NULL , // Target is D24_S8
		&PSOUT_NULL , // Target is D24_S16
	
		&PSOUT_NULL , // Target is DF32
		&PSOUT_NULL , // Target is DF32_S8
		&PSOUT_NULL , // Target is DF32_S16

		&PSOUT_NULL , // Target is DF32_IW
		&PSOUT_NULL , // Target is DF32_IW_S8
		&PSOUT_NULL , // Target is DF32_IW_S16

		&PSOUT_NULL , // Target is DF32_W
		&PSOUT_NULL , // Target is DF32_W_S8
		&PSOUT_NULL , // Target is DF32_W_S16

		&PSOUT_NULL , // Target is DF64
		&PSOUT_NULL , // Target is DF64_S8
		&PSOUT_NULL , // Target is DF64_S16

		&PSOUT_NULL , // Target is DF64_IW
		&PSOUT_NULL , // Target is DF64_IW_S8
		&PSOUT_NULL , // Target is DF64_IW_S16

		&PSOUT_NULL , // Target is DF64_W
		&PSOUT_NULL , // Target is DF64_W_S8
		&PSOUT_NULL , // Target is DF64_W_S16

		&PSOUT_NULL , // Target is C16
		&PSOUT_NULL , // Target is C16_A1
	
		&PSOUT_C32_ARGB_FLOAT4_X86_SSE2 , // Target is C32_ARGB
		&PSOUT_NULL , // Target is C32_RGBA
		&PSOUT_NULL , // Target is C32_A2

		&PSOUT_NULL , // Target is C64

		&PSOUT_NULL , // Target is CF32_ARGB
		&PSOUT_NULL , // Target is CF32_R
		&PSOUT_NULL , // Target is CF32_RG

		&PSOUT_NULL , // Target is GRAY8
		&PSOUT_NULL , // Target is C24_RGB

		&PSOUT_NULL , // Target is STENCIL8
		&PSOUT_NULL , // Target is STENCIL16

	};

namespace Ceng
{
	void PSOUT_NULL(void *dest,void *source,void *coverageMask)
	{
	}

	//const FLOAT32 colorScaleScalar = FLOAT32(255.0f);

	void PSOUT_C32_ARGB_FLOAT_X86_SSE2 (void *dest,void *source,void *coverageMask)
	{
		// Write vertical A-G-R-B

		// pixel[0].blue = float[0]
		// pixel[0].green = float[0]
		// pixel[0].red = float[0]
		// pixel[0].alpha = float[0]

		// pixel[1].blue = float[1]
		// ...

		/*
		__asm
		{
			mov eax,dest;
			mov ecx,source;

			movss xmm7,colorScaleScalar;
			shufps xmm7,xmm7,0;

			movaps xmm0,[ecx];

			mulps xmm0,xmm7; // *= 255.0

			cvttps2dq xmm0,xmm0;

			packssdw xmm0,xmm0;
			packuswb xmm0,xmm0;

			movdqa [eax],xmm0;
		}
		*/
	}

	/*
	_declspec(align(64)) const Ceng::INT8 coverageTable8[16][4] =
	{
		{0,0,0,0} ,
		{-1,0,0,0} ,
		{0,-1,0,0} ,
		{-1,-1,0,0} ,
		{0,0,-1,0} ,
		{-1,0,-1,0} ,
		{0,-1,-1,0} ,
		{-1,-1,-1,0} ,
		{0,0,0,-1} ,
		{-1,0,0,-1} ,
		{0,-1,0,-1} ,
		{-1,-1,0,-1} ,
		{0,0,-1,-1} ,
		{-1,0,-1,-1} ,
		{0,-1,-1,-1} ,
		{-1,-1,-1,-1}
	};
	*/

	void PSOUT_C32_ARGB_FLOAT4_X86_SSE2 (void *dest,void *source,void *coverageMask)
	{
		//_declspec(align(16)) const FLOAT32 testColor[4][4] = { {0.25,0,0,1} , {0,0.5,0,1} , {0,0,0.75,1} , {0,0,0,0} };

		//_declspec(align(16)) const FLOAT32 testColor[4][4] = { {0,0,0,0} , {0,0,0,0} , {0,0,0,0} , {0,0,0,0} };

		const INT8 *coverage = &coverageTable8[(INT32)coverageMask][0];

		/*
		FLOAT32 *sourceColor;
		
		sourceColor = (FLOAT32*)source;
		//sourceColor = (FLOAT32*)&testColor[0][0];
		
		UINT8 *destColor = (UINT8*)dest;

		// DEBUG: normal mode
		if (coverage[0] == -1)
		{
			destColor[0] = UINT8(sourceColor[0] * colorScaleScalar); // blue

			// NOTE: Swap red and green to simplify post-processing

			destColor[8] = UINT8(sourceColor[4] * colorScaleScalar); // green
			destColor[4] = UINT8(sourceColor[8] * colorScaleScalar); // red

			destColor[12] = UINT8(sourceColor[12] * colorScaleScalar); // alpha
		}

		if (coverage[1] == -1)
		{
			destColor[1] = UINT8(sourceColor[1] * colorScaleScalar);
			destColor[9] = UINT8(sourceColor[5] * colorScaleScalar);
			destColor[5] = UINT8(sourceColor[9] * colorScaleScalar);
			destColor[13] = UINT8(sourceColor[13] * colorScaleScalar);
		}

		if (coverage[2] == -1)
		{
			destColor[2] = UINT8(sourceColor[2] * colorScaleScalar);
			destColor[10] = UINT8(sourceColor[6] * colorScaleScalar);
			destColor[6] = UINT8(sourceColor[10] * colorScaleScalar);
			destColor[14] = UINT8(sourceColor[14] * colorScaleScalar);
		}

		if (coverage[3] == -1)
		{
			destColor[3] = UINT8(sourceColor[3] * colorScaleScalar);
			destColor[11] = UINT8(sourceColor[7] * colorScaleScalar);
			destColor[7] = UINT8(sourceColor[11] * colorScaleScalar);
			destColor[15] = UINT8(sourceColor[15] * colorScaleScalar);
		}
		*/

		// DEBUG: additive blending
		/*
		if (coverage[0] == -1)
		{
			destColor[0] += UINT8(sourceColor[0] * colorScaleScalar); // blue
			destColor[8] += UINT8(sourceColor[4] * colorScaleScalar); // green
			destColor[4] += UINT8(sourceColor[8] * colorScaleScalar); // red
			destColor[12] += UINT8(sourceColor[12] * colorScaleScalar); // alpha
		}

		if (coverage[1] == -1)
		{
			destColor[1] += UINT8(sourceColor[1] * colorScaleScalar);
			destColor[9] += UINT8(sourceColor[5] * colorScaleScalar);
			destColor[5] += UINT8(sourceColor[9] * colorScaleScalar);
			destColor[13] += UINT8(sourceColor[13] * colorScaleScalar);
		}

		if (coverage[2] == -1)
		{
			destColor[2] += UINT8(sourceColor[2] * colorScaleScalar);
			destColor[10] += UINT8(sourceColor[6] * colorScaleScalar);
			destColor[6] += UINT8(sourceColor[10] * colorScaleScalar);
			destColor[14] += UINT8(sourceColor[14] * colorScaleScalar);
		}

		if (coverage[3] == -1)
		{
			destColor[3] += UINT8(sourceColor[3] * colorScaleScalar);
			destColor[11] += UINT8(sourceColor[7] * colorScaleScalar);
			destColor[7] += UINT8(sourceColor[11] * colorScaleScalar);
			destColor[15] += UINT8(sourceColor[15] * colorScaleScalar);
		}
		*/
		
		
		
		// DEBUG: bitwise XOR
		
		/*
		if (coverage[0] == -1)
		{
			destColor[0] ^= UINT8(sourceColor[0] * colorScaleScalar); // blue
			destColor[8] ^= UINT8(sourceColor[8] * colorScaleScalar); // green
			destColor[4] ^= UINT8(sourceColor[4] * colorScaleScalar); // red
			destColor[12] ^= UINT8(sourceColor[12] * colorScaleScalar); // alpha
		}

		if (coverage[1] == -1)
		{
			destColor[1] ^= UINT8(sourceColor[1] * colorScaleScalar);
			destColor[9] ^= UINT8(sourceColor[9] * colorScaleScalar);
			destColor[5] ^= UINT8(sourceColor[5] * colorScaleScalar);
			destColor[13] ^= UINT8(sourceColor[13] * colorScaleScalar);
		}

		if (coverage[2] == -1)
		{
			destColor[2] ^= UINT8(sourceColor[2] * colorScaleScalar);
			destColor[10] ^= UINT8(sourceColor[10] * colorScaleScalar);
			destColor[6] ^= UINT8(sourceColor[6] * colorScaleScalar);
			destColor[14] ^= UINT8(sourceColor[14] * colorScaleScalar);
		}

		if (coverage[3] == -1)
		{
			destColor[3] ^= UINT8(sourceColor[3] * colorScaleScalar);
			destColor[11] ^= UINT8(sourceColor[11] * colorScaleScalar);
			destColor[7] ^= UINT8(sourceColor[7] * colorScaleScalar);
			destColor[15] ^= UINT8(sourceColor[15] * colorScaleScalar);
		}		
		*/

		// DEBUG: bitwise XOR of non-covered pixels
		/*
		if (coverage[0] == 0)
		{
			destColor[0] ^= UINT8(sourceColor[0] * colorScaleScalar); // blue
			destColor[8] ^= UINT8(sourceColor[8] * colorScaleScalar); // green
			destColor[4] ^= UINT8(sourceColor[4] * colorScaleScalar); // red
			destColor[12] ^= UINT8(sourceColor[12] * colorScaleScalar); // alpha
		}

		if (coverage[1] == 0)
		{
			destColor[1] ^= UINT8(sourceColor[1] * colorScaleScalar);
			destColor[9] ^= UINT8(sourceColor[9] * colorScaleScalar);
			destColor[5] ^= UINT8(sourceColor[5] * colorScaleScalar);
			destColor[13] ^= UINT8(sourceColor[13] * colorScaleScalar);
		}

		if (coverage[2] == 0)
		{
			destColor[2] ^= UINT8(sourceColor[2] * colorScaleScalar);
			destColor[10] ^= UINT8(sourceColor[10] * colorScaleScalar);
			destColor[6] ^= UINT8(sourceColor[6] * colorScaleScalar);
			destColor[14] ^= UINT8(sourceColor[14] * colorScaleScalar);
		}

		if (coverage[3] == 0)
		{
			destColor[3] ^= UINT8(sourceColor[3] * colorScaleScalar);
			destColor[11] ^= UINT8(sourceColor[11] * colorScaleScalar);
			destColor[7] ^= UINT8(sourceColor[7] * colorScaleScalar);
			destColor[15] ^= UINT8(sourceColor[15] * colorScaleScalar);
		}		
		*/

		

		__m128 colorScaleVec = _mm_load1_ps(&colorScaleScalar);

		float *sourcePtr = (float*)source;

		__m128 blueChannel = _mm_load_ps(&sourcePtr[0]);
		__m128 greenChannel = _mm_load_ps(&sourcePtr[4]);
		__m128 redChannel = _mm_load_ps(&sourcePtr[8]);
		__m128 alphaChannel = _mm_load_ps(&sourcePtr[12]);

		blueChannel = _mm_mul_ps(blueChannel, colorScaleVec);
		greenChannel = _mm_mul_ps(greenChannel, colorScaleVec);
		redChannel = _mm_mul_ps(redChannel, colorScaleVec);
		alphaChannel = _mm_mul_ps(alphaChannel, colorScaleVec);

		__m128i blueInt = _mm_cvtps_epi32(blueChannel);
		__m128i greenInt = _mm_cvtps_epi32(greenChannel);
		__m128i redInt = _mm_cvtps_epi32(redChannel);
		__m128i alphaInt = _mm_cvtps_epi32(alphaChannel);

		__m128i br_Word = _mm_packs_epi32(blueInt, redInt);
		__m128i ga_Word = _mm_packs_epi32(greenInt, alphaInt);

		__m128i writeVec = _mm_packs_epi16(br_Word, ga_Word);

		__m128 coverageVecF = _mm_load1_ps((float*)coverage);

		__m128i *coverageVec = (__m128i*)&coverageVecF;

		__m128i destVec = _mm_load_si128((__m128i*)dest);

		// Select pixels from render target that won't be overwritten
		destVec = _mm_andnot_si128(*coverageVec,destVec);

		// Select pixels from input that will be written
		writeVec = _mm_and_si128(*coverageVec,writeVec);

		// Combine pixels
		writeVec = _mm_or_si128(writeVec, destVec);

		_mm_store_si128((__m128i*)dest, writeVec);
	
	}
};