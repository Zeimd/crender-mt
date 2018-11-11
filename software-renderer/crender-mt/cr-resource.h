/*****************************************************************************
*
* cr-resource.h
*
* Created By Jari Korkala 6/3/2015
*
*****************************************************************************/

#ifndef CENG_CR_RESOURCE_H
#define CENG_CR_RESOURCE_H

#include <ceng/datatypes/return-val.h>

namespace Ceng
{
	class ShaderResourceViewDesc;
	class ShaderResourceView;

	class CR_ResourceCommon
	{
	public:
		CR_ResourceCommon();
		~CR_ResourceCommon();

		const CRESULT GetShaderView(const ShaderResourceViewDesc &viewDesc,
			ShaderResourceView **out_view);

		virtual const CRESULT GetShaderViewTex2D(const ShaderResourceViewDesc &viewDesc,
			ShaderResourceView **out_view)=0;
	};
}

#endif