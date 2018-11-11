/*****************************************************************************
*
* parser-op-add.h
*
* By Jari Korkala 6/2014
*
*****************************************************************************/

#ifndef CASM_PARSER_OP_ADD_H
#define CASM_PARSER_OP_ADD_H

#include "parser-operator.h"

namespace Casm
{
	class ParserOp_Add : public ParserOperator
	{
	public:

		ParserOp_Add(const TokenData &data);

		virtual ~ParserOp_Add();

		virtual const Ceng::SharedPtr<ParserOperand> LocalAction(ParserState *state,
																Ceng::SharedPtr<ParserOperand> left,
																Ceng::SharedPtr<ParserOperand> right);
	};

	class ParserOp_Unary_Add : public ParserOperator
	{
	public:

		ParserOp_Unary_Add(const TokenData &data);

		virtual ~ParserOp_Unary_Add();

		virtual const Ceng::SharedPtr<ParserOperand> LocalAction(ParserState *state,
																Ceng::SharedPtr<ParserOperand> left,
																Ceng::SharedPtr<ParserOperand> right);
	};

}

#endif