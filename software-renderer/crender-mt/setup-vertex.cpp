/*****************************************************************************
*
* setup-vertex.cpp
*
* By Jari Korkala 8/2014
*
*****************************************************************************/

#include "crender.h"

#include "cr-vertex-format.h"

#include "cr-vertex-buffer.h"

#include "cr-vertex-shader.h"

using namespace Ceng;

CRESULT ConceptRenderer::CreateVertexFormat(const std::vector<Ceng::VertexDeclData> &vertexDecl,
												Ceng::VertexFormat **vertexFormat)
{	
	*vertexFormat = nullptr;

	Log::Print("CreateVertexFormat : method start");

	if (&vertexDecl == nullptr)
	{
		Log::Print("CreateVertexFormat : nullptr input");
		return CE_ERR_NULL_PTR;
	}

	
	UINT32 size = 0;
	INT32 k = 0;

	if (vertexDecl.size() == 0)
	{
		Log::Print("CreateVertexFormat : vertex declaration is empty");
		return CE_ERR_INVALID_PARAM;
	}

	// Count variables, but interrupt immediately if
	// an invalid value is encountered

	for(k=0;k<vertexDecl.size();k++)
	{
		if (vertexDecl[k].semantic == Ceng::SHADER_SEMANTIC::FORMAT_END)
		{
			break;
		}

		if (vertexDecl[k].format == Ceng::VTX_DATATYPE::FORMAT_END)
		{
			break;
		}

		// Check for illegal enumeration values

		if (vertexDecl[k].semantic == Ceng::SHADER_SEMANTIC::FORCE_32B)
		{
			return CE_ERR_INVALID_PARAM;
		}

		if (vertexDecl[k].streamSource > CRENDER_MAX_VERTEX_STREAMS)
		{
			return CE_ERR_INVALID_PARAM;
		}		
	}

	CR_VertexFormat *formatPtr;

	try
	{
		formatPtr = new CR_VertexFormat();
	}
	catch (std::bad_alloc &ba)
	{
		Log::Print("CreateVertexFormat : Error : failed to create format object");
		return CE_ERR_OUT_OF_MEMORY;
	}

	UINT32 variableCount = k;
 
	UINT32 flagBit = 0;
	UINT32 variableSizeBytes;

	memset(formatPtr->streamStrides,0,CRENDER_MAX_VERTEX_STREAMS*sizeof(POINTER));

	CR_VertexVariable tempVariable;

	formatPtr->sizeBytes = 0;

	for(k=0;k<variableCount;k++)
	{
		// Check that variables from the same stream don't
		// overlap

		if (vertexDecl[k].offset < formatPtr->streamStrides[vertexDecl[k].streamSource])
		{
			Log::Print("CreateVertexFormat : Error : variable overlap detected");

			delete formatPtr;
			formatPtr = nullptr;
			return CE_ERR_INVALID_PARAM;
		}

		tempVariable.semantic = vertexDecl[k].semantic;	

		tempVariable.outputOffset = formatPtr->sizeBytes;
	
		tempVariable.dataType = vertexDecl[k].format;

		tempVariable.inputStream = vertexDecl[k].streamSource;
		tempVariable.inputOffset = vertexDecl[k].offset;

		tempVariable.aligned = 0;
		
		formatPtr->semanticFlags |= tempVariable.semantic;

		variableSizeBytes = VertexVariableSize(vertexDecl[k].format);

		formatPtr->streamStrides[vertexDecl[k].streamSource] =
			vertexDecl[k].offset + variableSizeBytes;

		formatPtr->sizeBytes += variableSizeBytes;

		try
		{
			formatPtr->variables.push_back(tempVariable);
		}
		catch(std::bad_alloc &ba)
		{
			Log::Print("CreateVertexFormat : failed to allocate variable");

			delete formatPtr;
			formatPtr = nullptr;
			return CE_ERR_OUT_OF_MEMORY;
		}

	};

	// Check whether variables are 16-byte aligned in their
	// respective streams. First of all, the stride of
	// the source stream must be divisible by 16. Second,
	// the variable's offset in an input chunk must be
	// divisible by 16.

	// NOTE: x86 SSE has faster read/write
	//       operations that require 16-byte aligned 
	//       memory address. If the address is not
	//       aligned, a slower read/write operation
	//       must be used.

	for(k=0;k<formatPtr->variables.size();k++)
	{
		if (formatPtr->variables[k].inputOffset % 16 == 0)
		{
			if (formatPtr->streamStrides[formatPtr->variables[k].inputStream] % 16 == 0)
			{
				formatPtr->variables[k].aligned = 1;
			}
		}
	}

	*vertexFormat = (Ceng::VertexFormat*)formatPtr;

	Log::Print("\nCreateVertexFormat : format dump:\n");

	String temp;

	for(k=0;k<formatPtr->variables.size();k++)
	{
		temp = formatPtr->variables[k].inputStream;
		temp += ",";
		temp += formatPtr->variables[k].outputOffset;
		temp += ",";

		switch(formatPtr->variables[k].dataType)
		{
		case VTX_DATATYPE::FLOAT:
			temp += "float";
			break;
		case VTX_DATATYPE::FLOAT2:
			temp += "float2";
			break;
		case VTX_DATATYPE::FLOAT4:
			temp += "float4";
			break;
		default:
			break;
		}

		temp += ",";

		switch(formatPtr->variables[k].semantic)
		{
		case SHADER_SEMANTIC::POSITION:
			temp += "position";
			break;
		case SHADER_SEMANTIC::NORMAL:
			temp += "normal";
			break;
		case SHADER_SEMANTIC::COLOR_0:
			temp += "color0";
			break;
		case SHADER_SEMANTIC::TEXCOORD_0:
			temp += "texCoord0";
			break;
		case SHADER_SEMANTIC::TEXCOORD_1:
			temp += "texCoord1";
			break;
		case SHADER_SEMANTIC::TEXCOORD_2:
			temp += "texCoord2";
			break;
		default:
			break;
		}

		Log::Print(temp);
	}

	Log::Print("");

	return CE_OK;
}

