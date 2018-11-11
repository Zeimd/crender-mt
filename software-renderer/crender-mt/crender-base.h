/*****************************************************************************
*
* crender-base.h
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#ifndef _CENG_CRENDER_BASE_H
#define _CENG_CRENDER_BASE_H

#include <ceng/datatypes/basic-types.h>
#include <ceng/datatypes/return-val.h>
#include <ceng/datatypes/boolean.h>

namespace Ceng
{
	static const UINT32 CRENDER_MAX_COLOR_TARGETS = 8;
	static const UINT32 CRENDER_MAX_SHADER_TEXTURES = 8;

	static const UINT32 CRENDER_MAX_VTX_DECL_FLOATS = 64;

	static const UINT32 CRENDER_MAX_VERTEX_STREAMS = 8;
	static const UINT32 CRENDER_TEXCOORD_MAX_INDEX = 8;

	static const Ceng::FLOAT32 math_pi = 3.1415926535f;

	static const Ceng::FLOAT32 degrees_to_radians = math_pi / 180.0f;
}

#endif