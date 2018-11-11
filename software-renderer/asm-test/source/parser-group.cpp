/*****************************************************************************
*
* token-group.cpp
*
* By Jari Korkala 5/2014
*
*****************************************************************************/

#include "../include/parser-group.h"
#include "../include/parser-operator.h"
#include "../include/parser-operand.h"

using namespace Casm;

const Ceng::CRESULT TokenGroup::ProcessGroup(ParserState *state)
{
	std::list<Ceng::SharedPtr<Casm::ParserToken>>::iterator iter,next,previous;

	Ceng::INT32 priority = startPriority;

	Ceng::BOOL continueLoop = true;

	if (tokens.size() == 0)
	{
		return Ceng::CE_OK;
	}

	while(continueLoop)
	{
		startPriority = 10000;
		continueLoop = false;

		iter = tokens.begin();

		while(iter != tokens.end())
		{
			if ( (*iter)->TokenType() == Casm::ParserToken::op)
			{
				if (iter != tokens.begin())
				{
					previous = iter;
					--previous;
				}

				next = iter;
				++next;

				/*
				if (next == tokens.end())
				{
					std::wcout << "syntax error" << std::endl;
					break;
				}
				*/

				Ceng::SharedPtr<ParserOperator> op = (*iter);

				if (op->Priority() <= priority)
				{
					Ceng::SharedPtr<ParserOperand> leftSide = nullptr;
					Ceng::SharedPtr<ParserOperand> rightSide = nullptr;

					if (iter != tokens.begin())
					{
						leftSide = *previous;
					}

					if (next != tokens.end())
					{
						rightSide = *next;
					}

					Ceng::SharedPtr<ParserOperand> temp = nullptr;

					temp = op->Action(state,leftSide,rightSide);

					if (temp == nullptr)
					{
						std::wcout << "operator : unavailable" << std::endl;
						//break;
					}
					else
					{	
						if (op->Affects() == 3)
						{
							// For binary operators
							iter = tokens.erase(previous,next);
						}
						else
						{
							// Unary operators
							iter = tokens.erase(iter,next);
						}

						*iter = temp;	
					}
				}
				else
				{
					if (op->Priority() > priority)
					{
						continueLoop = true;

						if (op->Priority() < startPriority)
						{
							startPriority = op->Priority();
						}
					}
				}
			}

			++iter;
		}

		priority = startPriority;
	}

	Ceng::SharedPtr<ParserToken> temp = *tokens.begin();

	if (tokens.size() == 1)
	{
		/*
		if (temp->TokenType() == ParserToken::group)
		{
			TokenGroup *tempGroup = (Casm::TokenGroup*)(Casm::ParserToken*)temp;
			temp = tempGroup->resolved;
		}
		*/
		
		resolved = *tokens.begin();
	}
	else
	{
		resolved = nullptr;
	}

	return Ceng::CE_OK;
}