/*****************************************************************************
*
* vshader-output.cpp
*
* By Jari Korkala 5/2013
*
* ----------------------------------------------
*
* Jari Korkala 8/2014
*
* - Renamed to vshader-output.cpp
*
*****************************************************************************/

#include <immintrin.h>

#include "vshader-output.h"

using namespace Ceng;

void (*CR_vsOutputRegister::call_from_Float[32])(void *dest,void *source) =
	{
		// Indexed by destination type

		NULL , // UNKNOWN
		
		&VSOUT_FLOAT_FLOAT_X86 , // FLOAT
		NULL , // FLOAT2
		NULL , // FLOAT3
		NULL , // FLOAT4

		NULL , // DOUBLE
		NULL , // DOUBLE2
		NULL , // DOUBLE3
		NULL , // DOUBLE4

	};

void (*CR_vsOutputRegister::call_from_Float2[32])(void *dest,void *source) =
	{
		// Indexed by destination type

		NULL , // UNKNOWN
		
		NULL , // FLOAT
		&VSOUT_FLOAT2_FLOAT2_X86_SSE2 , // FLOAT2
		NULL , // FLOAT3
		NULL , // FLOAT4

		NULL , // DOUBLE
		NULL , // DOUBLE2
		NULL , // DOUBLE3
		NULL , // DOUBLE4

	};

void (*CR_vsOutputRegister::call_from_Float4[32])(void *dest,void *source) =
	{
		// Indexed by destination type

		NULL , // UNKNOWN
		
		NULL , // FLOAT
		NULL , // FLOAT2
		NULL , // FLOAT3
		&VSOUT_FLOAT4_FLOAT4_X86_SSE , // FLOAT4

		NULL , // DOUBLE
		NULL , // DOUBLE2
		NULL , // DOUBLE3
		NULL , // DOUBLE4

	};

namespace Ceng
{
	void VSOUT_FLOAT_FLOAT_X86(void *dest,void *source)
	{
		Ceng::UINT32 *destPtr = (Ceng::UINT32*)dest;
		Ceng::UINT32 *sourcePtr = (Ceng::UINT32*)source;

		*destPtr = *sourcePtr;
	}

	void VSOUT_FLOAT2_FLOAT2_X86_SSE2(void *dest,void *source)
	{
		__m128d temp = _mm_load_sd((double*)source);

		_mm_store_sd((double*)dest, temp);
	}

	void VSOUT_FLOAT4_FLOAT4_X86_SSE(void *dest,void *source)
	{
		__m128 temp = _mm_load_ps((float*)source);

		_mm_store_ps((float*)dest, temp);
	}

};

