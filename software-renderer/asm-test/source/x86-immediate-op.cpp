/*****************************************************************************
*
* x86-immediate-op.cpp
*
* By Jari Korkala 2/2014
*
*****************************************************************************/

#include "../include/x86-immediate-op.h"

#include "../include/encode-data.h"

using namespace X86;

ImmediateOperand::ImmediateOperand(Ceng::SharedPtr<Symbol> &symbol) :
	Operand(OPERAND_TYPE::IMM,false,OPERAND_SIZE::IMPLICIT)
{
	this->symbol = symbol;
	bytes = 0;
	value = 0;
}

ImmediateOperand::ImmediateOperand(const Ceng::INT64 value) :
	Operand(OPERAND_TYPE::IMM,false,OPERAND_SIZE::IMPLICIT),
	value(value),bytes(8)
{
	if (value == 0)
	{
		size = OPERAND_SIZE::BYTE;
		bytes = 1;
	}
	else if (value >= -128 && value <= 127)
	{
		bytes = 1;
		size = OPERAND_SIZE::BYTE;
	}
	else if (value >= -32768 && value <= 32767)
	{
		bytes = 2;
		size = OPERAND_SIZE::WORD;
	}
	else if (value >= -(Ceng::INT64(1) << 32) && value <= ((Ceng::INT64(1)<<32)-1))
	{
		bytes = 4;
		size = OPERAND_SIZE::DWORD;
	}
	else
	{
		bytes = 8;
		size = OPERAND_SIZE::QWORD;
	}
}

ImmediateOperand::ImmediateOperand(const OPERAND_SIZE::value size,const Ceng::INT64 value) :
	Operand(OPERAND_TYPE::IMM,false,size),
	value(value),bytes(8)
{
	switch(size)
	{
	case OPERAND_SIZE::BYTE:
		bytes = 1;
		this->size = OPERAND_SIZE::BYTE;
		break;
	case OPERAND_SIZE::WORD:
		this->size = OPERAND_SIZE::WORD;
		bytes = 2;
		break;
	case OPERAND_SIZE::DWORD:
		this->size = OPERAND_SIZE::DWORD;
		bytes = 4;
		break;
	case OPERAND_SIZE::QWORD:
		this->size = OPERAND_SIZE::QWORD;
		bytes = 8;
		break;
	}
}

const Ceng::INT32 ImmediateOperand::SizeBytes() const
{
	return bytes;
}
		
const Ceng::INT8 ImmediateOperand::Value8() const
{
	return Ceng::INT8(value);
}

const Ceng::INT16 ImmediateOperand::Value16() const
{
	return Ceng::INT16(value);
}

const Ceng::INT32 ImmediateOperand::Value32() const
{
	return Ceng::INT32(value);
}

const Ceng::INT64 ImmediateOperand::Value64() const
{
	return Ceng::INT64(value);
}

const Ceng::SharedPtr<Symbol> ImmediateOperand::GetSymbol() const
{
	return symbol;
}

const Ceng::CRESULT ImmediateOperand::EncodeAsOperand(BuildParams *params,EncodeData *encodeData,
													  const Ceng::UINT32 operandIndex) const
{
	if (encodeData->operandPlace[operandIndex] == OPERAND_PLACE::NOT_USED)
	{
		return Ceng::CE_OK;
	}

	encodeData->hasImmediate = true;
	encodeData->immediate = value;

	return Ceng::CE_OK;
}