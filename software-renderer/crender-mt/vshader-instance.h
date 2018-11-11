/*****************************************************************************
*
* vshader-instance.h
*
* By Jari Korkala 8/2014
*
*****************************************************************************/

#ifndef CRENDER_VSHADER_INSTANCE_H
#define CRENDER_VSHADER_INSTANCE_H

#include <memory>
#include <ceng/datatypes/aligned-buffer.h>
#include <ceng/datatypes/ring-buffer.h>

#include "vshader-input.h"
#include "vshader-output.h"

#include "shader-const-data.h"

namespace Ceng
{
	class DrawBatch;

	class CR_vsInputSemantic;

	class CR_VertexShader;

	class CR_VertexFormat;

	class CR_FragmentFormat;

	class VertexStreamData;

	class LockingStage;

	class CR_VertexShaderInstance
	{
	public:

		CR_VertexShader *shader;

		AlignedBuffer<Ceng::UINT8*> uniformPtr;

		AlignedBuffer<Ceng::UINT8> uniformBuffer;

		//************************************
		// Input state

		/**
		 * Pointer to vertex declaration.
		 */
		CR_VertexFormat *vertexFormat;

		/**
		 * Which vertex format variable each inputRegister corresponds to.
		 */
		AlignedBuffer<Ceng::UINT32> sourceIndex;

		/**
		 * Semantic links to vertex buffer data. Used
		 * in ShaderFunction().
		 */
		AlignedBuffer<CR_vsInputRegister> inputRegisters;

		/**
		 * Correct stride (stream specific) for each
		 * input register.
		 */
		AlignedBuffer<POINTER> inputSteps;

		/**
		 * Stream specific base address for each
		 * input register.
		 */
		AlignedBuffer<POINTER> inputBaseAddress;

		Ceng::UINT32 streamCount;

		VertexStreamData *vertexStreams;

		//************************************
		// Output state

		/**
		 * List of output semantics written by vertex shader AND consumed
		 * by pixel shader.
		 */
		CR_FragmentFormat *fragmentFormat;

		/**
		 * Fragment buffer address step per vertex.
		 */
		Ceng::UINT32 fragmentSizeBytes;

		/**
		 * Semantic links to fragment output buffer.
		 * Used in ShaderFunction().
		 */
		AlignedBuffer<CR_vsOutputRegister> outputRegisters;

		/**
		 * Pointer to current output fragment. Output registers
		 * access this value through pointers.
		 */
		POINTER outputBaseAddress;

		// Input references
		CR_vsInputRegister *IN_POSITION;
	
		CR_vsInputRegister *IN_NORMAL;
		CR_vsInputRegister *IN_BINORMAL;
		CR_vsInputRegister *IN_TANGENT;

		CR_vsInputRegister *IN_COLOR0;
		CR_vsInputRegister *IN_COLOR1;
	
		CR_vsInputRegister *IN_TEXCOORD0;
		CR_vsInputRegister *IN_TEXCOORD1;
		CR_vsInputRegister *IN_TEXCOORD2;
		CR_vsInputRegister *IN_TEXCOORD3;
		CR_vsInputRegister *IN_TEXCOORD4;
		CR_vsInputRegister *IN_TEXCOORD5;
		CR_vsInputRegister *IN_TEXCOORD6;
		CR_vsInputRegister *IN_TEXCOORD7;

		// Output references
		CR_vsOutputRegister *OUT_POSITION;

		CR_vsOutputRegister *OUT_NORMAL;
		CR_vsOutputRegister *OUT_BINORMAL;
		CR_vsOutputRegister *OUT_TANGENT;

		CR_vsOutputRegister *OUT_COLOR0;
		CR_vsOutputRegister *OUT_COLOR1;

		CR_vsOutputRegister *OUT_TEXCOORD0;
		CR_vsOutputRegister *OUT_TEXCOORD1;
		CR_vsOutputRegister *OUT_TEXCOORD2;
		CR_vsOutputRegister *OUT_TEXCOORD3;
		CR_vsOutputRegister *OUT_TEXCOORD4;
		CR_vsOutputRegister *OUT_TEXCOORD5;
		CR_vsOutputRegister *OUT_TEXCOORD6;
		CR_vsOutputRegister *OUT_TEXCOORD7;		

	public:

		CR_VertexShaderInstance();

		CR_VertexShaderInstance(CR_VertexShader *shader);

		CR_VertexShaderInstance(const CR_VertexShaderInstance &source);

		virtual ~CR_VertexShaderInstance();

		const CRESULT ConfigureInput(const std::vector<CR_vsInputSemantic> &inputSemantics);
		
		const CRESULT SetFragmentFormat();

		const CRESULT SetVertexFormat(const std::vector<CR_vsInputSemantic> &inputSemantics);

		const CRESULT SetVertexStreams();

		const CRESULT ConfigureUniforms(const std::vector<CR_ShaderConstant> &uniformList,
										const Ceng::UINT32 bufferSize);

		virtual const CRESULT ProcessVertexBatch(std::shared_ptr<DrawBatch> batch,
												LockingStage *outputQueue);

		virtual void ShaderFunction();


	};
}

#endif