/*****************************************************************************
*
* x86-opcode-map.h
*
* By Jari Korkala 6/2014
*
*****************************************************************************/

#ifndef CASM_X86_OPCODE_MAP_H
#define CASM_X86_OPCODE_MAP_H

#include "keyword-map.h"
#include "x86-instruction.h"

#include <datatypes/shared-ptr.h>

#include <string>
#include <unordered_map>

namespace X86
{
	class OpcodeMap : public Casm::KeywordMap
	{
	protected:

		static Ceng::SharedPtr<OpcodeMap> instance;

		std::unordered_map<std::string,const Instruction*> opcodeMap;

		OpcodeMap();

	public:
		
		virtual ~OpcodeMap();

		virtual Casm::ParserToken* FindKeyword(const Ceng::String &name) const;
		
		static Ceng::SharedPtr<OpcodeMap> GetInstance();

		const X86::Instruction* FindOpcode(const Ceng::String &name) const;

	protected:

		void BuildMap();
	};
}


#endif