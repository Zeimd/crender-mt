/*****************************************************************************
*
* object-code.h
*
* By Jari Korkala 2/2014
*
*****************************************************************************/

#ifndef X86_OBJECT_CODE_H
#define X86_OBJECT_CODE_H

#include "obj-function.h"
#include "symbol.h"

#include <datatypes/basic-types.h>
#include <datatypes/shared-ptr.h>


namespace X86
{
	class ObjectCode
	{
	public:

		std::vector<Ceng::SharedPtr<Symbol>> *dataSection;

		std::vector<Ceng::SharedPtr<ObjectFunction>> *functions;

		ObjectCode(std::vector<Ceng::SharedPtr<Symbol>> *dataSection,
			std::vector<Ceng::SharedPtr<ObjectFunction>> *functions)
			: dataSection(dataSection),functions(functions)
		{
		}

		~ObjectCode()
		{
			if (dataSection != nullptr)
			{
				delete dataSection;
			}

			if (functions != nullptr)
			{
				delete functions;
			}
		}
	};
}

#endif