/*****************************************************************************
*
* cr-vshader-input.h
*
* By Jari Korkala 4/2013
*
* File created as part of project refactoring.
*
*****************************************************************************/

#ifndef _CENG_CR_VSHADER_INPUT_H
#define _CENG_CR_VSHADER_INPUT_H

#include <ceng/interfaces/vertex-decl.h>

#include <ceng/math/ce-vector.h>

#include "crender-base.h"

namespace Ceng
{
	class CR_vsInputRegister
	{
	public:
		POINTER sourceAddress;
		Ceng::VTX_DATATYPE::value sourceFormat;
	public:
		CR_vsInputRegister() 
		{
			sourceAddress = 0;
			sourceFormat = VTX_DATATYPE::UNKNOWN;
		}

		operator VectorF4() const;
		operator VectorF2() const;
		operator FLOAT32() const;

		
	
	public:

		static void (*call_ToFloat[128])(void *dest,void *source);
		static void (*call_ToFloat2[128])(void *dest,void *source);
		static void (*call_ToFloat4[128])(void *dest,void *source);

	};

	//******************************************************
	// Methods

	inline CR_vsInputRegister::operator VectorF4 () const
	{
		_declspec(align(16)) VectorF4 temp;

		(*call_ToFloat4[sourceFormat])(&temp,(void*)sourceAddress);										
		return temp;
	}

	inline CR_vsInputRegister::operator VectorF2 () const
	{
		_declspec(align(16)) VectorF2 temp;

		(*call_ToFloat2[sourceFormat])(&temp,(void*)sourceAddress);									
		return temp;
	}

	inline CR_vsInputRegister::operator FLOAT32 () const
	{
		FLOAT32 temp;

		(*call_ToFloat[sourceFormat])(&temp,(void*)sourceAddress);									
		return temp;
	}

	// Callbacks

	extern void VSIN_FLOAT_FLOAT_X86(void *dest,void *source);
	extern void VSIN_FLOAT2_FLOAT2_X86_SSE2(void *dest,void *source);
	extern void VSIN_FLOAT4_FLOAT4_X86_SSE(void *dest,void *source);
};

#endif