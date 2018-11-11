/*****************************************************************************
*
* cr-shader-const.h
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#ifndef _CENG_CR_SHADER_CONST_H
#define _CENG_CR_SHADER_CONST_H

#include <ceng/interfaces/shader-const.h>

#include "shader-const-data.h"

#include "shader-wrapper.h"

namespace Ceng
{
	

	class CR_ShaderConstHandle : public Ceng::ShaderConstant
	{
	protected:

		Ceng::UINT32 index;

		ShaderWrapper *shader;

	public:

		CR_ShaderConstHandle();

		CR_ShaderConstHandle(const Ceng::UINT32 index,ShaderWrapper *shader);

		virtual ~CR_ShaderConstHandle();

		virtual void Release() override;

		/**
		 * Writes the constant into a buffer.
		 *
		 * @param *dest Target buffer.
		 */
		virtual const CRESULT GetValue(void *dest) override;

		/**
		 * Sets the constant.
		 *
		 * @param *source Pointer to a buffer that contains the value.
		 */
		virtual const CRESULT SetValue(void *source) override;
		
	};
};

#endif