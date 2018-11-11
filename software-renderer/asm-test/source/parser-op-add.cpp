/*****************************************************************************
*
* parser-op-add.cpp
*
* By Jari Korkala 6/2014
*
*****************************************************************************/

#include "../include/parser-op-add.h"

using namespace Casm;

ParserOp_Add::ParserOp_Add(const TokenData &data) 
	: ParserOperator(ParserOpType::add,ParserOpMode::binary,
		ParserOpRange::nearest,ParserOpAssocDir::right,parser_priority_add,3,data)
{
}

ParserOp_Add::~ParserOp_Add()
{
}

const Ceng::SharedPtr<ParserOperand> ParserOp_Add::LocalAction(ParserState *state,
														Ceng::SharedPtr<ParserOperand> left,
														Ceng::SharedPtr<ParserOperand> right)
{
	return left->BinaryAdd(state,left,right);
}

//***************************************************************************************
// Unary add

ParserOp_Unary_Add::ParserOp_Unary_Add(const TokenData &data) 
	: ParserOperator(ParserOpType::unary_add,ParserOpMode::unary_prefix,
		ParserOpRange::nearest,ParserOpAssocDir::left,parser_priority_unary_add,2,data)
{
}

ParserOp_Unary_Add::~ParserOp_Unary_Add()
{
}

const Ceng::SharedPtr<ParserOperand> ParserOp_Unary_Add::LocalAction(ParserState *state,
														Ceng::SharedPtr<ParserOperand> left,
														Ceng::SharedPtr<ParserOperand> right)
{
	return right;
}
