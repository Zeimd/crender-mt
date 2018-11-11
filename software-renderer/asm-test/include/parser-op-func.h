/*****************************************************************************
*
* parser-op-func.h
*
* By Jari Korkala 6/2014
*
*****************************************************************************/

#ifndef CASM_PARSER_OP_FUNC_H
#define CASM_PARSER_OP_FUNC_H

#include "parser-operator.h"

namespace Casm
{
	class ParserOp_FunctionDecl : public ParserOperator
	{
	public:

		ParserOp_FunctionDecl(const TokenData &data);

		virtual ~ParserOp_FunctionDecl();

		virtual const char* TypeString() const;

		virtual const Ceng::SharedPtr<ParserOperand> LocalAction(ParserState *state,
															Ceng::SharedPtr<ParserOperand> left,
															Ceng::SharedPtr<ParserOperand> right);

	};
}

#endif