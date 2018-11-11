/*****************************************************************************
*
* shader-const-data.h
*
* Created By Jari Korkala 26/8/2014
*
*****************************************************************************/

#ifndef CRENDER_SHADER_CONST_DATA_H
#define CRENDER_SHADER_CONST_DATA_H

#include <ceng/datatypes/ce-string.h>

#include <ceng/enums/shader-datatype.h>

#include "crender-base.h"

namespace Ceng
{
	/**
	 * Shader constant information.
	 */
	class CR_ShaderConstant
	{
	public:
		/**
		 * Name of the constant in the shader code.
		 */
		String name;

		/**
		 * Data type.
		 */
		Ceng::SHADER_DATATYPE::value dataType;

		/**
		 * Address of the constant in the shader's
		 * constant buffer.
		 */
		UINT32 bufferOffset;

		/**
		 * Size of the constant in bytes.
		 */
		UINT32 size;

	public:


	};
}

#endif