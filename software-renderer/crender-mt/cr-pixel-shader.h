/*****************************************************************************
*
* cr-pshader.h
*
* By Jari Korkala 2/2012
*
* Pixel shader declarations moved here form "crender.h"
*
* Update 4/2013 : Moved PixelShaderInput and PixelShaderTarget to
*                 separate files.
*
*****************************************************************************/

#ifndef _CENG_CR_PSHADER_H
#define _CENG_CR_PSHADER_H

#include <memory>

#include <ceng/interfaces/pixel-shader.h>

#include "crender-base.h"

#include "shader-datatype.h"

#include "triangle-data.h"

#include "cr-shader-const.h"

#include "pshader-input-semantic.h"
#include "pshader-input.h"

#include "pshader-output-semantic.h"
#include "pshader-output.h"

#include "shader-common.h"

#include "pshader-instance.h"

#include "pshader-wrapper.h"

#include "texture-unit.h"

namespace Ceng
{
	class CR_NewTargetData;

	
	struct CR_QuadHeader;

	class CR_PixelShader : public Ceng::PixelShader
	{
	public:

		Ceng::UINT32 cacheLine;

		PixelShaderWrapper wrapper;

		std::shared_ptr<PixelShaderInstance> nextInstance;
		std::shared_ptr<PixelShaderInstance> currentInstance;

		std::vector<CR_PixelShaderSemantic> inputSemantics;

		std::vector<CR_PixelShaderTarget> renderTargets;

		CR_NewTargetData *targetHandle[2+8];

		std::vector<CR_ShaderConstant> uniformList;

		Ceng::UINT32 uniformBufferSize;

		UINT32 outputFlags;

		UINT32 disableStencil;
		UINT32 disableDepth;

		CR_PixelShaderInput nullInput;
		CR_psOutputRegister nullOutput;

	protected:

	

	public:
		CR_PixelShader();
		virtual ~CR_PixelShader();

		virtual void Release() override;

		CRESULT ConfigureConstants();

		virtual CRESULT GetConstant(const char *variableName,
										Ceng::ShaderConstant **handle);

		const CRESULT ReadUniform(const Ceng::UINT32 index,void *destBuffer);

		const CRESULT WriteUniform(const Ceng::UINT32 index,void *sourceBuffer);

		CRESULT SetFragmentFormat(CR_QuadFormat *newQuadFormat);

		const CRESULT SetTextureUnits(std::vector<TextureUnit> &source);

		CRESULT SetRenderTargets(UINT32 amount,std::shared_ptr<CR_NewTargetData> targets[],
									std::shared_ptr<CR_NewTargetData> &depthStencil);

		CRESULT SetGradients(CR_FloatFragment *floatVariable,CR_DoubleFragment *doubleVariable,
								UINT8 *variableStep);

		const CRESULT GetInstances(std::vector<std::shared_ptr<PixelShaderInstance>> &instances,
									const Ceng::UINT32 renderThreads);

	};

}; // namespace Ceng

#endif // Include guard