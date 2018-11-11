/*****************************************************************************
*
* keyword-group.h
*
* By Jari Korkala 6/2014
*
*****************************************************************************/

#ifndef CASM_KEYWORD_GROUP_H
#define CASM_KEYWORD_GROUP_H

#include "keyword-map.h"

#include <datatypes/shared-ptr.h>

#include <list>

namespace Casm
{
	class KeywordGroup : public KeywordMap
	{
	public:

		std::list<Ceng::SharedPtr<KeywordMap>> groups;

	public:

		KeywordGroup();

		virtual ~KeywordGroup();

		virtual ParserToken* FindKeyword(const Ceng::String &name) const;
	};
}

#endif