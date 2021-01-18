/*****************************************************************************
*
* cr-vertex-shader.cpp
*
* --------------------------------------------------
* Jari Korkala 8/2014
*
* - Renamed to cr-vertex-shader.cpp
*
* --------------------------------------------------
*
* Created By Jari Korkala 9/2011
*
* Implements CR_VertexShader
*
*
*****************************************************************************/

#include "cr-vertex-shader.h"

#include "vshader-instance.h"

#include "cr-vertex-format.h"

#include "vertex-stream.h"

#include "draw-batch.h"

#include "cr-shader-const.h"

#include <ceng/math/ce-matrix.h>

#include <ceng/lib/liblog.h>

using namespace Ceng;

CR_VertexShader::CR_VertexShader()
{
	cacheLine = 64;

	wrapper.shader = this;

	// Set up a NULL input and output registers 
	// to guard against exceptions from use
	// of undefined semantic links

	nullBuffer = AlignedBuffer<Ceng::UINT8>(64,64);

	//nullInput.sourceFormat = Ceng::SHADER_DATATYPE::UINT;
	nullInput.sourceAddress = (UINT32)&nullBuffer[0];

	nullOutput.destFormat = Ceng::SHADER_DATATYPE::UINT;
	//nullOutput.destAddress = (UINT32)&nullBuffer[0];

	nextInstance = std::shared_ptr<CR_VertexShaderInstance>(new CR_VertexShaderInstance(this));
}

CR_VertexShader::~CR_VertexShader()
{
}

void CR_VertexShader::Release()
{
	delete this;
}

CRESULT CR_VertexShader::GetConstant(const char *variableName,
									 Ceng::ShaderConstant **handle)
{
	size_t k;
	for(k=0;k<uniformList.size();k++)
	{
		if (uniformList[k].name == variableName)
		{
			try
			{
				*handle = (Ceng::ShaderConstant*)
							new CR_ShaderConstHandle(k,&wrapper);
			}
			catch(std::bad_alloc&)
			{
				return CE_ERR_OUT_OF_MEMORY;
			}

			return CE_OK;
		}
	}

	if (k == uniformList.size())
	{
		return CE_ERR_FAIL;
	}

	return CE_OK;
}

Ceng::UINT32 CR_VertexShader::GetDataSize(const Ceng::SHADER_DATATYPE::value datatype)
{
	switch(datatype)
	{
	case Ceng::SHADER_DATATYPE::FLOAT:
		return 4;
	case Ceng::SHADER_DATATYPE::FLOAT2:
		return 8;
	case Ceng::SHADER_DATATYPE::FLOAT3:
	case Ceng::SHADER_DATATYPE::FLOAT4:
		return 16;
	case Ceng::SHADER_DATATYPE::FLOAT4x4:
		return 64;
	}
	return 0;
}

CRESULT CR_VertexShader::ConfigureConstants()
{
	UINT32 k;
	UINT32 currentOffset = 0;

	for(k=0;k<uniformList.size();k++)
	{
		uniformList[k].bufferOffset = currentOffset;

		uniformList[k].size = GetDataSize(uniformList[k].dataType);
		currentOffset += GetDataSize(uniformList[k].dataType);
	}

	uniformBufferSize = currentOffset;

	return nextInstance->ConfigureUniforms(uniformList,currentOffset);
}

CRESULT CR_VertexShader::SetFragmentFormat(CR_FragmentFormat *format)
{
	nextInstance->fragmentFormat = format;
	nextInstance->fragmentSizeBytes = format->size;

	

	return CE_OK;
}

CRESULT CR_VertexShader::ConfigureInput()
{
	inputFlags = 0;

	if (inputSemantics.size() == 0)
	{
		return CE_ERR_INCOMPATIBLE_FORMAT;
	}

	Ceng::UINT32 k;

	for(k=0;k<inputSemantics.size();k++)
	{
		// Mark semantic as required
		inputFlags |= inputSemantics[k].semantic;
	}

	return CE_OK;
}

