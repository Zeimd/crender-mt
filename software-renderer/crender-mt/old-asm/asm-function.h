/*****************************************************************************
*
* asm-function.h
*
* By Jari Korkala 10/2013
*
*****************************************************************************/

#ifndef CENG_ASM_FUNCTION_H
#define CENG_ASM_FUNCTION_H

#include "asm-base.h"

#include "asm-code-block.h"

namespace Ceng
{
	class Function : public Symbol
	{
	public:
		std::vector<Ceng::SharedPtr<Variable>> parameterList;

		Ceng::UINT32 returnFlags;

		String returnType;

		SharedPtr<CodeBlock> body;
	public:

		Function()
		{
		}

		Function(SharedPtr<ProgramType> &programType,const String &name,Scope *parent)
		{
			this->programType = programType;

			this->name = name;

			body = new CodeBlock(programType,parent);
		}

		virtual ~Function()
		{
		}

		SharedPtr<CodeBlock> GetBody()
		{
			return body;
		}
		
		virtual String ThisToSource(String &indent,const UINT32 options) const override
		{
			String temp;

			// TODO: Return type

			temp += name;
			temp += "(";

			// TODO: Parameter list

			temp += ")\n";

			// Function body

			temp += body->ToSource(indent,PRINT_OPTION::SCOPE_BRACKETS);

			return temp;
		}
	};
}

#endif