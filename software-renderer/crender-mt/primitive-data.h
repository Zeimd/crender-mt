/*****************************************************************************
*
* primitive-data.h
*
* Created By Jari Korkala 31/8/2014
*
*****************************************************************************/

#ifndef CRENDER_PRIMITIVE_DATA_H
#define CRENDER_PRIMITIVE_DATA_H

#include <ceng/datatypes/basic-types.h>
#include <ceng/enums/primitive-type.h>

#include "render-state.h"

namespace Ceng
{
	class CR_PrimitiveData
	{
	public:

		PRIMITIVE_TYPE::value primitiveType;

		Ceng::UINT8* fragmentPtr[4];
	};
}

#endif