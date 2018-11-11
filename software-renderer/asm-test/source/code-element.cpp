/*****************************************************************************
*
* code-element.cpp
*
* By Jari Korkala 1/2014
*
*****************************************************************************/

#include "../include/code-element.h"

#include "../include/code-label.h"

using namespace X86;

CodeElement::CodeElement(const Ceng::UINT32 position) : position(position)
{
}

CodeElement::~CodeElement()
{
}

Ceng::CRESULT CodeElement::Build(BuildParams *params,
								 const std::vector<Ceng::SharedPtr<Label>> &labels,
								 const std::vector<Ceng::SharedPtr<CodeElement>> &codeList)
{
	return Ceng::CE_OK;
}

Ceng::CRESULT CodeElement::Append(std::vector<Ceng::UINT8> &destBuffer) const
{
	return Ceng::CE_OK;
}

Ceng::UINT32 CodeElement::SizeBytes() const
{
	return 0;
}