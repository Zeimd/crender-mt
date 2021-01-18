/*****************************************************************************
*
* vshader-instance.cpp
*
* Created By Jari Korkala 8/2014
*
* - CR_VertexShader split into static and state-dependent parts.
*
*****************************************************************************/

#include "vshader-instance.h"

#include "cr-vertex-format.h"

#include "vshader-input.h"
#include "vshader-output.h"

#include "draw-batch.h"

#include "fragment-format.h"

#include "vshader-input-semantic.h"

#include "cr-vertex-shader.h"

#include <ceng/math/ce-matrix.h>

#include "clipper-batch.h"
#include "task-clipper.h"

#include "locking-stage.h"

using namespace Ceng;

CR_VertexShaderInstance::CR_VertexShaderInstance()
{
	vertexFormat = nullptr;
	fragmentFormat = nullptr;

	fragmentSizeBytes = 0;
}

CR_VertexShaderInstance::CR_VertexShaderInstance(CR_VertexShader *shader)
{
	vertexFormat = nullptr;
	fragmentFormat = nullptr;

	fragmentSizeBytes = 0;

	this->shader = shader;

	IN_POSITION = &shader->nullInput;

	IN_NORMAL = &shader->nullInput;
	IN_TANGENT = &shader->nullInput;
	IN_BINORMAL = &shader->nullInput;

	IN_COLOR0 = &shader->nullInput;
	IN_COLOR1 = &shader->nullInput;

	IN_TEXCOORD0 = &shader->nullInput;
	IN_TEXCOORD1 = &shader->nullInput;
	IN_TEXCOORD2 = &shader->nullInput;
	IN_TEXCOORD3 = &shader->nullInput;
	IN_TEXCOORD4 = &shader->nullInput;
	IN_TEXCOORD5 = &shader->nullInput;
	IN_TEXCOORD6 = &shader->nullInput;
	IN_TEXCOORD7 = &shader->nullInput;

	OUT_POSITION = &shader->nullOutput;

	OUT_NORMAL = &shader->nullOutput;
	OUT_BINORMAL = &shader->nullOutput;
	OUT_TANGENT = &shader->nullOutput;

	OUT_COLOR0 = &shader->nullOutput;
	OUT_COLOR1 = &shader->nullOutput;

	OUT_TEXCOORD0 = &shader->nullOutput;
	OUT_TEXCOORD1 = &shader->nullOutput;
	OUT_TEXCOORD2 = &shader->nullOutput;
	OUT_TEXCOORD3 = &shader->nullOutput;
	OUT_TEXCOORD4 = &shader->nullOutput;
	OUT_TEXCOORD5 = &shader->nullOutput;
	OUT_TEXCOORD6 = &shader->nullOutput;
	OUT_TEXCOORD7 = &shader->nullOutput;
	
}

CR_VertexShaderInstance::~CR_VertexShaderInstance()
{
}

CR_VertexShaderInstance::CR_VertexShaderInstance(const CR_VertexShaderInstance &source)
{
	this->shader = source.shader;

	// Uniform data

	uniformPtr = AlignedBuffer<Ceng::UINT8*>(shader->uniformList.size(),shader->cacheLine);
	uniformBuffer = source.uniformBuffer;

	// Write correct addresses to uniformPtr
	ConfigureUniforms(shader->uniformList,shader->uniformBufferSize);

	// Input state

	

	vertexFormat = source.vertexFormat;

	// These four are currently reallocated each time instance is created
	sourceIndex = source.sourceIndex; 
	inputRegisters = source.inputRegisters;
	inputBaseAddress = source.inputBaseAddress;
	inputSteps = source.inputSteps;	

	streamCount = source.streamCount;
	vertexStreams = source.vertexStreams;

	// Output state

	fragmentFormat = source.fragmentFormat;

	fragmentSizeBytes = source.fragmentSizeBytes;

	// Currently reallocated when instance is created
	outputRegisters = source.outputRegisters;

	outputBaseAddress = source.outputBaseAddress;

	// Semantic links

	IN_POSITION = source.IN_POSITION;

	IN_NORMAL = source.IN_NORMAL;
	IN_TANGENT = source.IN_TANGENT;
	IN_BINORMAL = source.IN_BINORMAL;

	IN_COLOR0 = source.IN_COLOR0;
	IN_COLOR1 = source.IN_COLOR1;

	IN_TEXCOORD0 = source.IN_TEXCOORD0;
	IN_TEXCOORD1 = source.IN_TEXCOORD1;
	IN_TEXCOORD2 = source.IN_TEXCOORD2;
	IN_TEXCOORD3 = source.IN_TEXCOORD3;
	IN_TEXCOORD4 = source.IN_TEXCOORD4;
	IN_TEXCOORD5 = source.IN_TEXCOORD5;
	IN_TEXCOORD6 = source.IN_TEXCOORD6;
	IN_TEXCOORD7 = source.IN_TEXCOORD7;

	OUT_POSITION = source.OUT_POSITION;

	OUT_NORMAL = source.OUT_NORMAL;
	OUT_BINORMAL = source.OUT_BINORMAL;
	OUT_TANGENT = source.OUT_TANGENT;

	OUT_COLOR0 = source.OUT_COLOR0;
	OUT_COLOR1 = source.OUT_COLOR1;

	OUT_TEXCOORD0 = source.OUT_TEXCOORD0;
	OUT_TEXCOORD1 = source.OUT_TEXCOORD0;
	OUT_TEXCOORD2 = source.OUT_TEXCOORD0;
	OUT_TEXCOORD3 = source.OUT_TEXCOORD0;
	OUT_TEXCOORD4 = source.OUT_TEXCOORD0;
	OUT_TEXCOORD5 = source.OUT_TEXCOORD0;
	OUT_TEXCOORD6 = source.OUT_TEXCOORD0;
	OUT_TEXCOORD7 = source.OUT_TEXCOORD0;
}

