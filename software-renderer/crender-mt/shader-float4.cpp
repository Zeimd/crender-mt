/*****************************************************************************
*
* shader-float4.cpp
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#include <xmmintrin.h>

#include "shader-datatype.h"

using namespace Ceng;

void (*Shader::Float4::call_mov_float4)(void*,void*) = &Shader::PS_MOV_FLOAT4_FLOAT4_X86_SSE;

void (*Shader::Float4::call_mul_local_float)(void*,void*) = &Shader::PS_MUL_FLOAT4_LOCAL_FLOAT_X86_SSE;
void (*Shader::Float4::call_mul_varying_float)(void*,void*) = &Shader::PS_MUL_FLOAT4_FLOAT_X86_SSE;

namespace Ceng
{
	namespace Shader
	{
		void PS_MOV_FLOAT4_FLOAT4_X86_SSE(void *dest,void *source)
		{
			float *sourcePtr = (float*)source;
			float *destPtr = (float*)dest;

			__m128 vec1 = _mm_load_ps(&sourcePtr[0]);
			__m128 vec2 = _mm_load_ps(&sourcePtr[4]);
			__m128 vec3 = _mm_load_ps(&sourcePtr[8]);
			__m128 vec4 = _mm_load_ps(&sourcePtr[12]);

			_mm_store_ps(&destPtr[0], vec1);
			_mm_store_ps(&destPtr[4], vec2);
			_mm_store_ps(&destPtr[8], vec3);
			_mm_store_ps(&destPtr[12], vec4);
		}

		void PS_MUL_FLOAT4_LOCAL_FLOAT_X86_SSE(void *dest,void *source)
		{
			__m128 scalarFloat = _mm_load1_ps((float*)source);

			float *destPtr = (float*)dest;

			__m128 destVec1 = _mm_load_ps(&destPtr[0]);
			__m128 destVec2 = _mm_load_ps(&destPtr[4]);
			__m128 destVec3 = _mm_load_ps(&destPtr[8]);
			__m128 destVec4 = _mm_load_ps(&destPtr[12]);

			destVec1 = _mm_mul_ps(destVec1,scalarFloat);
			destVec2 = _mm_mul_ps(destVec2, scalarFloat);
			destVec3 = _mm_mul_ps(destVec3, scalarFloat);
			destVec4 = _mm_mul_ps(destVec4, scalarFloat);

			_mm_store_ps(&destPtr[0], destVec1);
			_mm_store_ps(&destPtr[4], destVec2);
			_mm_store_ps(&destPtr[8], destVec3);
			_mm_store_ps(&destPtr[12], destVec4);
		}

		void PS_MUL_FLOAT4_FLOAT4_X86_SSE(void *dest,void *source)
		{
			float *sourcePtr = (float*)source;
			float *destPtr = (float*)dest;

			__m128 source1 = _mm_load_ps(&sourcePtr[0]);
			__m128 source2 = _mm_load_ps(&sourcePtr[4]);
			__m128 source3 = _mm_load_ps(&sourcePtr[8]);
			__m128 source4 = _mm_load_ps(&sourcePtr[12]);

			__m128 dest1 = _mm_load_ps(&sourcePtr[0]);
			__m128 dest2 = _mm_load_ps(&sourcePtr[4]);
			__m128 dest3 = _mm_load_ps(&sourcePtr[8]);
			__m128 dest4 = _mm_load_ps(&sourcePtr[12]);

			dest1 = _mm_mul_ps(dest1, source1);
			dest2 = _mm_mul_ps(dest2, source2);
			dest3 = _mm_mul_ps(dest3, source3);
			dest4 = _mm_mul_ps(dest4, source4);

			_mm_store_ps(&destPtr[0], dest1);
			_mm_store_ps(&destPtr[4], dest2);
			_mm_store_ps(&destPtr[8], dest3);
			_mm_store_ps(&destPtr[12], dest4);
		}

		void PS_MUL_FLOAT4_FLOAT_X86_SSE(void *dest,void *source)
		{
			__m128 scalarFloat = _mm_load_ps((float*)source);

			float *destPtr = (float*)dest;

			__m128 destVec1 = _mm_load_ps(&destPtr[0]);
			__m128 destVec2 = _mm_load_ps(&destPtr[4]);
			__m128 destVec3 = _mm_load_ps(&destPtr[8]);
			__m128 destVec4 = _mm_load_ps(&destPtr[12]);

			destVec1 = _mm_mul_ps(destVec1, scalarFloat);
			destVec2 = _mm_mul_ps(destVec2, scalarFloat);
			destVec3 = _mm_mul_ps(destVec3, scalarFloat);
			destVec4 = _mm_mul_ps(destVec4, scalarFloat);

			_mm_store_ps(&destPtr[0], destVec1);
			_mm_store_ps(&destPtr[4], destVec2);
			_mm_store_ps(&destPtr[8], destVec3);
			_mm_store_ps(&destPtr[12], destVec4);

			/*
			__asm
			{
				mov eax,source;
				mov ecx,dest;

				movaps xmm0,[ecx];
				movaps xmm1,[ecx+16];
				movaps xmm2,[ecx+32];
				movaps xmm3,[ecx+48];

				movaps xmm4,[eax];

				mulps xmm0,xmm4;
				mulps xmm1,xmm4;
				mulps xmm2,xmm4;
				mulps xmm3,xmm4;

				movaps [ecx],xmm0;
				movaps [ecx+16],xmm1;
				movaps [ecx+32],xmm2;
				movaps [ecx+48],xmm3;				
			}
			*/
		}
	};
};