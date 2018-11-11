/*****************************************************************************
*
* symbol-ref.h
*
* By Jari Korkala 2/2014
*
*****************************************************************************/

#ifndef X86_SYMBOL_REF_H
#define X86_SYMBOL_REF_H

#include <datatypes/basic-types.h>
#include <datatypes/return-val.h>
#include <datatypes/shared-ptr.h>

#include "enums/reference-type.h"
#include "enums/x86-operand-sizes.h"

#include "symbol.h"



namespace X86
{
	class SymbolRef
	{
	public:
		Ceng::SharedPtr<Symbol> symbol;
		Ceng::INT64 encodeOffset;
		Ceng::BOOL isUnsigned;

		/**
		 * Offset of IP pointer
		 */
		Ceng::INT64 ipDelta;
		
		OPERAND_SIZE::value encodeSize;
		Casm::REFERENCE_TYPE::value refType;

		SymbolRef(Ceng::SharedPtr<Symbol> symbol,const Ceng::UINT64 offset,
							const OPERAND_SIZE::value encodeSize,
							const Casm::REFERENCE_TYPE::value refType);

		Ceng::CRESULT WriteOffset(const Ceng::UINT64 baseAddress);

		Ceng::CRESULT ConfigIPdelta(const Ceng::UINT64 baseIP);
	};
}

#endif