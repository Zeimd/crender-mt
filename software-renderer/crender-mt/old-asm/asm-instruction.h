/*****************************************************************************
*
* asm-instruction.h
*
* By Jari Korkala 10/2013
*
*****************************************************************************/

#ifndef CENG_ASM_INSTRUCTION_H
#define CENG_ASM_INSTRUCTION_H

#include "asm-base.h"

#include "asm-opcodes.h"

namespace Ceng
{
	class Operand
	{
	public:
		SharedPtr<Variable> variable;
		OperandData data;
	};

	/*
	 * A single instruction in the execution flow.
	 */
	class Instruction : public CodeItem
	{
	public:

		UINT32 operands;

		SharedPtr<Operator> operation;

		Operand destination;
		Operand source1;
		Operand source2;
		Operand source3;

		Instruction(const Operand &destination,const SharedPtr<Operator> &operation,
						const Operand &source1,const Operand &source2)
		{
			operands = 3;

			this->operation = operation;

			this->destination = destination;
			this->source1 = source1;
			this->source2 = source2;
		}

		virtual ~Instruction()
		{
		}

		virtual String ThisToSource(String &indent,const UINT32 options) const override
		{
			
			String temp;

			switch(operands)
			{
			case 1:

				temp += destination.variable->name;
				temp += " = ";

				temp += OperatorToString(operation->opcode);

				temp += destination.variable->name;
				break;
			case 2:

				temp += destination.variable->name;

				temp += " ";

				temp += OperatorToString(operation->opcode);

				if (operation->opcode != OPCODE::MOVE)
				{
					temp += "=";
				}

				temp += " ";

				temp += source1.variable->name;

				break;
			case 3:

				temp += destination.variable->name;

				temp += " = ";

				temp += source1.variable->name;

				temp += " ";
				temp += OperatorToString(operation->opcode);
				temp += " ";			

				temp += source2.variable->name;
				break;
			}

			return temp;
		}

		String OperatorToString(const OPCODE::value opcode) const
		{
			switch(opcode)
			{
			case OPCODE::MOVE:
				return("=");
				break;
			case OPCODE::ADD:
				return("+");
				break;
			case OPCODE::SUB:
				return("-");
				break;
			case OPCODE::MULTIPLY:
				return("*");
				break;
			}

			return String();
		}
	};
}

#endif