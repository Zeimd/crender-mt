/*****************************************************************************
*
* parser-uint64.h
*
* By Jari Korkala 5/2014
*
*****************************************************************************/

#ifndef CASM_PARSER_UINT64_H
#define CASM_PARSER_UINT64_H

#include <limits.h>

#include "parser-literal.h"

namespace Casm
{
	class Literal_Uint64 : public ParserLiteral
	{
	protected:
		Ceng::UINT64 value;

	public:

		static const Ceng::INT64 minValue = 0;
		static const Ceng::UINT64 maxValue = ULLONG_MAX;

		Literal_Uint64();
		Literal_Uint64(const Ceng::UINT64 value,const TokenData &data);
		Literal_Uint64(const Ceng::UINT64 value,const X86::OPERAND_SIZE::value opSize,const TokenData &data);

		virtual ~Literal_Uint64();

		const Ceng::UINT64 Value() const;

		virtual void ToBuffer(void *destBuffer) const override;

		virtual const Ceng::SharedPtr<ParserOperand> BinaryAdd(ParserState *state,
																Ceng::SharedPtr<ParserOperand> left_this,
																Ceng::SharedPtr<ParserOperand> right) override;

		virtual const Ceng::SharedPtr<ParserOperand> BinarySub(ParserState *state,
																Ceng::SharedPtr<ParserOperand> left_this,
																Ceng::SharedPtr<ParserOperand> right) override;


		virtual const Ceng::SharedPtr<ParserOperand> BinaryMul(ParserState *state,
																Ceng::SharedPtr<ParserOperand> left_this,
																Ceng::SharedPtr<ParserOperand> right) override;
		
		
		virtual const Ceng::SharedPtr<ParserOperand> BinaryDiv(ParserState *state,
																Ceng::SharedPtr<ParserOperand> left_this,
																Ceng::SharedPtr<ParserOperand> right) override;
		
		virtual const Ceng::SharedPtr<ParserOperand> operator *= (const Ceng::INT32 value) override;

		virtual const Ceng::BOOL RangeCheck(const Ceng::INT64 minValue,const Ceng::UINT64 maxValue) const override;
		virtual const Ceng::BOOL RangeCheck(const Ceng::FLOAT64 minValue,const Ceng::FLOAT64 maxValue) const override;

		virtual const Ceng::BOOL UnderflowCheck(const Ceng::FLOAT64 minPosValue) const override;

		const Ceng::BOOL IsValueInt() const;

		const Ceng::BOOL IsNegative() const override;

		virtual const Ceng::SharedPtr<ParserOperand> SizeCast(ParserState *state,
																Ceng::SharedPtr<ParserOperand> right_this,
																const Ceng::UINT32 newSize) override;

		virtual operator const Ceng::FLOAT32() const override;
		virtual operator const Ceng::UINT64() const override;
		virtual operator const Ceng::INT64() const override;
	};
}

#endif