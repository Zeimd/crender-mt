/*****************************************************************************
*
* cr-shader-view-tex2d.h
*
* Created By Jari Korkala 5/3/2015
*
*****************************************************************************/

#ifndef CENG_CR_SHADER_RES_TEX2D_H
#define CENG_CR_SHADER_RES_TEX2D_H

#include <ceng/datatypes/tex2d-desc.h>

#include "cr-shader-view.h"

#include "cr-texture-2d.h"

namespace Ceng
{
	class CR_ShaderViewTex2D : public CR_ShaderResourceView
	{
	public:

		Texture2dDesc desc;

		TextureMipVector textures;

		enum SamplingDir
		{
			nearest,
			minification,
			magnification,
		};

	public:

		CR_ShaderViewTex2D();
		CR_ShaderViewTex2D(const Texture2dDesc &desc, TextureMipVector &&textures);

		virtual ~CR_ShaderViewTex2D();

		virtual void Release() override;

		virtual void Sample2D(Shader::Float2 &uv, const CR_SamplerState *sampler,
			Ceng::FLOAT32 *out_colorVecs) override;

	protected:

		typedef void (CR_ShaderViewTex2D::*FilterFunction)(const Ceng::INT32*, const Ceng::INT32*, Ceng::UINT32,
			const Ceng::FLOAT32, Ceng::FLOAT32*);

		static FilterFunction minFilterTable[16];

		static FilterFunction magFilterTable[8];

		void Nearest_Port(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel,
			const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color);
		
		void Nearest_SSE2(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel, 
			const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color);

		void Linear_Port(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel, 
			const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color);

		void Linear_SSE2(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel, 
			const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color);

		void Nearest_MipNearest_Port(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel, 
			const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color);

		void Nearest_MipNearest_SSE2(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel,
			const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color);

		void Linear_MipNearest_Port(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel, 
			const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color);

		void Linear_MipNearest_SSE2(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel,
			const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color);
		
		void Nearest_MipLinear(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel, 
			const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color);

		void Linear_MipLinear(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel, 
			const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color);

	};
}

#endif