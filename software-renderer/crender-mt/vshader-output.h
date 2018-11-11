/*****************************************************************************
*
* cr-vshader-output.h
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#ifndef _CENG_CR_VSHADER_OUTPUT_H
#define _CENG_CR_VSHADER_OUTPUT_H

#include <ceng/enums/shader-datatype.h>
#include <ceng/enums/shader-semantic.h>

#include <ceng/math/ce-vector.h>

#include "crender-base.h"

namespace Ceng
{
	class CR_vsOutputRegister
	{
	public:
		POINTER *destAddress;
		POINTER destOffset;

		Ceng::SHADER_DATATYPE::value destFormat;

	public:

		CR_vsOutputRegister() {}
		~CR_vsOutputRegister() {}

		CR_vsOutputRegister& operator = (const FLOAT32 &source);
		CR_vsOutputRegister& operator = (const VectorF2 &source);
		CR_vsOutputRegister& operator = (const VectorF4 &source);	

	public:

		static void (*call_from_Float[32])(void *dest,void *source);
		static void (*call_from_Float2[32])(void *dest,void *source);
		static void (*call_from_Float4[32])(void *dest,void *source);
	};

	inline CR_vsOutputRegister& CR_vsOutputRegister::operator = (const FLOAT32 &source)
	{
		(*call_from_Float[destFormat])((void*)(*destAddress + destOffset),
										(void*)&source);		
		return *this;
	}
	
	inline CR_vsOutputRegister& CR_vsOutputRegister::operator = (const VectorF2 &vector)
	{
		(*call_from_Float2[destFormat])((void*)(*destAddress + destOffset),
										(void*)&vector);
		return *this;
	}
	
	inline CR_vsOutputRegister& CR_vsOutputRegister::operator = (const VectorF4 &vector)
	{
		(*call_from_Float4[destFormat])((void*)(*destAddress + destOffset),
										(void*)&vector);
									

		return *this;
	}

	//********************************************
	// Callbacks

	extern void VSOUT_FLOAT_FLOAT_X86(void *dest,void *source);

	extern void VSOUT_FLOAT2_FLOAT2_X86_SSE2(void *dest,void *source);
	extern void VSOUT_FLOAT4_FLOAT4_X86_SSE(void *dest,void *source);
		

};

#endif