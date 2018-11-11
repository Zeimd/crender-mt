/*****************************************************************************
*
* asm-opcodes.h
*
* By Jari Korkala 10/2013
*
*****************************************************************************/

#ifndef CENG_ASM_OPCODES_H
#define CENG_ASM_OPCODES_H

#include <ceng/datatypes/basic-types.h>

namespace Ceng
{
	namespace OPCODE
	{
		enum value
		{
			/*
			 * No action.
			 */ 
			NOP = 0 ,

			/*
			 * Copy contents.
			 */
			MOVE = 1 ,
			
			/*
			 * Addition.
			 */
			ADD = 2 ,

			/*
			 * Subtraction.
			 */
			SUB = 3 ,
			
			/*
			 * Multiplication.
			 */
			MULTIPLY = 4,

			/*
			 * Division.
			 */
			DIVISION = 5,

			/*
			 * Bitwise AND.
			 */
			AND = 6,

			/*
			 * Bitwise OR.
			 */
			OR = 7,

			/*
			 * Bitwise exclusive OR
			 */
			XOR = 8,

			/*
			 * Multiply by -1
			 */
			NEGATION = 9,

			/*
			 * Decrement by 1.
			 */
			DECREMENT = 10,

			/*
			 * Increase by 1.
			 */
			INCREMENT = 11,

			/*
			 * One's complement of operand.
			 */
			BIT_FLIP = 12,

			/*
			 * Dot product.
			 */
			DOT_PRODUCT = 13 ,

			/*
			 * Cross product. 3-component vectors only.
			 */
			CROSS_PRODUCT = 14 ,

			SQUARE_ROOT = 15 ,

			/*
			 * Absolute value or vector length.
			 */
			LENGTH = 16 ,

			/*
			 * Normalize vector.
			 */
			NORMALIZE = 17 ,

			/*
			 * Reflect in respect to another vector
			 */
			REFLECT = 18 ,

			/*
			 * Return minimum of two values.
			 */
			MIN = 19 ,

			/*
			 * Return maximum of two values.
			 */
			MAX = 20 ,

			/*
			 * Limit to range:
			 * if (x < min) x = min;
			 * if (x > max) x = max;
			 */
			CLAMP = 21 ,

			SIN = 22 ,
			COS = 23 ,
			TAN = 24 ,





			FORCE32B = (1 << 30) ,
		};

	};

	/*
	 * Used when constructing mnemonics of the shader assembly.
	 */
	namespace OPCODE_CLASS
	{
		enum value
		{
			/*
			 * All operands are scalars.
			 */
			SCALAR = 1 ,

			/*
			 * All operands are vectors.
			 */
			VECTOR = 2 ,

			/*
			 * All operands are matrices.
			 */
			MATRIX = 3 ,

			/*
			 * Used for vector-scalar operations.
			 */
			VECTOR_SCALAR = 4 ,
			
			/*
			 * Used for vector = matrix * vector.
			 */
			MATRIX_VECTOR = 5 ,

			/*
			 * Used for matrix-scalar operations.
			 */
			MATRIX_SCALAR = 6 ,
		};
	};

	/*
	shader assembly example:

	add_v4_f vector(dest),vector,vector;
	mul_m4_v4_f vector(dest),matrix(source1),vector(source2);
	mul_v4_s_f vector(dest),vector(source),scalar(source2);
	*/


	class OperandData
	{
	public:
		/*
		 * Container type. Essentially number of dimensions:
		 *
		 * scalar : dim=0
		 * vector : dim=1
		 * matrix : dim=2
		 */
		CONTAINER::value container;

		/*
		 * Hardware data type for each element. Must be one
		 * of OPCODE::data values.
		 */
		DATATYPE::value dataFormat;

		/*
		 * Size of the container. 
		 *
		 * If ZERO, dimension is checked against all operands that
		 * have specified this value.
		 *
		 * vector : number of components
		 * matrix : size x size elements
		 */
		UINT32 size;

		static const UINT32 SAME_SIZE = 256;

		OperandData()
		{
			dataFormat = DATATYPE::UNKNOWN;
			container = CONTAINER::UNKNOWN;
			size = 0;
		}

		OperandData(const BasicType &type)
		{
			SetHardwareType(type.hardwareType);
		}

