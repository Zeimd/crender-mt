/*****************************************************************************
*
* cr-shader-link.h
*
* By Jari Korkala 5/2013
*
*****************************************************************************/

#ifndef _CENG_CR_SHADER_LINK_H
#define _CENG_CR_SHADER_LINK_H

#include <ceng/datatypes/aligned-buffer.h>

#include "quad-format.h"

#include "fragment-format.h"

#include <ceng/lib/liblog.h>

namespace Ceng
{
	class CR_VertexShader;
	class CR_PixelShader;

	class CR_ShaderLink
	{
	public:

		CR_VertexShader *vertexShader;
		CR_PixelShader *pixelShader;

		CR_FragmentFormat fragmentFormat;
		CR_QuadFormat quadFormat;

		Ceng::UINT32 fragmentCacheSize;

		Ceng::UINT32 shaderStepBufferSize;

		Ceng::UINT32 clipperCacheSize;

		Ceng::UINT32 quadCacheSize;

		UINT32 maxViewWidth;
		UINT32 maxViewHeight;

		Ceng::UINT32 cacheLineSize;

	public:

		CR_ShaderLink();

		CR_ShaderLink(const Ceng::UINT32 cacheLineSize);
		~CR_ShaderLink();

		CRESULT SetViewSize(const UINT32 maxViewWidth,const UINT32 maxViewHeight);

		CRESULT Configure(CR_VertexShader *vertexShader,CR_PixelShader *pixelShader);

		CRESULT SetRenderTargets(const Ceng::UINT32 targets);

		AlignedBuffer<Ceng::UINT8>*	GetFragmentCache(const Ceng::UINT32 batchSize);

		AlignedBuffer<CR_FloatFragment>* GetFloatVariableBuffer();

		AlignedBuffer<CR_DoubleFragment>* GetDoubleVariableBuffer();

		AlignedBuffer<Ceng::UINT8>* GetShaderStepBuffer();

		AlignedBuffer<CR_QuadHeader>* GetQuadCache(const Ceng::UINT32 maxQuads);

		AlignedBuffer<Ceng::UINT8>* GetClipperCache(const Ceng::UINT32 batchSize);

	protected:

		void Clear();
		CRESULT SyncQuadCache();
	};
};

#endif