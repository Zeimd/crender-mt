/*****************************************************************************
*
* shader-const.h
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#ifndef _CENG_SHADER_CONST_H
#define _CENG_SHADER_CONST_H

#include "../datatypes/return-val.h"

#include "base-interface.h"

namespace Ceng
{
	class ShaderConstant : public BASE_INTERFACE
	{
	protected:
		virtual ~ShaderConstant()
		{
		}
	
	public:
		ShaderConstant()
		{
		}
		
		virtual const CRESULT GetValue(void *dest)=0;
		virtual const CRESULT SetValue(void *source)=0;
	};

};

#endif