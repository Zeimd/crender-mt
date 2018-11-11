/*****************************************************************************
*
* code-element.h
*
* By Jari Korkala 1/2014
*
*****************************************************************************/

#ifndef X86_CODE_ELEMENT_H
#define X86_CODE_ELEMENT_H

#include <datatypes/basic-types.h>

#include "build-params.h"

#include "symbol-ref.h"

namespace X86
{
	class Label;
	class ProgramBuilder;

	class CodeElement
	{
	public:
		enum TYPE
		{
			BASIC_BLOCK = 1 ,
			CONDITION_JUMP = 2 ,

			FORCE_32B = 1 << 30 ,
		};

	protected:
		Ceng::UINT32 position;
		TYPE type;

		CodeElement()
		{
		}

	public:

		Ceng::INT64 offset;

		std::vector<Ceng::SharedPtr<SymbolRef>> references;

		CodeElement(const Ceng::UINT32 position);

		virtual ~CodeElement();	

		virtual Ceng::CRESULT Build(BuildParams *params,
									 const std::vector<Ceng::SharedPtr<Label>> &labels,
									 const std::vector<Ceng::SharedPtr<CodeElement>> &codeList);
	
		virtual Ceng::CRESULT Append(std::vector<Ceng::UINT8> &destBuffer) const;

		Ceng::UINT32 Position() const;

		TYPE Type() const;

		virtual Ceng::UINT32 SizeBytes() const;
	};

	inline Ceng::UINT32 CodeElement::Position() const
	{
		return position;
	}

	inline CodeElement::TYPE CodeElement::Type() const
	{
		return type;
	}
}

#endif