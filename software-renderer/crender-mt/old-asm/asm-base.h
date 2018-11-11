/*****************************************************************************
*
* asm-base.h
*
* By Jari Korkala 10/2013
*
*****************************************************************************/

#ifndef CENG_ASM_BASE_H
#define CENG_ASM_BASE_H

#include <memory>
#include <ceng/enums/shader-datatype.h>

#include <ceng/lib/liblog.h>


#include "crender-base.h"

namespace Ceng
{
	namespace CONTAINER
	{
		enum value
		{
			UNKNOWN = 0 ,
			SCALAR = 1 ,
			VECTOR = (1 << 1) ,
			MATRIX = (1 << 2) ,

			SAME = 256 ,

			FORCE_32B = (1 << 30) ,
		};
	}

	namespace DATATYPE
	{
		enum value
		{
			UNKNOWN = 0,

			FLOAT = 1 ,
			DOUBLE = 2 ,
			INT = 3 ,
			UINT = 4 ,
			BOOL = 5 ,

			SAME = 6 ,

			FORCE32_B = (1 << 30) ,
		};
	}

	/*
	 * Variable declaration modifiers.
	 *
	 * Storage class: CONSTANT , UNIFORM , ATTRIBUTE , VARYING 
	 *
	 * Pre-shader setup : NO_PERSPECTIVE , NO_INTERPOLATION
	 *
	 * Assembler set: NO_WRITE , PER_PIXEL
	 */
	namespace VARIABLE_FLAG
	{
		enum value
		{
			/*
			 * Cannot be a destination operand in any instruction.
			 *
			 * NOTE: Causes an error if given when declaring a variable.
			 */
			NO_WRITE = 1 ,
			
			/*
			 * Constant over multiple calls. Application sets the value.
			 * Global level only.
			 */
			UNIFORM = (1 << 1) ,

			/*
			 * Input in pixel shader. Output in vertex shader.
			 * Global level only.
			 */
			VARYING = (1 << 2) ,

			/*
			 * Input in vertex shader.
			 * Global level only.
			 */
			ATTRIBUTE = (1 << 3) ,

			/*
			 * Variable is part of a dependence chain that starts from an input
			 * VARYING. Pixel-specific instances are created.
			 */
			PER_PIXEL = (1 << 4) ,

		
			/*
			 * Skip perspective correction. 
			 * NOTE: Pixel shader only.
			 */
			NO_PERSPECTIVE = (1 << 5) ,

			/*
			 * Don't interpolate variable in screen space during rasterization.
			 * NOTE: Pixel shader only.
			 */
			NO_INTERPOLATION = (1 << 6) ,

			
			/*
			 * Value doesn't change after initialization. Implemented
			 * as hard-coded data.
			 */
			CONSTANT = (1 << 7) ,

			FORCE_32B = (1 << 30) ,
		};
	};

	namespace SCOPE_OPTION
	{
		enum value
		{
			/**
			 * Datatypes can be declared in this scope.
			 */
			ALLOW_DATATYPES = 1 ,

			/**
			 * Functions can be declared in this scope.
			 */
			ALLOW_FUNCTION = (1 << 1) ,

			FORCE32B = (1 << 30) ,
		};
	};

	namespace PRINT_OPTION
	{
		enum value
		{
			/*
			 * Create a new scope with brackets and indenting.
			 */
			SCOPE_BRACKETS = 1 ,

			/*
			 * If the language requires a symbol at the end of each command,
			 * this adds it after printing the symbol.
			 */
			COMMAND_END = (1 << 1) ,

			NEW_LINE = (1 << 2) ,

			NO_INDENT = (1 << 3) ,

			FORCE_32B = (1 << 30) ,
		};
	}

	namespace PROGRAM_TYPE
	{
		enum value
		{
			VERTEX_SHADER = 1 ,
			PIXEL_SHADER = 2 ,

			FORCE_32B = (1 << 30) ,
		};
	};

	class ProgramType
	{
	public:
		PROGRAM_TYPE::value type;
		Ceng::Log *errorLog;

		Ceng::UINT32 maxArrayDim;

		ProgramType()
		{
			errorLog = NULL;
		}
	};

