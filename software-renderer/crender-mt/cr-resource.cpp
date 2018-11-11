/*****************************************************************************
*
* cr-resource.cpp
*
* Created By Jari Korkala 6/3/2015
*
*****************************************************************************/

#include <ceng/datatypes/shader-resource-view-desc.h>

#include "cr-resource.h"

using namespace Ceng;

CR_ResourceCommon::CR_ResourceCommon()
{
}

CR_ResourceCommon::~CR_ResourceCommon()
{
}

const CRESULT CR_ResourceCommon::GetShaderView(const ShaderResourceViewDesc &viewDesc,
	ShaderResourceView **out_view)
{
	switch (viewDesc.dimensions)
	{
	case BufferType::texture_2d:
		return GetShaderViewTex2D(viewDesc, out_view);
	}

	return CE_ERR_INVALID_PARAM;
}
