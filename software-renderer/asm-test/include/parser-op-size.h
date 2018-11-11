/*****************************************************************************
*
* parser-op-size.h
*
* By Jari Korkala 6/2014
*
*****************************************************************************/

#ifndef CASM_PARSER_OP_SIZE_H
#define CASM_PARSER_OP_SIZE_H

#include "parser-operator.h"

namespace Casm
{
		class ParserOp_SizeCast : public ParserOperator
	{
	protected:

		const Ceng::UINT32 newSize;

	public:

		ParserOp_SizeCast(const Ceng::UINT32 newSize,const TokenData &data);

		virtual ~ParserOp_SizeCast();

		virtual const Ceng::SharedPtr<ParserOperand> LocalAction(ParserState *state,
																Ceng::SharedPtr<ParserOperand> left,
																Ceng::SharedPtr<ParserOperand> right);
	};

}

#endif