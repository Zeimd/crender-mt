/*****************************************************************************
*
* basic-block.cpp
*
* By Jari Korkala 1/2014
*
*****************************************************************************/

#include "../include/basic-block.h"

using namespace X86;

BasicBlock::BasicBlock(const Ceng::UINT32 position) : CodeElement(position)
{
	type = BASIC_BLOCK;
}

BasicBlock::~BasicBlock()
{
}

Ceng::CRESULT BasicBlock::AddLine(CodeLine *newLine)
{
	lines.push_back(newLine);

	return Ceng::CE_OK;
}

Ceng::CRESULT BasicBlock::Build(BuildParams *params,
								const std::vector<Ceng::SharedPtr<Label>> &labels,
								const std::vector<Ceng::SharedPtr<CodeElement>> &codeList)
{
	Ceng::INT32 k;

	Ceng::CRESULT cresult;

	params->codeElement = this;

	for(k=0;k<lines.size();k++)
	{
		cresult = lines[k]->Encode(params,codeBuffer);
		if (cresult != Ceng::CE_OK)
		{
			//codeBuffer.clear();
			return cresult;
		}

		// Store base offset for IP-relative addressing
	
		if (params->immRef != nullptr)
		{
			params->immRef->ConfigIPdelta(codeBuffer.size());
		}

		if (params->memRef != nullptr)
		{
			params->memRef->ConfigIPdelta(codeBuffer.size());
		}

		params->Clean();

	}

	return Ceng::CE_OK;
}

Ceng::UINT32 BasicBlock::SizeBytes() const
{
	return codeBuffer.size();
}

Ceng::CRESULT BasicBlock::Append(std::vector<Ceng::UINT8> &destBuffer) const
{
	std::copy(codeBuffer.begin(),codeBuffer.end(),std::back_inserter(destBuffer));

	return Ceng::CE_OK;
}