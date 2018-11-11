/*****************************************************************************
*
* parser-opcode.cpp
*
* By Jari Korkala 5/2014
*
*****************************************************************************/

#include "../include/parser-opcode.h"

#include "../include/parser-group.h"
#include "../include/parser-literal.h"
#include "../include/parser-register.h"
#include "../include/parser-memory.h"

#include "../include/x86-immediate-op.h"
#include "../include/x86-memory-op.h"
#include "../include/x86-register-op.h"

#include "../include/parser-instruction.h"

using namespace Casm;

ParserOpcode::ParserOpcode(const X86::Instruction *instruction,const TokenData &data) 
	: ParserOperator(ParserOpType::opcode,ParserOpMode::unary_prefix,ParserOpRange::nearest,
				ParserOpAssocDir::right,parser_priority_opcode,2,data),instruction(instruction)
{
}

ParserOpcode::~ParserOpcode()
{
}

const Ceng::SharedPtr<ParserOperand> ParserOpcode::Action(ParserState *state,
														  Ceng::SharedPtr<ParserOperand> left,
														  Ceng::SharedPtr<ParserOperand> right)
{
	return LocalAction(state,left,right);
}

const Ceng::SharedPtr<ParserOperand> ParserOpcode::LocalAction(ParserState *state,
															   Ceng::SharedPtr<ParserOperand> left,
															   Ceng::SharedPtr<ParserOperand> right)
{
	if (right->TokenType() != ParserToken::group)
	{
		return nullptr;
	}

	std::vector<const X86::Operand*> operands;

	const X86::Operand* tempOperand = nullptr;

	TokenGroup *rhs = (TokenGroup*)(ParserOperand*)right;

	auto iter = rhs->tokens.begin();

	while(iter != rhs->tokens.end())
	{
		tempOperand = nullptr;

		if ( (*iter)->TokenType() == Casm::ParserToken::literal_value)
		{
			Casm::ParserLiteral *literal = (Casm::ParserLiteral*)(Casm::ParserToken*)*iter;
		
			if (literal->Datatype() == Casm::ParserBasicType::float32)
			{
				Ceng::FLOAT32 floatVal[2];

				literal->ToBuffer(&floatVal[0]);

				tempOperand = new X86::ImmediateOperand(literal->OpSize(),*(Ceng::INT64*)floatVal);
			}
			else
			{
				Ceng::UINT64 int64val;

				literal->ToBuffer(&int64val);

				tempOperand = new X86::ImmediateOperand(literal->OpSize(),int64val);
			}		
		}
		else if ( (*iter)->TokenType() == Casm::ParserToken::memoryOp)
		{
			Casm::ParserMemory *mem = (Casm::ParserMemory*)(Casm::ParserToken*)*iter;

			tempOperand = new X86::MemoryOperand(mem->OpSize(),mem->BaseReg(),mem->IndexReg(),mem->IndexScale(),mem->Offset());
		}
		else if ( (*iter)->TokenType() == Casm::ParserToken::registerOp)
		{
			Casm::ParserRegister *reg = (Casm::ParserRegister*)(Casm::ParserToken*)*iter;

			tempOperand = reg->RegisterOp();
		}
		else if ( (*iter)->TokenType() == Casm::ParserToken::op)
		{
			Casm::ParserOperator *op = (Casm::ParserOperator*)(Casm::ParserToken*)*iter;

			if (op->OperatorType() != ParserOpType::separator)
			{
				return nullptr;
			}
		}

		if (tempOperand != nullptr)
		{
			operands.push_back(tempOperand);
		}

		++iter;
	}

	X86::CodeLine *line;

	switch(operands.size())
	{
	case 0:
		line = new X86::BareOp(*instruction);
		break;
	case 1:
		line = new X86::UnaryOp(*instruction,operands[0]);
		break;
	case 2:
		line = new X86::BinaryOp(*instruction,operands[0],operands[1]);
		break;
	case 3:
		line = new X86::ThreeOp(*instruction,operands[0],operands[1],operands[2]);
		break;
	default:
		return nullptr;
	}

	return new ParserInstruction(line,data);
}