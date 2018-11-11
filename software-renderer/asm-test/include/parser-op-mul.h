/*****************************************************************************
*
* parser-op-mul.h
*
* By Jari Korkala 6/2014
*
*****************************************************************************/

#ifndef CASM_PARSER_OP_MUL_H
#define CASM_PARSER_OP_MUL_H

#include "parser-operator.h"

namespace Casm
{
	class ParserOp_Mul : public ParserOperator
	{
	public:

		ParserOp_Mul(const TokenData &data);

		virtual ~ParserOp_Mul();

		virtual const Ceng::SharedPtr<ParserOperand> LocalAction(ParserState *state,
																Ceng::SharedPtr<ParserOperand> left,
																Ceng::SharedPtr<ParserOperand> right);	
	};

}

#endif