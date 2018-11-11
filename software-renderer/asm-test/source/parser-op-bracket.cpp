/*****************************************************************************
*
* parser-op-bracket.cpp
*
* By Jari Korkala 6/2014
*
*****************************************************************************/

#include "../include/parser-op-bracket.h"

using namespace Casm;

ParserOp_Parenthesis::ParserOp_Parenthesis(const TokenData data) 
	: ParserOperator(ParserOpType::unary_parenthesis,ParserOpMode::unary_prefix,ParserOpRange::nearest,
				ParserOpAssocDir::right,parser_priority_parenthesis,2,data)
{
}

ParserOp_Parenthesis::~ParserOp_Parenthesis()
{
}

const Ceng::SharedPtr<ParserOperand> ParserOp_Parenthesis::LocalAction(ParserState *state,
																	   Ceng::SharedPtr<ParserOperand> left,
																	   Ceng::SharedPtr<ParserOperand> right)
{
	if (right->TokenType() == ParserToken::group)
	{
		Ceng::SharedPtr<TokenGroup> temp = right;

		if (temp->resolved == nullptr)
		{
			// Priority parenthesis with multiple terms

			return temp;
		}

		else if (temp->resolved->TokenType() == ParserToken::datatype)
		{
			// C-style cast operator

			//return new ParserOp_CstyleCast(temp->resolved->DataType(),data);
		}
		else
		{
			// Priority parenthesis with single term

			return temp->resolved;
		}
	}

	return nullptr;
}