CRESULT CR_VertexShader::SetVertexFormat(CR_VertexFormat *vertexDecl)
{
	if (vertexDecl == nullptr)
	{
		return CE_ERR_NULL_PTR;
	}

	if ((inputFlags & vertexDecl->semanticFlags) != inputFlags)
	{
		return CE_ERR_INCOMPATIBLE_FORMAT;
	}

	nextInstance->vertexFormat = vertexDecl;

	return CE_OK;

	// Link each register to the variable with matching semantic
	// NOTE: multiple shader inputs can be linked to same input variable

	

	/*
	Log::Print("\nVertexShader.SetVertexFormat: input indices\n");

	String temp;

	for(k=0;k<inputSemantics.size();k++)
	{
		switch(inputSemantics[k].semantic)
		{
		case SHADER_SEMANTIC::POSITION:
			temp = "position = ";
			break;
		case SHADER_SEMANTIC::NORMAL:
			temp = "normal = ";
			break;
		case SHADER_SEMANTIC::COLOR_0:
			temp = "color0 = ";
			break;
		case SHADER_SEMANTIC::TEXCOORD_0:
			temp = "texCoord0 = ";
			break;
		case SHADER_SEMANTIC::TEXCOORD_1:
			temp = "texCoord1 = ";
			break;
		case SHADER_SEMANTIC::TEXCOORD_2:
			temp = "texCoord2 = ";
			break;
		default:
			break;
		}

		temp += inputSemantics[k].sourceIndex;

		Log::Print(temp);
	}
	*/

	return CE_OK;
}

CRESULT CR_VertexShader::SetVertexStreams(Ceng::UINT32 streamCount,
										  VertexStreamData *streamList)
{
	
	if (streamList == nullptr)
	{
		return CE_ERR_NULL_PTR;
	}

	nextInstance->streamCount = streamCount;
	nextInstance->vertexStreams = streamList;

	
	/*
	Log::Print("\nVertexShader.SetVertexStreams: input streams\n");

	String temp;

	for(k=0;k<CRENDER_MAX_VERTEX_STREAMS;k++)
	{
		temp = "stream[";
		temp += k;
		temp += "] = ";
		temp += streamList[k].elementCount;
		temp += ",";
		temp += streamList[k].elementSize;
		temp += ", base address = ";
		temp += (POINTER)streamList[k].inputPtr;

		Log::Print(temp);
	}

	Log::Print("\nVertexShader.SetVertexStreams: semantic source address\n");
	
	for(k=0;k<inputSemantics.size();k++)
	{
		switch(inputSemantics[k].semantic)
		{
		case SHADER_SEMANTIC::POSITION:
			temp = "position = ";
			break;
		case SHADER_SEMANTIC::NORMAL:
			temp = "normal = ";
			break;
		case SHADER_SEMANTIC::COLOR_0:
			temp = "color0 = ";
			break;
		case SHADER_SEMANTIC::TEXCOORD_0:
			temp = "texCoord0 = ";
			break;
		case SHADER_SEMANTIC::TEXCOORD_1:
			temp = "texCoord1 = ";
			break;
		case SHADER_SEMANTIC::TEXCOORD_2:
			temp = "texCoord2 = ";
			break;
		default:
			break;
		}

		temp += "stream[";
		temp += vertexFormat->variables[j].inputStream;
		temp += "]";

		Log::Print(temp);
	}
	*/

	return CE_OK;
}

const CRESULT CR_VertexShader::GetInstance(std::shared_ptr<CR_VertexShaderInstance> &instance)
{
	// Create an instance using *nextState*

	currentInstance = nextInstance;

	nextInstance = std::shared_ptr<CR_VertexShaderInstance>(new CR_VertexShaderInstance(*currentInstance));

	currentInstance->ConfigureInput(inputSemantics);

	currentInstance->SetVertexFormat(inputSemantics);
	currentInstance->SetVertexStreams();

	currentInstance->SetFragmentFormat();	
	
	currentInstance->ConfigureUniforms(uniformList,uniformBufferSize);
	
	instance = currentInstance;	

	return CE_OK;
}

const CRESULT CR_VertexShader::ReadUniform(const Ceng::UINT32 index,void *destBuffer)
{
	memcpy(destBuffer,nextInstance->uniformPtr[index],uniformList[index].size);

	return CE_OK;
}


const CRESULT CR_VertexShader::WriteUniform(const Ceng::UINT32 index,void *sourceBuffer)
{
	memcpy(nextInstance->uniformPtr[index],sourceBuffer,uniformList[index].size);

	return CE_OK;
}