		OperandData(const SHADER_DATATYPE::value hardwareType)
		{
			SetHardwareType(hardwareType);
		}

		void SetHardwareType(const SHADER_DATATYPE::value hardwareType)
		{
			switch(hardwareType)
			{
			case SHADER_DATATYPE::FLOAT:

				dataFormat = DATATYPE::FLOAT;
				container = CONTAINER::VECTOR;
				size = 1;

				break;
			case SHADER_DATATYPE::FLOAT2:

				dataFormat = DATATYPE::FLOAT;
				container = CONTAINER::VECTOR;
				size = 2;

				break;
			case SHADER_DATATYPE::FLOAT3:

				dataFormat = DATATYPE::FLOAT;
				container = CONTAINER::VECTOR;
				size = 3;

				break;
			case SHADER_DATATYPE::FLOAT4:

				dataFormat = DATATYPE::FLOAT;
				container = CONTAINER::VECTOR;
				size = 4;

				break;
			case SHADER_DATATYPE::FLOAT4x4:

				dataFormat = DATATYPE::FLOAT;
				container = CONTAINER::MATRIX;
				size = 4;
				break;
			}
		}
	};

	class Pattern;

	class PatternItem
	{
	public:
		PatternItem()
		{
		}

		virtual ~PatternItem()
		{
		}

		/*
		 * Test for unary operations.
		 */
		virtual SharedPtr<Pattern> Test(const OperandData &destination,const OperandData &source1) const abstract;
	
		/*
		 * Test for binary operations.
		 */
		virtual SharedPtr<Pattern> Test(const OperandData &destination,const OperandData &source1,
								const OperandData &source2) const abstract;

		/*
		 * Test for three-form operations.
		 */
		virtual SharedPtr<Pattern> Test(const OperandData &destination,const OperandData &source1,const OperandData &source2,
								const OperandData &source3) const abstract;
	};

	

	class Pattern : public PatternItem
	{
	public:

		// pattern = [container,format,size]

		/*
		 * Total number of operands used.
		 */
		UINT32 operandNum;

		OperandData destination;
		OperandData source1;
		OperandData source2;
		OperandData source3;

	public:
		Pattern()
		{
			operandNum = 0;
		}

		virtual ~Pattern()
		{
		}

		virtual SharedPtr<Pattern> Test(const OperandData &destination,const OperandData &source1) const override
		{
			return NULL;
		}
	
		virtual SharedPtr<Pattern> Test(const OperandData &destination,const OperandData &source1,
								const OperandData &source2) const override
		{



			return NULL;
		}

		virtual SharedPtr<Pattern> Test(const OperandData &destination,const OperandData &source1,const OperandData &source2,
								const OperandData &source3) const override
		{
			return NULL;
		}
	};

	class PatternGroup : public PatternItem
	{
	public:
		std::vector<SharedPtr<PatternItem>> patterns;

		PatternGroup()
		{
		}

		virtual ~PatternGroup()
		{
		}

		virtual SharedPtr<Pattern> Test(const OperandData &destination,const OperandData &source1) const override
		{
			UINT32 k;

			SharedPtr<Pattern> temp;

			for(k=0;k<patterns.size();k++)
			{
				temp = patterns[k]->Test(destination,source1);

				if (temp != NULL)
				{
					return temp;
				}
			}

			return NULL;
		}
	
		virtual SharedPtr<Pattern> Test(const OperandData &destination,const OperandData &source1,
								const OperandData &source2) const override
		{
			UINT32 k;

			SharedPtr<Pattern> temp;

			for(k=0;k<patterns.size();k++)
			{
				temp = patterns[k]->Test(destination,source1,source2);

				if (temp != NULL)
				{
					return temp;
				}
			}

			return NULL;
		}

		virtual SharedPtr<Pattern> Test(const OperandData &destination,const OperandData &source1,const OperandData &source2,
								const OperandData &source3) const override
		{
			UINT32 k;

			SharedPtr<Pattern> temp;

			for(k=0;k<patterns.size();k++)
			{
				temp = patterns[k]->Test(destination,source1,source2,source3);

				if (temp != NULL)
				{
					return temp;
				}
			}

			return NULL;
		}
	};

	class Operator : public PatternGroup
	{
	public:
		OPCODE::value opcode;

	public:
		Operator()
		{
		}

		virtual ~Operator()
		{
		}
	};

}

#endif
