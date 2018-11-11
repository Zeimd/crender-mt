/*****************************************************************************
*
* pshader-instance.h
*
* Created By Jari Korkala 2/9/2014
*
*****************************************************************************/

#ifndef CRENDER_PSHADER_INSTANCE_H
#define CRENDER_PSHADER_INSTANCE_H

#include <memory>
#include <ceng/datatypes/aligned-buffer.h>

#include "pshader-input-semantic.h"
#include "pshader-input.h"

#include "pshader-output-semantic.h"
#include "pshader-output.h"

#include "rtarget-data.h"

#include "texture-unit.h"

namespace Ceng
{
	class Task_PixelShader;

	class CR_ShaderConstant;


	class CR_PixelShader;

	class CR_PixelShaderTarget;

	class CR_QuadFormat;

	class PixelShaderInstance
	{
	public:

		CR_PixelShader *shader;

		AlignedBuffer<Ceng::UINT8*> uniformPtr;

		AlignedBuffer<Ceng::UINT8> uniformBuffer;

		// Output format

		CR_QuadFormat *quadFormat;	

		AlignedBuffer<CR_PixelShaderInput> inputRegisters;

		UINT32 quadSizeBytes;
		UINT32 quadTargetStart;

		/**
		 * Temporary buffer for current quad.
		 */
		AlignedBuffer<Ceng::UINT8> quadBuffer;

		// w for each pixel in a quad (4xFLOAT or 4xDOUBLE)
		// must be 16-byte aligned
		AlignedBuffer<Ceng::UINT8> perspectiveTemp;

		/**
		 * Pixel coverage mask for render target writes.
		 * Accessed through pointers.
		 */
		POINTER coverageAddress;

		POINTER inputBaseAddress;

		POINTER stepBufferPtr;

		AlignedBuffer<CR_psOutputRegister> outputRegisters;

		UINT32 activeRenderTargets;

		std::shared_ptr<CR_NewTargetData> targetHandles[2+CRENDER_MAX_COLOR_TARGETS];

		std::vector<TextureUnit> textureUnits;


		// Semantic links

		// Input references
		CR_PixelShaderInput *IN_POSITION;
		CR_PixelShaderInput *IN_SCREENPOS;
	
		CR_PixelShaderInput *IN_NORMAL;
		CR_PixelShaderInput *IN_BINORMAL;
		CR_PixelShaderInput *IN_TANGENT;

		CR_PixelShaderInput *IN_COLOR0;
		CR_PixelShaderInput *IN_COLOR1;
	
		CR_PixelShaderInput *IN_TEXCOORD0;
		CR_PixelShaderInput *IN_TEXCOORD1;
		CR_PixelShaderInput *IN_TEXCOORD2;
		CR_PixelShaderInput *IN_TEXCOORD3;
		CR_PixelShaderInput *IN_TEXCOORD4;
		CR_PixelShaderInput *IN_TEXCOORD5;
		CR_PixelShaderInput *IN_TEXCOORD6;
		CR_PixelShaderInput *IN_TEXCOORD7;

		// Output references

		CR_psOutputRegister *OUT_TARGET0;
		CR_psOutputRegister *OUT_TARGET1;
		CR_psOutputRegister *OUT_TARGET2;
		CR_psOutputRegister *OUT_TARGET3;
		CR_psOutputRegister *OUT_TARGET4;
		CR_psOutputRegister *OUT_TARGET5;
		CR_psOutputRegister *OUT_TARGET6;
		CR_psOutputRegister *OUT_TARGET7;

		CR_psOutputRegister *OUT_DEPTH;
		CR_psOutputRegister *OUT_STENCIL;

		// Local variables

		AlignedBuffer<Ceng::UINT8> localVariables;

		Shader::Float4 shaderLocal_temp;
		Shader::Float2 shaderLocal_uvDiffuse;

		Shader::Float shaderLocal_colorScale;

		Shader::SampleTexture2D sample2d;

		TextureUnit diffuseTexUnit;

	public:

		PixelShaderInstance();
		~PixelShaderInstance();

		PixelShaderInstance(CR_PixelShader *shader);

		PixelShaderInstance(const PixelShaderInstance &source);

		const CRESULT ProcessQuads(Task_PixelShader *batch,const Ceng::INT32 threadId);

		const CRESULT ConfigureInput(std::vector<CR_PixelShaderSemantic> &inputSemantics);

		const CRESULT ConfigureOutput(std::vector<CR_PixelShaderTarget> &renderTargets);

		const CRESULT ConfigureUniforms(const std::vector<CR_ShaderConstant> &uniformList,
										const Ceng::UINT32 bufferSize);

		const CRESULT ConfigureLocals();

		const CRESULT SetFragmentFormat(const std::vector<CR_PixelShaderSemantic> &inputSemantics,
										const std::vector<CR_PixelShaderTarget> &targetSemantics);

		const CRESULT SetRenderTargets(const std::vector<CR_PixelShaderTarget> &targetSemantics);


		
		void ShaderFunction(const FLOAT32 *perspective,const FLOAT32 *invertW,
			const Ceng::INT32 coverage, const Ceng::INT32 threadId);
	};
}

#endif