const CRESULT CR_VertexShaderInstance::ConfigureInput(const std::vector<CR_vsInputSemantic> &inputSemantics)
{
	// TODO: Provide as input parameter

	UINT32 k;

	sourceIndex = AlignedBuffer<Ceng::UINT32>(inputSemantics.size(),shader->cacheLine);

	inputRegisters = AlignedBuffer<CR_vsInputRegister>(inputSemantics.size(),shader->cacheLine);

	inputSteps = AlignedBuffer<Ceng::POINTER>(inputSemantics.size(),shader->cacheLine);

	inputBaseAddress = AlignedBuffer<Ceng::POINTER>(inputSemantics.size(),shader->cacheLine);

	// Set up references to input variables
	for(k=0;k<inputSemantics.size();k++)
	{
	
		switch(inputSemantics[k].semantic)
		{
		case Ceng::SHADER_SEMANTIC::POSITION:
			IN_POSITION = &inputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::NORMAL:
			IN_NORMAL = &inputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::BINORMAL:
			IN_BINORMAL = &inputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TANGENT:
			IN_TANGENT = &inputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::COLOR_0:
			IN_COLOR0 = &inputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::COLOR_1:
			IN_COLOR1 = &inputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_0:
			IN_TEXCOORD0 = &inputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_1:
			IN_TEXCOORD1 = &inputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_2:
			IN_TEXCOORD2 = &inputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_3:
			IN_TEXCOORD3 = &inputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_4:
			IN_TEXCOORD4 = &inputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_5:
			IN_TEXCOORD5 = &inputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_6:
			IN_TEXCOORD6 = &inputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_7:
			IN_TEXCOORD7 = &inputRegisters[k];
			break;
		default:
			break;
		}
	}
	
	return CE_OK;
}

const CRESULT CR_VertexShaderInstance::SetFragmentFormat()
{
	// Set up references to output blocks

	outputRegisters = AlignedBuffer<CR_vsOutputRegister>(
		fragmentFormat->variables.size(),shader->cacheLine);

	for(size_t k=0;k<fragmentFormat->variables.size();k++)
	{
		outputRegisters[k].destAddress = &outputBaseAddress;
		outputRegisters[k].destFormat = fragmentFormat->variables[k].format;
		outputRegisters[k].destOffset = fragmentFormat->variables[k].offset;
	}

	for(size_t k=0;k<fragmentFormat->variables.size();k++)
	{
		switch(fragmentFormat->variables[k].semantic)
		{
		case Ceng::SHADER_SEMANTIC::POSITION:
			OUT_POSITION = &outputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::NORMAL:
			OUT_NORMAL = &outputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::BINORMAL:
			OUT_BINORMAL = &outputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TANGENT:
			OUT_TANGENT = &outputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::COLOR_0:
			OUT_COLOR0 = &outputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::COLOR_1:
			OUT_COLOR1 = &outputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_0:
			OUT_TEXCOORD0 = &outputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_1:
			OUT_TEXCOORD1 = &outputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_2:
			OUT_TEXCOORD2 = &outputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_3:
			OUT_TEXCOORD3 = &outputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_4:
			OUT_TEXCOORD4 = &outputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_5:
			OUT_TEXCOORD5 = &outputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_6:
			OUT_TEXCOORD6 = &outputRegisters[k];
			break;
		case Ceng::SHADER_SEMANTIC::TEXCOORD_7:
			OUT_TEXCOORD7 = &outputRegisters[k];
			break;
		default:
			break;
		}
	}

	return CE_OK;
}

