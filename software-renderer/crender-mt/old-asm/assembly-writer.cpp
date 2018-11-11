/*****************************************************************************
*
* assembly-writer.cpp
*
* By Jari Korkala 10/2013
*
*****************************************************************************/

//#include "stdafx.h"

#include "assembly-writer.h"

#include <fstream>

namespace Ceng
{
	AssemblyWriter::AssemblyWriter()
	{
		// Build reserved names table

		language.DeclareType(new ReservedSymbol("const"));
		language.DeclareType(new ReservedSymbol("uniform"));
		language.DeclareType(new ReservedSymbol("varying"));
		language.DeclareType(new ReservedSymbol("attribute"));

		// Basic data types

		language.DeclareType(new BasicType("bool",CONTAINER::VECTOR,DATATYPE::BOOL,1,SHADER_DATATYPE::BOOL));

		language.DeclareType(new BasicType("int",CONTAINER::VECTOR,DATATYPE::INT,1,SHADER_DATATYPE::INT));
		language.DeclareType(new BasicType("uint",CONTAINER::VECTOR,DATATYPE::INT,1,SHADER_DATATYPE::UINT));
	
		language.DeclareType(new BasicType("float",CONTAINER::VECTOR,DATATYPE::FLOAT,1,SHADER_DATATYPE::FLOAT));
		language.DeclareType(new BasicType("float2",CONTAINER::VECTOR,DATATYPE::FLOAT,2,SHADER_DATATYPE::FLOAT2));
		language.DeclareType(new BasicType("float3",CONTAINER::VECTOR,DATATYPE::FLOAT,3,SHADER_DATATYPE::FLOAT3));
		language.DeclareType(new BasicType("float4",CONTAINER::VECTOR,DATATYPE::FLOAT,4,SHADER_DATATYPE::FLOAT4));

		language.DeclareType(new BasicType("float4x4",CONTAINER::MATRIX,DATATYPE::FLOAT,4,SHADER_DATATYPE::FLOAT4x4));

		language.AddAlias("float4","vec4");

		// Opcode patterns

		SharedPtr<Pattern> matrixVecMul = new Pattern();

		matrixVecMul->operandNum = 3;

		matrixVecMul->destination.container = CONTAINER::VECTOR;
		matrixVecMul->destination.dataFormat = DATATYPE::SAME;
		matrixVecMul->destination.size = OperandData::SAME_SIZE;

		matrixVecMul->source1.container = CONTAINER::MATRIX;
		matrixVecMul->source1.dataFormat = DATATYPE::SAME;
		matrixVecMul->source1.size = OperandData::SAME_SIZE;

		matrixVecMul->source2.container = CONTAINER::VECTOR;
		matrixVecMul->source2.dataFormat = DATATYPE::SAME;
		matrixVecMul->source2.size = OperandData::SAME_SIZE;

		SharedPtr<PatternGroup> commonPatterns = new PatternGroup();

		SharedPtr<Pattern> twoForm = new Pattern();

		twoForm->operandNum = 3;

		twoForm->destination.container = CONTAINER::SAME;
		twoForm->destination.dataFormat = DATATYPE::SAME;
		twoForm->destination.size = OperandData::SAME_SIZE;

		twoForm->source1.container = CONTAINER::SAME;
		twoForm->source1.dataFormat = DATATYPE::SAME;
		twoForm->source1.size = OperandData::SAME_SIZE;

		SharedPtr<Pattern> threeForm = new Pattern();

		threeForm->operandNum = 3;

		threeForm->destination.container = CONTAINER::SAME;
		threeForm->destination.dataFormat = DATATYPE::SAME;
		threeForm->destination.size = OperandData::SAME_SIZE;

		threeForm->source1.container = CONTAINER::SAME;
		threeForm->source1.dataFormat = DATATYPE::SAME;
		threeForm->source1.size = OperandData::SAME_SIZE;

		threeForm->source2.container = CONTAINER::SAME;
		threeForm->source2.dataFormat = DATATYPE::SAME;
		threeForm->source2.size = OperandData::SAME_SIZE;

		commonPatterns->patterns.push_back(twoForm);
		commonPatterns->patterns.push_back(threeForm);

		// Build operators

		Operator *opcode;
		
		opcode = new Operator();

		opcode->opcode = OPCODE::MULTIPLY;
		opcode->patterns.push_back(matrixVecMul);
		opcode->patterns.push_back(commonPatterns);

		operators.push_back(opcode);

		opcode = new Operator();

		opcode->opcode = OPCODE::ADD;
		opcode->patterns.push_back(commonPatterns);
		
		operators.push_back(opcode);

		// Create program configurations

		vertexProgram = new ProgramType();

		vertexProgram->type = PROGRAM_TYPE::VERTEX_SHADER;		
	}

	AssemblyWriter::~AssemblyWriter()
	{
	}

	SharedPtr<Operator> AssemblyWriter::GetOperator(const OPCODE::value opcode)
	{
		UINT32 k;

		for(k=0;k<operators.size();k++)
		{
			if (operators[k]->opcode == opcode)
			{
				return operators[k];
			}
		}

		return NULL;
	}

	Program* AssemblyWriter::CreateProgram(const PROGRAM_TYPE::value programType)
	{
		SharedPtr<ProgramType> *progType;

		progType = NULL;

		switch(programType)
		{
		case PROGRAM_TYPE::VERTEX_SHADER:
			progType = &vertexProgram;
			break;
		case PROGRAM_TYPE::PIXEL_SHADER:
			break;
		default:
			return NULL;
		}

		Program *temp = new Program(*progType,&language);

		// Add implicit variables

		if (programType == PROGRAM_TYPE::VERTEX_SHADER)
		{
			temp->DeclareVariable(VARIABLE_FLAG::VARYING,"float4","gl_Position",NULL);
		}

		return temp;
	}

	CRESULT AssemblyWriter::GenerateCode(const Program *program,const char *outputFile)
	{
		std::fstream file;

		file.open(outputFile,std::ios_base::out);

		file << "Shader assembler test \n*************************** \n\n";

		String temp = program->ToSource(String(),0);

		file << temp.ToCString() << std::endl;

		file.close();

		return CE_OK;
	}
};