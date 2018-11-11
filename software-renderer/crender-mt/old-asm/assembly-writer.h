/*****************************************************************************
*
* assembly-writer.h
*
* By Jari Korkala 10/2013
*
*****************************************************************************/

#ifndef CENG_ASSEMBLY_WRITER_H
#define CENG_ASSEMBLY_WRITER_H

#include <ceng/datatypes/shared-ptr.h>

#include "asm-base.h"

#include "asm-scope.h"
#include "asm-struct.h"

#include "asm-program.h"

namespace Ceng
{
	class AssemblyWriter
	{
	protected:

		std::vector<Ceng::SharedPtr<Datatype>> defaultTypes;

		std::vector<SharedPtr<Operator>> operators;

		Scope language;

		SharedPtr<ProgramType> vertexProgram;

	public:

		SharedPtr<Operator> AssemblyWriter::GetOperator(const OPCODE::value opcode);

		AssemblyWriter();

		~AssemblyWriter();

		Program* CreateProgram(const PROGRAM_TYPE::value programType);

		CRESULT GenerateCode(const Program *program,const char *outputFile);

		
	};
};

#endif