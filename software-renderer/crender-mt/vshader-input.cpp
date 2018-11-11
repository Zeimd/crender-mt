/*****************************************************************************
*
* vshader-input.cpp
*
* By Jari Korkala 5/2013
*
* ----------------------------------------------
*
* Jari Korkala 8/2014
*
* - Renamed to vshader-input.cpp
*
*****************************************************************************/

#include "vshader-input.h"

using namespace Ceng;

void (*CR_vsInputRegister::call_ToFloat[128])(void*,void*) =
	{
		// Indexed by source format

		NULL , // UNKNOWN

		&VSIN_FLOAT_FLOAT_X86 , // FLOAT
		NULL , // FLOAT2
		NULL , // FLOAT3
		NULL , // FLOAT4

		NULL , // DOUBLE
		NULL , // DOUBLE2
		NULL , // DOUBLE3
		NULL , // DOUBLE4

		NULL , // FP16_2
		NULL , // FP16_4
	};

void (*CR_vsInputRegister::call_ToFloat2[128])(void*,void*) =
	{
		// Indexed by source format

		NULL , // UNKNOWN

		NULL , // FLOAT
		&VSIN_FLOAT2_FLOAT2_X86_SSE2 , // FLOAT2
		NULL , // FLOAT3
		NULL , // FLOAT4

		NULL , // DOUBLE
		NULL , // DOUBLE2
		NULL , // DOUBLE3
		NULL , // DOUBLE4

		NULL , // FP16_2
		NULL , // FP16_4
	};

void (*CR_vsInputRegister::call_ToFloat4[128])(void*,void*) =
	{
		// Indexed by source format

		NULL , // UNKNOWN

		NULL , // FLOAT
		NULL , // FLOAT2
		NULL , // FLOAT3
		&VSIN_FLOAT4_FLOAT4_X86_SSE , // FLOAT4

		NULL , // DOUBLE
		NULL , // DOUBLE2
		NULL , // DOUBLE3
		NULL , // DOUBLE4

		NULL , // FP16_2
		NULL , // FP16_4
	};

namespace Ceng
{
	void VSIN_FLOAT_FLOAT_X86(void *dest,void *source)
	{
		Ceng::UINT32 *destPtr = (Ceng::UINT32*)dest;
		Ceng::UINT32 *sourcePtr = (Ceng::UINT32*)source;

		*destPtr = *sourcePtr;
	}

	void VSIN_FLOAT2_FLOAT2_X86_SSE2(void *dest,void *source)
	{
		__m128d temp = _mm_load_sd((double*)source);

		_mm_store_sd((double*)dest,temp);
	}

	void VSIN_FLOAT4_FLOAT4_X86_SSE(void *dest,void *source)
	{
		__m128 temp = _mm_load_ps((float*)source);

		_mm_store_ps((float*)dest, temp);
	}

};