Ceng::UINT32 ConceptRenderer::VertexVariableSize(Ceng::VTX_DATATYPE::value dataType)
{
	switch(dataType)
	{
	case Ceng::VTX_DATATYPE::FLOAT:
		return 4;
	case Ceng::VTX_DATATYPE::FLOAT2:
		return 8;
	case Ceng::VTX_DATATYPE::FLOAT3:
		return 12;
	case Ceng::VTX_DATATYPE::FLOAT4:
		return 16;
	default:
		return 0;
	};
}

CRESULT ConceptRenderer::CreateVertexBuffer(Ceng::UINT32 vertexSizeBytes,
											Ceng::UINT32 vertexNum,
											Ceng::UINT32 usage,
											Ceng::VertexBuffer **destPtr)
{
	Log::Print("CreateVertexBuffer : method start");

	String temp = "vertexSize = ";
	temp += vertexSizeBytes;
	Log::Print(temp);

	temp = "vertexNum = ";
	temp += vertexNum;
	Log::Print(temp);

	UINT8 *allocPtr = nullptr;
	CR_VertexBuffer *vertexBuffer = nullptr;

	if (vertexSizeBytes == 0)
	{
		Log::Print("CreateVertexBuffer : vertex size is zero");
		return CE_ERR_INVALID_PARAM;
	}

	if (vertexNum == 0)
	{
		Log::Print("CreateVertexBuffer : vertex amount is zero");
		return CE_ERR_INVALID_PARAM;
	}

	allocPtr = (UINT8*)AlignedMalloc(vertexSizeBytes*vertexNum,cacheLineSize);
	if (allocPtr == nullptr)
	{
		Log::Print("CreateVertexBuffer : memory allocation failed");
		return CE_ERR_OUT_OF_MEMORY;
	}

	try
	{
		vertexBuffer = new CR_VertexBuffer(allocPtr,vertexSizeBytes,
											vertexNum);
	}
	catch(std::bad_alloc &ba)
	{
		Log::Print("CreateVertexBuffer : failed to create buffer");
		AlignedFree(allocPtr);
		return CE_ERR_OUT_OF_MEMORY;
	}

	temp = "base address = ";
	temp += (POINTER)allocPtr;

	Log::Print(temp);

	*destPtr = vertexBuffer;

	return CE_OK;
}

