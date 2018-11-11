/*****************************************************************************
*
* render-context.h
*
* Created By Jari Korkala 2/3/2015
*
*****************************************************************************/

#ifndef CENG_RENDER_CONTEXT_H
#define CENG_RENDER_CONTEXT_H

#include "base-interface.h"

#include "../datatypes/return-val.h"

#include "../enums/primitive-type.h"

#include "shader-resource-view.h"
#include "sampler-state.h"

namespace Ceng
{
	class RenderTarget;
	class VertexShader;
	class PixelShader;

	class VertexFormat;
	class VertexBuffer;
	class CE_Color;

	class DepthStencilDesc;
	class RasterizerState;
	struct Rectangle;

	class RenderContext : public BASE_INTERFACE
	{
	protected:

		virtual ~RenderContext()
		{
		}

	public:

		RenderContext()
		{
		}

		virtual void Release() = 0;

		virtual const Ceng::CRESULT SetRenderTarget(const UINT32 targetNum,Ceng::RenderTarget *renderTarget) = 0;

		virtual const Ceng::CRESULT ClearTarget(Ceng::RenderTarget *renderTarget, const CE_Color &color) = 0;

		virtual const Ceng::CRESULT ClearDepth(const FLOAT32 depth) = 0;

		virtual const Ceng::CRESULT ClearDepthStencil(const FLOAT32 depth, const UINT32 stencil) = 0;

		virtual const Ceng::CRESULT SetVertexFormat(Ceng::VertexFormat *formatPtr) = 0;
		virtual const Ceng::CRESULT SetVertexShader(Ceng::VertexShader *shaderPtr) = 0;

		virtual const Ceng::CRESULT SetVertexStream(const Ceng::UINT32 index, Ceng::VertexBuffer *source) = 0;

		virtual const Ceng::CRESULT SetPixelShader(Ceng::PixelShader *shaderPtr) = 0;

		virtual const Ceng::CRESULT StartScene() = 0;
		virtual const Ceng::CRESULT EndScene() = 0;
	
		virtual const Ceng::CRESULT DrawPrimitive(const Ceng::PRIMITIVE_TYPE::value type,
										const Ceng::UINT32 firstVertex,const Ceng::UINT32 primitives) = 0;

		virtual const Ceng::CRESULT SetDepthStencilState(const Ceng::DepthStencilDesc *state) = 0;

		virtual const Ceng::CRESULT SetRasterizerState(const Ceng::RasterizerState *state) = 0;

		virtual const Ceng::CRESULT SetScissorRect(const Ceng::Rectangle *rect) = 0;

		virtual const Ceng::CRESULT SetShaderResource(const Ceng::UINT32 index,
			ShaderResourceView *resource) = 0;
		
		virtual const Ceng::CRESULT SetSamplerState(const Ceng::UINT32 index,
			SamplerState *sampler) = 0;

	};
}

#endif