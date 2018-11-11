/*****************************************************************************
*
* shader-common.h
*
* By Jari Korkala 2/2012
*
*****************************************************************************/

#ifndef _CONCEPT2_SHADER_COMMON_H
#define _CONCEPT2_SHADER_COMMON_H

#include "crender-base.h"

namespace Ceng
{
	
	enum CR_SHADER_RENDER_TARGET
	{
		CR_SHADER_TARGET_DEPTH = 0 ,
		CR_SHADER_TARGET_STENCIL = 1 ,

		CR_SHADER_TARGET0 = 2 ,
		CR_SHADER_TARGET1 = 3 ,
		CR_SHADER_TARGET2 = 4 ,
		CR_SHADER_TARGET3 = 5 ,
		CR_SHADER_TARGET4 = 6 ,
		CR_SHADER_TARGET5 = 7 ,
		CR_SHADER_TARGET6 = 8 ,
		CR_SHADER_TARGET7 = 9 ,

		CR_SHADER_TARGET_FORCE32B = 1 << 30 ,
	};
	

	
} // Namespace end


#endif // Include guard