CRESULT ConceptRenderer::CreateVertexShader(Ceng::VertexShader **shaderPtr)
{
	Log::Print("CreateVertexShader : method start");

	
	
	//************************************************************
	// Assembler test

	/*
	AssemblyWriter assembler;

	Program *program = assembler.CreateProgram(PROGRAM_TYPE::VERTEX_SHADER);

	// Input attributes

	program->DeclareVariable(VARIABLE_FLAG::ATTRIBUTE,"float4","vPosition",NULL);

	program->DeclareVariable(VARIABLE_FLAG::ATTRIBUTE,"float4","vNormal",NULL);
	program->DeclareVariable(VARIABLE_FLAG::ATTRIBUTE,"float4","vColor",NULL);

	program->DeclareVariable(VARIABLE_FLAG::ATTRIBUTE,"float2","vTexCoord",NULL);
	program->DeclareVariable(VARIABLE_FLAG::ATTRIBUTE,"float","vTest1",NULL);
	program->DeclareVariable(VARIABLE_FLAG::ATTRIBUTE,"float","vTest2",NULL);

	// Output varyings
	
	program->DeclareVariable(VARIABLE_FLAG::VARYING,"float4","fNormal",NULL);
	program->DeclareVariable(VARIABLE_FLAG::VARYING,"float4","fColor",NULL);

	program->DeclareVariable(VARIABLE_FLAG::VARYING,"float2","fTexCoord",NULL);
	program->DeclareVariable(VARIABLE_FLAG::VARYING,"float","fTest1",NULL);
	program->DeclareVariable(VARIABLE_FLAG::VARYING,"float","fTest2",NULL);

	// Uniforms

	program->DeclareVariable(VARIABLE_FLAG::UNIFORM,"float4x4","fullVertexTransform",NULL);

	// Struct test

	Struct *lightSource = program->CreateStruct("LightSource");

	lightSource->members.DeclareVariable(0,"float3","position",NULL);
	lightSource->members.DeclareVariable(0,"float3","normal",NULL);

	program->DeclareType(lightSource);
	
	Struct *testStruct = lightSource->members.CreateStruct("IndentTest");

	testStruct->members.DeclareVariable(0,"float4","testing",NULL);

	lightSource->members.DeclareType(testStruct);
	
	std::vector<UINT32> arraySizes;

	arraySizes.push_back(2);

	program->DeclareVariable(VARIABLE_FLAG::UNIFORM,"LightSource","lights",&arraySizes);

	// Constants

	arraySizes.clear();
	arraySizes.push_back(4);

	// Initializer list

	InitializerList *initializer = new InitializerList();

	initializer->AddItem(new InitFloat4(1.0,1.0,1.0,1.0));

	program->DeclareConstant(0,"float","constTest",&arraySizes,initializer);

	// Shader function

	Function *vertexMain = program->CreateFunction("VertexMain");

	SharedPtr<CodeBlock> body = vertexMain->GetBody();

	body->DeclareVariable(0,"float4","temp",NULL);

	body->AddInstruction("gl_Position",assembler.GetOperator(OPCODE::MULTIPLY),
								"fullVertexTransform","vPosition");

	body->AddInstruction("temp",assembler.GetOperator(OPCODE::ADD),"vPosition");

	program->AddFunction(vertexMain);
	

	assembler.GenerateCode(program,"asm-out.txt");

	delete program;
	*/

	//************************************************************
	// Old creation

	Ceng::CR_VertexShader *vertexShader;

	*shaderPtr = NULL;

	try
	{
		vertexShader = new CR_VertexShader();
	}
	catch (std::bad_alloc &ba)
	{
		return CE_ERR_OUT_OF_MEMORY;
	}
	
	Ceng::CR_vsInputSemantic vsTempInput;

	// Set inputs registers
	
	vsTempInput.semantic = Ceng::SHADER_SEMANTIC::POSITION;
	vsTempInput.options = 0;

	vertexShader->inputSemantics.push_back(vsTempInput);

	/*
	vsTempInput.semantic = Ceng::SHADER_SEMANTIC::NORMAL;
	vsTempInput.options = 0;
	vertexShader->inputSemantics.push_back(vsTempInput);
	*/

	/*
	vsTempInput.semantic = Ceng::SHADER_SEMANTIC::COLOR_0;
	vsTempInput.options = 0;

	vertexShader->inputSemantics.push_back(vsTempInput);
	*/

	vsTempInput.semantic = Ceng::SHADER_SEMANTIC::TEXCOORD_0;
	vsTempInput.options = 0;
	
	vertexShader->inputSemantics.push_back(vsTempInput);

	vsTempInput.semantic = Ceng::SHADER_SEMANTIC::TEXCOORD_1;
	vsTempInput.options = 0;
	
	vertexShader->inputSemantics.push_back(vsTempInput);

	/*
	vsTempInput.semantic = Ceng::SHADER_SEMANTIC::TEXCOORD_2;
	vsTempInput.options = 0;
	
	vertexShader->inputSemantics.push_back(vsTempInput);
	*/

	vertexShader->ConfigureInput();

	// Set output registers

	Ceng::CR_vsOutputSemantic vShaderOutput;

	vShaderOutput.semantic = Ceng::SHADER_SEMANTIC::POSITION;
	vShaderOutput.dataType = Ceng::SHADER_DATATYPE::FLOAT4;
	vShaderOutput.options = 0;
	
	vertexShader->outputSemantics.push_back(vShaderOutput);

	/*
	vShaderOutput.semantic = Ceng::SHADER_SEMANTIC::NORMAL;
	vShaderOutput.dataType = Ceng::SHADER_DATATYPE::FLOAT4;
	vShaderOutput.options = 0;
	
	vertexShader->outputSemantics.push_back(vShaderOutput);
	*/

	/*
	vShaderOutput.semantic = Ceng::SHADER_SEMANTIC::COLOR_0;
	vShaderOutput.dataType = Ceng::SHADER_DATATYPE::FLOAT4;
	vShaderOutput.options = 0;
	
	vertexShader->outputSemantics.push_back(vShaderOutput);
	*/

	vShaderOutput.semantic = Ceng::SHADER_SEMANTIC::TEXCOORD_0;
	vShaderOutput.dataType = Ceng::SHADER_DATATYPE::FLOAT2;
	vShaderOutput.options = 0;
	
	vertexShader->outputSemantics.push_back(vShaderOutput);

	vShaderOutput.semantic = Ceng::SHADER_SEMANTIC::TEXCOORD_1;
	vShaderOutput.dataType = Ceng::SHADER_DATATYPE::FLOAT2;
	vShaderOutput.options = 0;
	
	vertexShader->outputSemantics.push_back(vShaderOutput);

	/*

	vShaderOutput.semantic = Ceng::SHADER_SEMANTIC::TEXCOORD_2;
	vShaderOutput.dataType = Ceng::SHADER_DATATYPE::FLOAT;
	vShaderOutput.options = 0;
	
	vertexShader->outputSemantics.push_back(vShaderOutput);
	*/

	// Set constants

	Ceng::CR_ShaderConstant tempShaderConst;

	tempShaderConst.name = "fullVertexTransform";
	tempShaderConst.dataType = Ceng::SHADER_DATATYPE::FLOAT4x4;

	vertexShader->uniformList.push_back(tempShaderConst);

	vertexShader->ConfigureConstants();

	*shaderPtr = (Ceng::VertexShader*)vertexShader;

	return CE_OK;
}


