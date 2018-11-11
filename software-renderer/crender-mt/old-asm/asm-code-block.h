/*****************************************************************************
*
* asm-code-block.h
*
* By Jari Korkala 10/2013
*
*****************************************************************************/

#ifndef CENG_ASM_CODE_BLOCK_H
#define CENG_ASM_CODE_BLOCK_H

#include "asm-base.h"

#include "asm-instruction.h"

namespace Ceng
{
	class CodeBlock : public Scope
	{
	public:
		/*
		 * Actual code.
		 */
		std::vector<Ceng::SharedPtr<CodeItem>> code;

	public:

		CodeBlock()
		{
		}

		CodeBlock(SharedPtr<ProgramType> &programType,Scope *parent)
		{
			this->programType = programType;
			this->parent = parent;
		}

		virtual ~CodeBlock()
		{
		}

		CRESULT CreateOperand(const String &operand,Operand *out)
		{
			// Find destination variable

			*out = Operand();

			SharedPtr<Variable> variable = FindSymbol<Variable>(&Scope::variables,operand);

			if (variable == NULL)
			{
				return CE_ERR_FAIL;
			}

			Datatype *datatype = variable->datatype;

			if (datatype->IsBasicType() == false)
			{
				return CE_ERR_FAIL;
			}

			BasicType *basicType = static_cast<BasicType*>(datatype);

			out->variable = variable;
			out->data = OperandData(*basicType);

			return CE_OK;
		}

		CRESULT AddInstruction(const String &destination,const SharedPtr<Operator> &operation,
								const String &source1)
		{
			return CE_OK;
		}

		/*
		 * Appends a three-operand instruction.
		 */
		CRESULT AddInstruction(const String &destination,const SharedPtr<Operator> &operation,
								const String &source1,const String &source2)
		{
			Operand destOp,sourceOp1,sourceOp2;
			
			CRESULT cresult;
			
			cresult = CreateOperand(destination,&destOp);
			if (cresult != CE_OK)
			{
				Log::Print("CodeBlock.AddInstruction : unknown destination operand");
				return cresult;
			}

			cresult = CreateOperand(source1,&sourceOp1);
			if (cresult != CE_OK)
			{
				Log::Print("CodeBlock.AddInstruction : unknown first source operand");
				return cresult;
			}
			
			cresult = CreateOperand(source2,&sourceOp2);
			if (cresult != CE_OK)
			{
				Log::Print("CodeBlock.AddInstruction : unknown second source operand");
				return cresult;
			}

			SharedPtr<Pattern> pattern = operation->Test(destOp.data,sourceOp1.data,sourceOp2.data);

			if (pattern == NULL)
			{
				Log::Print("CodeBlock.AddInstruction : no matching pattern found");
				return CE_ERR_FAIL;
			}

			code.push_back(new Instruction(destOp,operation,sourceOp1,sourceOp2));

			return CE_OK;
		}

		CRESULT AddBlock(CodeBlock *block)
		{
			code.push_back(block);
			return CE_OK;
		}

		virtual String ThisToSource(String &indentLevel,const UINT32 options) const override
		{
			UINT32 k;

			String temp;

			for(k=0;k<variables.size();k++)
			{
				temp += variables[k]->ToSource(indentLevel,
													PRINT_OPTION::COMMAND_END |
													PRINT_OPTION::NEW_LINE);
			}

			if (variables.size() > 0 && code.size() > 0)
			{
				temp += "\n";
			}			
			
			for(k=0;k<code.size();k++)
			{
				temp += code[k]->ToSource(indentLevel,
											PRINT_OPTION::COMMAND_END |
											PRINT_OPTION::NEW_LINE);
			}
			

			return temp;
		}
	};
}

#endif