	class AssemblerObject
	{
	protected:

		SharedPtr<ProgramType> programType;

		AssemblerObject()
		{
		}

	public:
		
		virtual ~AssemblerObject()
		{
		}

		String ToSource(String &indent,const UINT32 options) const
		{
			String temp;
		
			if (options & PRINT_OPTION::SCOPE_BRACKETS)
			{				
				temp += indent;
				temp += "{\n";

				indent += "\t";
			}
			else
			{
				if ( !(options & PRINT_OPTION::NO_INDENT) )
				{
					temp += indent;
				}
			}

			temp += ThisToSource(indent,0);

			if (options & PRINT_OPTION::SCOPE_BRACKETS)
			{
				indent.RemoveLastChar();

				temp += indent;
				temp += "}";
			}

			if (options & PRINT_OPTION::COMMAND_END)
			{
				temp += ";";
			}

			if (options & PRINT_OPTION::NEW_LINE)
			{
				temp += "\n";
			}

			return temp;
		}

		virtual String ThisToSource(String &indent,const UINT32 options) const
		{
			return String("Printable");
		}
	};

	class Symbol : public AssemblerObject
	{
	public:
		String name;

		Symbol()
		{
		}

		virtual ~Symbol()
		{
		}

		virtual String ThisToSource(String &indent,const UINT32 options) const override
		{
			return String("Symbol");
		}
	};

	class Datatype : public Symbol
	{
	public:
		Datatype()
		{
		}

		virtual ~Datatype()
		{
		}

		virtual Ceng::BOOL IsBasicType() const
		{
			return false;
		}

		virtual String ThisToSource(String &indent,const UINT32 options) const override
		{
			return String("Datatype");
		}

	};

	class ReservedSymbol : public Datatype
	{
	public:
		ReservedSymbol()
		{
		}

		ReservedSymbol(const String &name)
		{
			this->name = name;
		}

		virtual ~ReservedSymbol()
		{
		}

		virtual String ThisToSource(String &indent,const UINT32 options) const override
		{
			String temp = "Reserved symbol : ";
			temp += name;

			return temp;
		}
	};

	class Alias : public Datatype
	{
	public:
		SharedPtr<Datatype> actualType;

		Alias()
		{
			actualType = NULL;
		}

		Alias(const String &aliasName,SharedPtr<Datatype> &actualName)
		{
			this->name = aliasName;
			this->actualType = actualType;
		}

		virtual ~Alias()
		{
		}

		virtual String ThisToSource(String &indent,const UINT32 options) const override
		{
			String temp;

			temp = "typedef ";
			temp += actualType->name;
			temp += " ";
			temp += name;

			return temp;
		}
	};

	/*
	 * A basic data type that is expected to be supported by hardware.
	 */
	class BasicType : public Datatype
	{
	public:

		SHADER_DATATYPE::value hardwareType;

		CONTAINER::value container;
		DATATYPE::value datatype;

		UINT32 size;

		BasicType()
		{
			hardwareType = SHADER_DATATYPE::UNKNOWN;
			
			container = CONTAINER::UNKNOWN;
			datatype = DATATYPE::UNKNOWN;
			size = 1;
		}

		BasicType(const String &name,const CONTAINER::value container,
			const DATATYPE::value datatype, const UINT32 size, const SHADER_DATATYPE::value hardwareType)
		{
			this->name = name;
			this->hardwareType = hardwareType;

			this->container = container;
			this->datatype = datatype;
			this->size = size;
		}

		virtual ~BasicType()
		{
		}

		virtual Ceng::BOOL IsBasicType() const override
		{
			return true;
		}

		virtual String ThisToSource(String &indent,const UINT32 options) const override
		{
			return String("BasicType");
		}
	};

	/*
	 * A common ancestor for objects that can be combined to describe source code.
	 */
	class CodeItem : public AssemblerObject
	{
	public:

		/*
		 * Position in declaration list.
		 */
		UINT32 declareIndex;

		CodeItem()
		{
		}

		virtual ~CodeItem()
		{
		}

		virtual String ThisToSource(String &indent,const UINT32 options) const
		{
			return String("CodeItem");
		}
	};

}

#endif