/*****************************************************************************
*
* cr-shader-view.h
*
* Created By Jari Korkala 6/3/2015
*
*****************************************************************************/

#ifndef CENG_CR_SHADER_VIEW_H
#define CENG_CR_SHADER_VIEW_H

#include <ceng/interfaces/shader-resource-view.h>

namespace Ceng
{
	class CR_SamplerState;

	namespace Shader
	{
		class Float2;
	}

	class CR_ShaderResourceView : public ShaderResourceView
	{
	public:
		CR_ShaderResourceView()
		{
		}

		virtual ~CR_ShaderResourceView()
		{
		}

		virtual void Sample2D(Shader::Float2 &uv,const CR_SamplerState *sampler,
								Ceng::FLOAT32 *out_colorVecs) =0;
	};
}

#endif