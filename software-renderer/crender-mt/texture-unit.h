/*****************************************************************************
*
* texture-unit.h
*
* Created By Jari Korkala 6/3/2015
*
*****************************************************************************/

#ifndef CENG_TEXTURE_UNIT_H
#define CENG_TEXTURE_UNIT_H

#include <memory>

#include <ceng/interfaces/shader-resource-view.h>
#include "cr-sampler-state.h"
#include "cr-shader-view.h"

#include "cr-shader-view-tex2d.h"

namespace Ceng
{
	class TextureUnit
	{
	public:

		CR_SamplerState *sampler;

		CR_ShaderResourceView *view;

	public:
		TextureUnit();
		~TextureUnit();

		const CRESULT SetSampler(CR_SamplerState *sampler);
		const CRESULT SetView(CR_ShaderResourceView *view);

	};
}

#endif