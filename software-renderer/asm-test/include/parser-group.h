/*****************************************************************************
*
* parser-group.h
*
* By Jari Korkala 5/2014
*
*****************************************************************************/

#ifndef CASM_PARSER_GROUP_H
#define CASM_PARSER_GROUP_H

#include <datatypes/return-val.h>

#include "parser-operand.h"

namespace Casm
{
	class TokenGroup : public ParserOperand
	{
	public:

		std::list<Ceng::SharedPtr<ParserToken>> tokens;

		const char bracketType;

		/**
		 * External operators can operate on each term separately.
		 */
		Ceng::BOOL allowSplit;

		Ceng::INT32 startPriority;

		Ceng::SharedPtr<ParserToken> resolved;

	protected:

		TokenGroup() : ParserOperand(ParserToken::group,ParserBasicType::unknown,0,Casm::TokenData()),bracketType('\0')
		{
		}

	public:

		TokenGroup(const char bracketType,const Ceng::BOOL allowSplit,const TokenData &data) 
			: ParserOperand(ParserToken::group,ParserBasicType::unknown,0,data),bracketType(bracketType)
		{
			this->allowSplit = allowSplit;
			startPriority = 10000;
			resolved = nullptr;
		}

		virtual ~TokenGroup()
		{

		}

		const Ceng::CRESULT ProcessGroup(ParserState *state);

		Ceng::SharedPtr<ParserOperand> GetLiteralOperand(Ceng::SharedPtr<ParserToken> token);

	};
}

#endif