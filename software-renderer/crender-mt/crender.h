/*****************************************************************************
*
* crender.h
*
* By Jari Korkala 30/6/2011
*
* Class ConceptRenderer : Implements a software renderer.
*
*****************************************************************************/

#ifndef _CONCEPT2_CONCEPT_RENDERER_H
#define _CONCEPT2_CONCEPT_RENDERER_H

#include <vector>
#include <exception>
#include <atomic>

#include <ceng/interfaces/render-device.h>

#include "crender-base.h"

// Include CE_Buffer declaration
#include "buffer-factory.h"

// Include CR_RenderTarget declaration
#include "rendertarget-cr.h"


// Include CR_Rasterizer declaration

#include "cr-image-buffer.h"



#include <ceng/interfaces/graphics-2d.h>

#include <ceng/interfaces/display-mode.h>

#include <ceng/lib/liblog.h>

#include <ceng/datatypes/swap-chain-desc.h>

//*************************************************************************
// Concept Renderer

namespace Ceng
{
	class RenderContext;

	class CR_VertexFormat;
	class CR_VertexShader;

	class CR_PixelShader;

	class CR_Texture2D;

	class CR_Rasterizer;

	class CPU_Info;

	class CR_RenderContext;
	class SwapChain;

	class ConceptRenderer : public RenderDevice
	{
	public:

		FileLog debugLog;

		SwapChainDesc outputParams;
		
		CPU_Info *cpuInfo;

		CR_RenderContext *renderContext;
	
		Graphics2D *outputDevice;
		PlatformWindow *deviceWindow;

		DisplayMode desktopMode;
		
		Ceng::IMAGE_FORMAT::value backBufferFormat;

		CR_BufferFactory bufferFactory;
		
		
		std::shared_ptr<CR_NewTargetData> depthBuffer;

		Ceng::UINT32 maxScreenSubBuckets;		

	
		
		UINT8 renderTileSize;
		UINT16 renderThreadCount;
		UINT16 cacheLineSize;
		
		UINT16 maxViewWidth,maxViewHeight;
		
		//*******************************************
		// Scene variables
		
	
	
	public:
		ConceptRenderer();
		virtual ~ConceptRenderer();
		
		virtual void Release() override;

		CRESULT Configure(CPU_Info *cpuInfo,Graphics2D *devicePtr,DisplayMode *desktopMode,
							SwapChainDesc &swapChainDesc);

		const Ceng::CRESULT GetContext(RenderContext **contextPtr);

		const CRESULT AutoCreateDepthStencil(SwapChainDesc &swapChainDesc,
			DisplayMode *desktopMode);

		const CRESULT CreateSwapChain(SwapChainDesc &swapChainDesc, SwapChain **out_swapChain);
		
		CRESULT CreateFrameBuffer(SwapChainDesc *parameters,
									UINT32 desktopWidth,
									UINT32 desktopHeight);

		//**************************
		// Vertex format methods

		virtual CRESULT CreateVertexShader(Ceng::VertexShader **shaderPtr);
		
		virtual CRESULT CreateVertexFormat(const std::vector<Ceng::VertexDeclData> &vertexDecl,
											Ceng::VertexFormat **format);

		UINT32 VertexVariableSize(Ceng::VTX_DATATYPE::value dataType);		
		
		CRESULT CreateVertexShader(void *symbolTable,void *shaderFunction,Ceng::VertexShader **shaderPtr);

		
		//*********************************
		// Pixel shader methods

		virtual CRESULT CreatePixelShader(Ceng::PixelShader **shaderPtr);

		//***************************
		// Vertex buffer methods
		
		virtual CRESULT CreateVertexBuffer(UINT32 vertexSizeBytes,
											UINT32 vertexNum,
											UINT32 usage,
											Ceng::VertexBuffer **destPtr);
		
		//********************************
		// Texture methods

		virtual const CRESULT CreateSamplerState(const SamplerStateDesc &desc,
			Ceng::SamplerState **out_state) override;

		virtual const CRESULT CreateShaderResourceView(RenderResource *resource,
			ShaderResourceViewDesc &desc,
			ShaderResourceView **out_resourceView) override;
		
		virtual const CRESULT CreateTexture2D(const Texture2dDesc &desc,
			const SubResourceData *initialData,Ceng::Texture2D **texturePtr) override;

		virtual CRESULT CreateProjectionMatrix(UINT32 displayWidth,
												UINT32 displayHeight,
												FLOAT32 horizontalField,
												FLOAT32 zNear,
												FLOAT32 zFar,
												Matrix4 *dest);

	protected:

		const CRESULT CopyTextureData(CR_NewTargetData *texture, const SubResourceData *sourceData,
			const Ceng::IMAGE_FORMAT::value sourceFormat);

		const CRESULT CopyToTiled(CR_NewTargetData *tiledTex, CR_NewTargetData *sourceTex);

		const Ceng::UINT8* GetTiledAddress(CR_NewTargetData *tiledTexconst, 
			const Ceng::UINT32 u, const Ceng::UINT32 v) const;

	public:
		
		/**
		* Returns the number of bytes required by a variable of CE_VTX_FORMAT.
		*/
		UINT32 GetDeclarationVariableSize(UINT32 format);

		

	};

} // Namespace end

#endif // Include guard
