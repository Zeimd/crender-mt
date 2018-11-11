/*****************************************************************************
*
* parser-op-mul.cpp
*
* By Jari Korkala 6/2014
*
****************************************************************************/

#include "../include/parser-op-mul.h"

using namespace Casm;

ParserOp_Mul::ParserOp_Mul(const TokenData &data) 
	: ParserOperator(ParserOpType::mul,ParserOpMode::binary,
		ParserOpRange::nearest,ParserOpAssocDir::right,parser_priority_mul,3,data)
{
}

ParserOp_Mul::~ParserOp_Mul()
{
}

const Ceng::SharedPtr<ParserOperand> ParserOp_Mul::LocalAction(ParserState *state,
														Ceng::SharedPtr<ParserOperand> left,
														Ceng::SharedPtr<ParserOperand> right)
{
			

	return left->BinaryMul(state,left,right);
}
