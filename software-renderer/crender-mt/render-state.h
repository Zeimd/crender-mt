/*****************************************************************************
*
* render-state.h
*
* By Jari Korkala 8/2014
*
*****************************************************************************/

#ifndef CRENDER_RENDER_STATE_H
#define CRENDER_RENDER_STATE_H

#include <memory>

#include <ceng/interfaces/rasterizer-state.h>
#include <ceng/interfaces/depth-stencil-desc.h>

#include <ceng/datatypes/rectangle.h>

#include "crender-base.h"

#include "vertex-stream.h"

#include "vshader-instance.h"

#include "rtarget-data.h"

#include "pshader-instance.h"

#include "clipper-state.h"
#include "viewport-state.h"

#include "texture-unit.h"

namespace Ceng
{
	class CR_VertexFormat;
	class CR_VertexShader;
	class CR_PixelShader;

	class CR_Clipper;
	class CR_Rasterizer;

	class CR_ShaderLink;

	class CR_NewTargetData;

	class CR_QuadGenerator;

	typedef std::shared_ptr<CR_VertexShaderInstance> VshaderInstancePtr;
	typedef std::shared_ptr<PixelShaderInstance> PshaderInstancePtr;

	class RenderState
	{
	public:
		Ceng::DepthStencilDesc depthStencilState;

		Ceng::RasterizerState rasterizerState;

		/**
		 * This rectangle is used as *activeRect* when scissor test is
		 * enabled.
		 */
		Ceng::Rectangle scissorRect;

		Ceng::Rectangle activeRect;

		Ceng::ClipperState clipperState;

		Ceng::CR_Clipper *clipper;

		Ceng::ViewportState viewportState;

		// Vertex shader state

		Ceng::UINT32 maxVertexStreams;

		VertexStreamData* vertexStreams;

		CR_VertexFormat *vertexFormat;

		CR_VertexShader *vertexShader;

		CR_ShaderLink *shaderLink;

		std::shared_ptr<CR_NewTargetData> depthBuffer;

		CR_Rasterizer *rasterizer;

		Ceng::UINT32 activeTargets;

		std::shared_ptr<CR_NewTargetData> renderTargets[2+CRENDER_MAX_COLOR_TARGETS];

		/**
		 * Vertex shader with current state applied.
		 */
		VshaderInstancePtr vshaderInstance;

		// Texture unit state

		std::vector<TextureUnit> textureUnits;

		// Pixel shader state

		CR_PixelShader *pixelShader;	

		/** 
		 * Pixel shader with current state applied.
		 */
		std::vector<PshaderInstancePtr> pshaderInstance;

		RenderState(const Ceng::UINT32 maxVertexStreams,const Ceng::UINT32 maxTextureUnits);

		RenderState(const RenderState &source);

		~RenderState();

		const CRESULT CommitState(const RenderState &oldState,CR_ShaderLink *shaderLink,
									const Ceng::UINT32 cacheLineSize,
									const Ceng::UINT32 renderThreads);
	};
}

#endif