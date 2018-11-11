/*****************************************************************************
*
* render-state.cpp
*
* By Jari Korkala 8/2014
*
*****************************************************************************/

#include "render-state.h"

#include "cr-vertex-shader.h"
#include "cr-vertex-format.h"

#include "cr-pixel-shader.h"

#include "shader-link.h"

#include "rtarget-data.h"

#include "clipper.h"

using namespace Ceng;

RenderState::RenderState(const Ceng::UINT32 maxVertexStreams,const Ceng::UINT32 maxTextureUnits)
{
	rasterizer = nullptr;
	clipper = nullptr;

	vertexShader = nullptr;

	vertexFormat = nullptr;

	this->maxVertexStreams = maxVertexStreams;

	vertexStreams = (VertexStreamData*)malloc(sizeof(VertexStreamData)*maxVertexStreams);

	memset(vertexStreams,0,
			sizeof(VertexStreamData)*maxVertexStreams);

	textureUnits = std::vector<TextureUnit>(maxTextureUnits);

	Ceng::UINT32 k;

	for(k=0;k<2+CRENDER_MAX_COLOR_TARGETS;k++)
	{
		renderTargets[k] = nullptr;
	}

	activeTargets = 0;

	pixelShader = nullptr;
}

RenderState::RenderState(const RenderState &source)
{
	rasterizer = source.rasterizer;
	clipper = source.clipper;

	vertexShader = source.vertexShader;

	vshaderInstance = source.vshaderInstance;

	pshaderInstance = source.pshaderInstance;

	vertexFormat = source.vertexFormat;

	maxVertexStreams = source.maxVertexStreams;

	scissorRect = source.scissorRect;

	clipperState = source.clipperState;

	viewportState = source.viewportState;

	vertexStreams = (VertexStreamData*)malloc(sizeof(VertexStreamData)*maxVertexStreams);

	memcpy(vertexStreams,source.vertexStreams,sizeof(VertexStreamData)*source.maxVertexStreams);

	textureUnits = source.textureUnits;

	shaderLink = source.shaderLink;

	pixelShader = source.pixelShader;

	depthStencilState = source.depthStencilState;

	rasterizerState = source.rasterizerState;

	activeTargets = source.activeTargets;

	Ceng::UINT32 k;

	for(k=0;k<2+CRENDER_MAX_COLOR_TARGETS;k++)
	{
		renderTargets[k] = source.renderTargets[k];
	}
}

RenderState::~RenderState()
{
	free(vertexStreams);
}

const CRESULT RenderState::CommitState(const RenderState &oldState,CR_ShaderLink *shaderLink,
										const Ceng::UINT32 cacheLineSize,
										const Ceng::UINT32 renderThreads)
{
	CRESULT cresult;

	if (depthBuffer != nullptr)
	{
		depthBuffer->SetDepthStencilState(&depthStencilState);
	}

	this->shaderLink = shaderLink;

	cresult = shaderLink->Configure(vertexShader,pixelShader);
	if (cresult != CE_OK)
	{
		//Ceng::Log::Print("SetVertexShader : Shader link config failed");
		return cresult;
	}

	shaderLink->SetRenderTargets(activeTargets);

	if (vertexShader != nullptr)
	{
		if (vertexFormat != nullptr)
		{
			cresult = vertexShader->SetVertexFormat(vertexFormat);
			if (cresult != CE_OK)
			{
				//Ceng::Log::Print("SetVertexShader : Vertex format sync failed");
				return cresult;
			}
		}

		cresult = vertexShader->SetVertexStreams(maxVertexStreams,&vertexStreams[0]);
		if (cresult != CE_OK)
		{
			//Ceng::Log::Print("SetVertexShader : Vertex stream sync failed");
			return cresult;
		}

		cresult = vertexShader->GetInstance(vshaderInstance);

		if (cresult != CE_OK)
		{
			return cresult;
		}
	}

	if (pixelShader != nullptr)
	{
		cresult = pixelShader->SetRenderTargets(activeTargets,renderTargets,depthBuffer);

		if (cresult != CE_OK)
		{
			return cresult;
		}

		cresult = pixelShader->SetTextureUnits(textureUnits);

		if (cresult != CE_OK)
		{
			return cresult;
		}

		cresult = pixelShader->GetInstances(pshaderInstance,renderThreads);

		if (cresult != CE_OK)
		{
			return cresult;
		}
	}

	if (rasterizerState.scissorEnable)
	{	
		activeRect.top = scissorRect.top;
		activeRect.left = scissorRect.left;
		
		activeRect.bottom = scissorRect.bottom;
		activeRect.right = scissorRect.right;
	}
	else
	{
		activeRect.top = 0;
		activeRect.left = 0;

		activeRect.right = viewportState.windowWidth;
		activeRect.bottom = viewportState.windowHeight;
	}

	return CE_OK;
}