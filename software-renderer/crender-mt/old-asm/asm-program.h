/*****************************************************************************
*
* asm-program.h
*
* By Jari Korkala 10/2013
*
*****************************************************************************/

#ifndef CENG_ASM_PROGRAM_H
#define CENG_ASM_PROGRAM_H

#include "asm-scope.h"

#include "asm-function.h"

namespace Ceng
{


	class Program : public Scope
	{
	public:

		Program(SharedPtr<ProgramType> &programType,Scope *language)
		{
			options = SCOPE_OPTION::ALLOW_DATATYPES | SCOPE_OPTION::ALLOW_FUNCTION;

			this->programType = programType;
			parent = language;
		}

		virtual ~Program()
		{
		}

		virtual String ThisToSource(String &indent,const UINT32 options) const override
		{
			String temp;

			temp += Scope::ThisToSource(indent,0);
			
			return temp;
		}
	};
	
}

#endif