/*****************************************************************************
*
* buffer-clear.h
*
* By Jari Korkala 11/2012
*
* Callbacks for buffer clear methods.
*
* NOTES:
*
* - These can assume that *fillVec is always aligned to 
*       vector processor's required memory alignment
*
* - Vector width is implicit
*
* - 
*
*****************************************************************************/

#ifndef _CONCEPT2_BUFFER_CLEAR_H
#define _CONCEPT2_BUFFER_CLEAR_H

#include "crender-base.h"

namespace Ceng
{
	//*******************************************
	// Portable buffer clear

	extern void CLEAR_BUFFER_BLOCK(void *bufferPtr,void *fillVec,POINTER vecAmount);

	extern void CLEAR_DEPTH_STENCIL(void *depthPtr,void *stencilPtr,void *depthVec,UINT32 depthCount,
										void *stencilVec,UINT32 stencilCount);

	//******************************************
	// x86 SSE buffer clear

	extern void CLEAR_BUFFER_BLOCK_X86_SSE(void *bufferPtr,void *fillVec,POINTER vecAmount);

} // namespace Ceng

#endif