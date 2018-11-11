/*****************************************************************************
*
* parser-op-div.h
*
* By Jari Korkala 6/2014
*
*****************************************************************************/

#ifndef CASM_PARSER_OP_DIV_H
#define CASM_PARSER_OP_DIV_H

#include "parser-operator.h"

namespace Casm
{
	class ParserOp_Div : public ParserOperator
	{
	public:

		ParserOp_Div(const TokenData &data);

		virtual ~ParserOp_Div();

		virtual const Ceng::SharedPtr<ParserOperand> LocalAction(ParserState *state,
																Ceng::SharedPtr<ParserOperand> left,
																Ceng::SharedPtr<ParserOperand> right);	
	};

}

#endif