const CRESULT CR_VertexShaderInstance::SetVertexFormat(const std::vector<CR_vsInputSemantic> &inputSemantics)
{
	Ceng::UINT32 k,j;

	for(k=0;k<inputSemantics.size();k++)
	{
		sourceIndex[k] = -1;

		for(j=0;j<vertexFormat->variables.size();j++)
		{		
			if (inputSemantics[k].semantic == vertexFormat->variables[j].semantic)
			{
				sourceIndex[k] = j;

				inputRegisters[k].sourceFormat = vertexFormat->variables[j].dataType;
				break;
			}
		}
	}

	return CE_OK;
}

const CRESULT CR_VertexShaderInstance::SetVertexStreams()
{
	UINT32 k,j;

	// Update stream base addresses
	for(k=0;k< inputRegisters.GetElements();k++)
	{
		j = sourceIndex[k];

		if (j == -1)
		{
			//Log::Print("SetVertexStreams : Error : called before SetVertexFormat");
			return CE_ERR_FAIL;
		}

		inputBaseAddress[k] = vertexFormat->variables[j].inputOffset +
			UINT32(vertexStreams[vertexFormat->variables[j].inputStream].inputPtr);

		inputSteps[k] = vertexStreams[vertexFormat->variables[j].inputStream].elementSize;
	}

	return CE_OK;
}

const CRESULT CR_VertexShaderInstance::ConfigureUniforms(const std::vector<CR_ShaderConstant> &uniformList,
														 const Ceng::UINT32 bufferSize)
{

	if (uniformBuffer == nullptr)
	{
		uniformBuffer = AlignedBuffer<UINT8>(bufferSize,shader->cacheLine);
	}

	if (uniformPtr == nullptr)
	{
		uniformPtr = AlignedBuffer<UINT8*>(uniformList.size(),shader->cacheLine);
	}

	Ceng::UINT32 k;

	for(k=0;k<uniformList.size();k++)
	{
		uniformPtr[k] = &uniformBuffer[uniformList[k].bufferOffset];
	}

	return CE_OK;
}


const CRESULT CR_VertexShaderInstance::ProcessVertexBatch(std::shared_ptr<DrawBatch> batch,
														  LockingStage *outputQueue)
{	
//	return CE_OK;

	INT32 inputCount = inputRegisters.GetElements();
	INT32 outputCount = fragmentFormat->variables.size();

	outputBaseAddress = (POINTER)& (*(batch->fragmentCache))[0];

	for(Ceng::UINT32 k=0;k<batch->vertexCount;k++)
	{
		Ceng::UINT32 index = batch->fragmentIndex[k].tag;

		// Move input pointers to correct position

		for(Ceng::INT32 i=0;i<inputCount;i++)
		{
			inputRegisters[i].sourceAddress = inputBaseAddress[i]
				+ index*inputSteps[i];
		}

		ShaderFunction();

		// Step register addresses

		/*
		for(i=0;i<inputCount;i++)
		{
			inputRegisters[i].sourceAddress += inputSteps[i];
		}
		*/
		outputBaseAddress += fragmentSizeBytes;
	}

	auto out_batch = std::shared_ptr<ClipperBatch>(new ClipperBatch(batch));

	auto out_task = std::shared_ptr<LockingTask>(new Task_Clipper(out_batch));

	outputQueue->AddTask(0,out_task);

	return CE_OK;
}

void CR_VertexShaderInstance::ShaderFunction()
{

	// ***** Constant setup

	const Matrix4 *transform = (Matrix4*)uniformPtr[0];

	// ***** Local variables

	_declspec(align(16)) VectorF4 temp;
	_declspec(align(16)) VectorF4 positionTemp;

	// NOTE: Use IN_SEMANTIC pointers to obtain
	//       correct variable from input stream

	positionTemp = *IN_POSITION;
	positionTemp.w = FLOAT32(1.0f);
	
	positionTemp = (*transform) * positionTemp;
	
	*OUT_POSITION = positionTemp;

	// NOTE: Can't transfer directly from input to output
	//       because the temporary Vector4 won't be
	//       aligned in memory

	/*
	temp = *IN_NORMAL;
	*OUT_NORMAL = temp;
	
	
	temp = *IN_COLOR0;
	*OUT_COLOR0 = temp;
	*/
	
	_declspec(align(16)) VectorF2 v2Temp;

	v2Temp = *IN_TEXCOORD0;
	*OUT_TEXCOORD0 = v2Temp;
	
	
	v2Temp = *IN_TEXCOORD1;
	*OUT_TEXCOORD1 = v2Temp;

	/*
	tempFloat = *IN_TEXCOORD2;
	*OUT_TEXCOORD2 = tempFloat;
	*/
}


