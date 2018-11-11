/*****************************************************************************
*
* crender-context.h
*
* Created By Jari Korkala 2/3/2015
*
*****************************************************************************/

#ifndef CRENDER_CONTEXT_H
#define CRENDER_CONTEXT_H

#include <memory>

#include <ceng/interfaces/render-context.h>
#include <ceng/datatypes/ring-buffer.h>

#include "command-thread.h"

#include "pipeline.h"

#include "shader-link.h"

namespace Ceng
{
	class RenderState;
	class ApiCommand;

	class Thread;
	class DrawBatch;

	class ConceptRenderer;

	class CR_Clipper;
	class CR_Rasterizer;

	class CR_RenderContext : public RenderContext
	{
	public:

		ConceptRenderer *parentDevice;

		/**
		* Render state that will be enabled by the next draw call.
		*/
		std::shared_ptr<RenderState> nextRenderState;

		/**
		* Current render state.
		*/
		std::shared_ptr<RenderState> currentRenderState;

		// Api command queue

		CommandProcessor cmdProcessor;
		Thread *cmdThread;

		Ceng::UINT32 apiCallCounter;

		RingBuffer<std::shared_ptr<ApiCommand>> commandQueue;

		std::shared_ptr<ConditionVariable> cmdProcessorSleep;

		std::shared_ptr<DrawBatch> drawBatch;

		Pipeline pipeline;

		Ceng::CR_Clipper *activeClipper;

		CR_ShaderLink shaderLink;

		CR_Rasterizer *activeRasterizer;

		/*
		* Previous width of output window. Used to determine
		* if the value has changed.
		*/
		UINT32 oldWindowWidth;

		/*
		* Previous height of output window. Used to determine
		* if the value has changed.
		*/
		UINT32 oldWindowHeight;

		bool scene;	// Don't allow external access to render targets while true

		/*
		* Width of output window for this frame.
		*/
		UINT32 windowWidth;

		/*
		* Height of output window for this frame.
		*/
		UINT32 windowHeight;

	public:

		CR_RenderContext();
		virtual ~CR_RenderContext();

		virtual void Release() override;

		const Ceng::CRESULT Configure(ConceptRenderer *parentDevice);

		virtual const Ceng::CRESULT SetRenderTarget(const UINT32 targetNum, Ceng::RenderTarget *renderTarget) override;

		virtual const Ceng::CRESULT ClearTarget(Ceng::RenderTarget *renderTarget, const CE_Color &color) override;

		virtual const Ceng::CRESULT ClearDepth(const FLOAT32 depth) override;

		virtual const Ceng::CRESULT ClearDepthStencil(const FLOAT32 depth, const UINT32 stencil) override;

		virtual const Ceng::CRESULT SetVertexFormat(Ceng::VertexFormat *formatPtr) override;
		virtual const Ceng::CRESULT SetVertexShader(Ceng::VertexShader *shaderPtr) override;

		virtual const Ceng::CRESULT SetVertexStream(const Ceng::UINT32 index, Ceng::VertexBuffer *source) override;

		virtual const Ceng::CRESULT SetPixelShader(Ceng::PixelShader *shaderPtr) override;

		virtual const Ceng::CRESULT StartScene() override;
		virtual const Ceng::CRESULT EndScene() override;
	
		virtual const Ceng::CRESULT DrawPrimitive(const Ceng::PRIMITIVE_TYPE::value type,
			const Ceng::UINT32 firstVertex, const Ceng::UINT32 primitives) override;

		virtual const Ceng::CRESULT SetDepthStencilState(const Ceng::DepthStencilDesc *state) override;

		virtual const Ceng::CRESULT SetRasterizerState(const Ceng::RasterizerState *state) override;

		virtual const Ceng::CRESULT SetScissorRect(const Ceng::Rectangle *rect) override;

		virtual const Ceng::CRESULT SetShaderResource(const Ceng::UINT32 index,
			ShaderResourceView *resource) override;

		virtual const Ceng::CRESULT SetSamplerState(const Ceng::UINT32 index,
			SamplerState *sampler) override;

	public:

		const CRESULT Execute_DrawPrimitive(const Ceng::UINT32 apiCallId, Ceng::PRIMITIVE_TYPE::value type,
			UINT32 firstVertex,UINT32 primitives,std::shared_ptr<RenderState> &renderState);

		const CRESULT Execute_ClearTarget(Ceng::RenderTarget *renderTarget,
			const CE_Color &color,
			std::shared_ptr<RenderState> &renderState);

		const CRESULT Execute_ClearDepth(const FLOAT32 depth,
			std::shared_ptr<RenderState> &renderState);

		const CRESULT Execute_ClearDepthStencil(const FLOAT32 depth, const UINT32 stencil,
			std::shared_ptr<RenderState> &renderState);

		const CRESULT InstantiateRenderState();
	};
}

#endif