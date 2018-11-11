/*****************************************************************************
*
* parser-op-bracket.h
*
* By Jari Korkala 6/2014
*
*****************************************************************************/

#ifndef CASM_PARSER_OP_BRACKET_H
#define CASM_PARSER_OP_BRACKET_H

#include "parser-operator.h"

namespace Casm
{
	class ParserOp_Parenthesis : public ParserOperator
	{
	public:

		ParserOp_Parenthesis(const TokenData data);

		virtual ~ParserOp_Parenthesis();

		virtual const Ceng::SharedPtr<ParserOperand> LocalAction(ParserState *state,
																Ceng::SharedPtr<ParserOperand> left,
																Ceng::SharedPtr<ParserOperand> right) override;

	};

}

#endif