/*****************************************************************************
*
* parser-opcode.h
*
* By Jari Korkala 5/2014
*
*****************************************************************************/

#ifndef CASM_PARSER_OPCODE_H
#define CASM_PARSER_OPCODE_H

#include "parser-operator.h"

#include "x86-instruction.h"

namespace Casm
{
	class ParserOpcode : public ParserOperator
	{
	protected:

		const X86::Instruction *instruction;
	
	public:

		ParserOpcode(const X86::Instruction *instruction,const TokenData &data);

		virtual ~ParserOpcode();

		virtual const Ceng::SharedPtr<ParserOperand> Action(ParserState *state,
														Ceng::SharedPtr<ParserOperand> left,
														Ceng::SharedPtr<ParserOperand> right) override;

		virtual const Ceng::SharedPtr<ParserOperand> LocalAction(ParserState *state,
															Ceng::SharedPtr<ParserOperand> left,
															Ceng::SharedPtr<ParserOperand> right) override;
	};
}

#endif