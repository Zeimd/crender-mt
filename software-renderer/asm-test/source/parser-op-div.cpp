/*****************************************************************************
*
* parser-op-div.cpp
*
* By Jari Korkala 6/2014
*
*****************************************************************************/

#include "../include/parser-op-div.h"

using namespace Casm;

ParserOp_Div::ParserOp_Div(const TokenData &data) 
	: ParserOperator(ParserOpType::div,ParserOpMode::binary,
		ParserOpRange::nearest,ParserOpAssocDir::right,parser_priority_mul,3,data)
{
}

ParserOp_Div::~ParserOp_Div()
{
}

const Ceng::SharedPtr<ParserOperand> ParserOp_Div::LocalAction(ParserState *state,
														Ceng::SharedPtr<ParserOperand> left,
														Ceng::SharedPtr<ParserOperand> right)
{
	return left->BinaryDiv(state,left,right);
}
