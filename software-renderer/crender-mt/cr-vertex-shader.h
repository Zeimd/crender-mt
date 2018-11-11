/*****************************************************************************
*
* cr-vshader.h
*
* By Jari Korkala 2/2012
*
* Vertex shader declarations moved here from "crender.h"
*
*****************************************************************************/

#ifndef _CENG_CR_VSHADER_H
#define _CENG_CR_VSHADER_H

#include <memory>
#include <vector>

#include <ceng/interfaces/vertex-shader.h>

#include <ceng/datatypes/aligned-buffer.h>

// Vertex shader interface declaration
#include "crender-base.h"

#include "fragment-var.h"
#include "fragment-format.h"

#include "vshader-input.h"
#include "vshader-output.h"

#include "vshader-input-semantic.h"

#include "pshader-input.h"

#include "vshader-instance.h"

#include "shader-const-data.h"

#include "vshader-wrapper.h"

namespace Ceng
{
	//****************************************************************************
	// Vertex shader interface

	class VertexStreamData;

	class CR_VertexFormat;

	class CR_FragmentFormat;

	class DrawBatch;

	class CR_VertexShaderInstance;

	class ShaderConstant;

	class CR_VertexShader : public Ceng::VertexShader
	{
	public:

		VertexShaderWrapper wrapper;

		std::vector<CR_ShaderConstant> uniformList;

		Ceng::UINT32 uniformBufferSize;

		Ceng::UINT32 cacheLine;

		//UINT32 vertexSizeBytes;
	
		/**
		 * List of input semantics the shader uses.
		 */
		std::vector<CR_vsInputSemantic> inputSemantics;

		/**
		 * List of output semantics the shader can use.
		 */
		std::vector<CR_vsOutputSemantic> outputSemantics;

		/**
		 * Used to temporarily store semantics that appear both in vertex shader
		 * output and pixel shader input. POSITION-semantic excluded.
		 */
		std::vector<CR_vsOutputSemantic> linkedOutput;

		std::shared_ptr<CR_VertexShaderInstance> nextInstance;
		std::shared_ptr<CR_VertexShaderInstance> currentInstance;

		/**
		 * Flags for input semantics the shader uses.
		 */
		UINT32 inputFlags;

		CR_vsInputRegister nullInput;
		CR_vsOutputRegister nullOutput;

	protected:

		AlignedBuffer<Ceng::UINT8> nullBuffer;

	public:
	
		CR_VertexShader();
		virtual ~CR_VertexShader();

		virtual void Release() override;

		virtual CRESULT GetConstant(const char *variableName,
										Ceng::ShaderConstant **handle);

		const CRESULT ReadUniform(const Ceng::UINT32 index,void *destBuffer);

		const CRESULT WriteUniform(const Ceng::UINT32 index,void *sourceBuffer);

		CRESULT SetVertexFormat(CR_VertexFormat *format);

		CRESULT SetVertexStreams(UINT32 streamCount,VertexStreamData *streamList);

		CRESULT SetFragmentFormat(CR_FragmentFormat *format);

		UINT32 GetDataSize(const Ceng::SHADER_DATATYPE::value datatype);

		const CRESULT GetInstance(std::shared_ptr<CR_VertexShaderInstance> &instance);

		CRESULT ConfigureConstants();
		CRESULT ConfigureInput();
	
		CRESULT ConfigureOutput();

	protected:

	};

} // Namespace end

#endif // Include guard