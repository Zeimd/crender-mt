/*****************************************************************************
*
* symbol-ref.cpp
*
* By Jari Korkala 2/2014
*
*****************************************************************************/

#include "../include/symbol-ref.h"

using namespace X86;

SymbolRef::SymbolRef(Ceng::SharedPtr<Symbol> symbol,const Ceng::UINT64 offset,
					 const OPERAND_SIZE::value encodeSize,
					 const Casm::REFERENCE_TYPE::value refType)
	: symbol(symbol),encodeOffset(offset),encodeSize(encodeSize),refType(refType),
	ipDelta(0)
{
}

Ceng::CRESULT SymbolRef::ConfigIPdelta(const Ceng::UINT64 baseIP)
{
	ipDelta = Ceng::INT64(baseIP - encodeOffset);

	return Ceng::CE_OK;
}

Ceng::CRESULT SymbolRef::WriteOffset(const Ceng::UINT64 baseAddress)
{
	if (refType == Casm::REFERENCE_TYPE::ADDRESS)
	{
		switch(encodeSize)
		{
		case OPERAND_SIZE::DWORD:
			{
				Ceng::UINT32 *ptr = (Ceng::UINT32*)(Ceng::UINT32(baseAddress) + Ceng::UINT32(encodeOffset));
				*ptr = Ceng::UINT32(symbol->Offset());
			}
			break;
		case OPERAND_SIZE::QWORD:
			{
				Ceng::UINT64 *ptr = (Ceng::UINT64*)(Ceng::UINT64(baseAddress) + Ceng::UINT64(encodeOffset));
				*ptr = Ceng::UINT64(symbol->Offset());
			}
			break;
		}
	}
	else
	{
		switch(encodeSize)
		{
		case OPERAND_SIZE::DWORD:
			{
				Ceng::INT32 *ptr = (Ceng::INT32*)(Ceng::UINT32(baseAddress) + Ceng::UINT32(encodeOffset));

				Ceng::INT32 delta = Ceng::INT32(symbol->Offset() - (encodeOffset + ipDelta));

				*ptr = delta;

			}
			break;
		}
	}

	return Ceng::CE_OK;
}