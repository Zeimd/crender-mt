/*****************************************************************************
*
* asm-struct.h
*
* By Jari Korkala 10/2013
*
*****************************************************************************/

#ifndef CENG_ASM_STRUCT_H
#define CENG_ASM_STRUCT_H

#include "asm-base.h"

namespace Ceng
{
class Struct : public Datatype
	{
	public:

		Scope members;

		Struct()
		{
		}

		Struct(SharedPtr<ProgramType> &programType,const String &name,Scope *parent)
		{
			this->programType = programType;

			this->name = name;

			members = Scope(programType,parent);
		}

		virtual ~Struct()
		{
		}

		virtual String ThisToSource(String &indent,const UINT32 options) const override
		{
			String temp;

			temp = "struct ";
			temp += name;
			temp += "\n";

			temp += members.ToSource(indent,PRINT_OPTION::SCOPE_BRACKETS);
			
			return temp;
		}
	};
}

#endif