/*****************************************************************************
*
* clipper-state.h
*
* Created By Jari Korkala 4/10/2014
*
*****************************************************************************/

#ifndef CENG_CLIPPER_STATE_H
#define CENG_CLIPPER_STATE_H

#include <ceng/datatypes/basic-types.h>

namespace Ceng
{
	class ClipperState
	{
	public:
		Ceng::FLOAT32 xMinClipModifier;
		Ceng::FLOAT32 xMaxClipModifier;

		Ceng::FLOAT32 yMinClipModifier;
		Ceng::FLOAT32 yMaxClipModifier;

	public:

		ClipperState()
			: xMinClipModifier(1.0f),xMaxClipModifier(1.0f),
			yMinClipModifier(1.0f),yMaxClipModifier(1.0f)
		{
		}
	};
}


#endif