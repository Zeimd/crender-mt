/*****************************************************************************
*
* win32-types.h
*
* By Jari Korkala 10/2012
*
*****************************************************************************/

#ifndef _CONCEPT2_WIN32_TYPES_H
#define _CONCEPT2_WIN32_TYPES_H

//**********************************************************
// Microsoft Visual Studio
// Data types and platform-specific methods

#ifdef _MSC_VER

#include <malloc.h>
	
namespace Ceng
{
	//*******************************
	// Data types

	typedef char INT8;
	typedef unsigned char UINT8;
	
	typedef short INT16;
	typedef unsigned short UINT16;
	
	typedef int INT32;
	typedef unsigned int UINT32;
	
	typedef __int64 INT64;
	typedef unsigned __int64 UINT64;
}

#define PLATFORM_HAS_FPU

#ifdef _M_IX86

namespace Ceng
{
	typedef unsigned int POINTER;
}

#define PLATFORM_CPU_X86
#endif

#ifdef _M_X64

namespace Ceng
{
	typedef unsigned __int64 POINTER;
}

#define PLATFORM_CPU_X86_64

#endif // _M_X64

namespace Ceng
{
	typedef char CHAR;
}

//************************************
// Placeholder typedefs for floating
// point formats

namespace Ceng
{
	typedef float FLOAT_SINGLE;
	typedef double FLOAT_DOUBLE;
}

//*************************************************************
// Aligned memory allocator

#define PLATFORM_ALIGNED_MALLOC // Flag the presence of allocator

namespace Ceng
{
	inline void* AlignedMalloc(UINT32 size,UINT32 alignment)
	{
		return _aligned_malloc(size,alignment);
	}
	
	inline void AlignedFree(void* pointer)
	{
		_aligned_free(pointer);
	}
}

//***************************************************************

/**
 * x86-specific float to int conversion that
 * doesn't change rounding mode to C-language's
 * standard truncation (which is slow).
 */

namespace Ceng
{
	inline const INT32 CE_FloatToInt(const FLOAT_SINGLE x)
	{
		INT32 temp;
		__asm
		{
			fld DWORD PTR x;
			fistp DWORD PTR temp;
		}
		return temp;
	}
	
	inline const INT64 CE_DoubleToInt(const FLOAT_DOUBLE x)
	{
		INT64 temp;
		__asm
		{
			fld QWORD PTR x;
			fistp QWORD PTR temp;
		}
		return temp;
	}

} // Namespace Ceng

#endif // _MSC_VER
#endif // Include guard