/*****************************************************************************
*
* buffer-layout.h
*
* Created By Jari Korkala 8/2014
*
*****************************************************************************/

#ifndef CRENDER_BUFFER_LAYOUT_H
#define CRENDER_BUFFER_LAYOUT_H

#include "crender-base.h"

namespace Ceng
{
	//***************************************************************
	// CR_BUFFER_DATA_LAYOUT:
	//
	// Selects whether buffer channels are
	//
	// a) packed to a struct with size padded to 2^N
	//
	// b) separated into arrays (element size padded to 2^N)
	//
	// The idea is to optimize for SIMD operations so that
	// the data which undergoes the same operations is
	// continuous in memory.
	
	namespace BUFFER_LAYOUT
	{
		enum value
		{
			// Buffer is allocated as Array of Structs.
			// Example: ARGB[4]
			AOS = 1 ,
			
			// Buffer is allocated as Struct of Arrays.
			// Example: A[4],R[4],G[4],B[4]
			SOA = 2 ,
			
			FORCE32B = UINT32(1) << 30 ,
		};	
	}

}

#endif