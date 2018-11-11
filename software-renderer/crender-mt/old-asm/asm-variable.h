/*****************************************************************************
*
* asm-variable.h
*
* By Jari Korkala 10/2013
*
*****************************************************************************/

#ifndef CENG_ASM_VARIABLE_H
#define CENG_ASM_VARIABLE_H

#include "asm-base.h"

#include "asm-constructor.h"

namespace Ceng
{
	class Variable : public Symbol
	{
	public:

		/**
		 * A combination of SHADER_DATA_FLAG elements.
		 */
		Ceng::UINT32 flags;
		
		SharedPtr<Datatype> datatype;

		/*
		 * Size of each array level. Stored from left to right according
		 * to declaration:
		 *
		 * type array[a][b][c];
		 *
		 * arraySizes[0] = a
		 * arraySizes[1] = b
		 * arraySizes[2] = c
		 */
		std::vector<Ceng::UINT32> arraySizes;

		Variable()
		{
			flags = 0;
		}

		Variable(Ceng::UINT32 flags,SharedPtr<Datatype> &datatype,const String &name,
					const std::vector<Ceng::UINT32> *arraySizes)
		{
			this->flags = flags;
			this->datatype = datatype;
			this->name = name;

			if (arraySizes != NULL)
			{
				this->arraySizes = *arraySizes;
			}
		}

		virtual Ceng::BOOL IsConstant()
		{
			return false;
		}

		virtual ~Variable()
		{
		}

		virtual String ThisToSource(String &indent,const UINT32 options) const override
		{
			String temp;

			// Options

			if (flags & VARIABLE_FLAG::CONSTANT)
			{
				temp += "const ";
			}

			if (flags & VARIABLE_FLAG::ATTRIBUTE)
			{
				temp += "attribute ";
			}

			if (flags & VARIABLE_FLAG::UNIFORM)
			{
				temp += "uniform ";
			}

			if (flags & VARIABLE_FLAG::VARYING)
			{
				temp += "varying ";
			}

			temp += datatype->name;
			temp += " ";
			temp += name;

			UINT32 k;

			
			for(k=0;k<arraySizes.size();k++)
			{
				temp += "[";
				temp += arraySizes[k];
				temp += "]";
			}
			

			return temp;
		}
	};


	/*
	 * Variable with value set at declaration time.
	 */
	class Constant : public Variable
	{
	public:
		Constant()
		{
		}

		virtual ~Constant()
		{
		}

		virtual Ceng::BOOL IsConstant()
		{
			return true;
		}

		virtual String ThisToSource(String &indent,const UINT32 options) const override
		{
			String temp;

			temp += Variable::ThisToSource(indent,options);

			return String("constant");
		}
	};
}

#endif