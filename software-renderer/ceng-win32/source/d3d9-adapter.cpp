/*****************************************************************************
*
* d3d9-adapter.cpp
*
* Created By Jari Korkala 3/3/2015
*
*****************************************************************************/

#include "../include/d3d9-adapter.h"

#include <ceng/datatypes/graphics-adapter-desc.h>

using namespace Ceng;

Direct3D9_Adapter::Direct3D9_Adapter()
{
}

Direct3D9_Adapter::Direct3D9_Adapter(const Ceng::UINT32 index, const D3DADAPTER_IDENTIFIER9 &source)
	:index(index)
{
	memcpy(&d3d_adapter, &source, sizeof(D3DADAPTER_IDENTIFIER9));
}

Direct3D9_Adapter::~Direct3D9_Adapter()
{
}

void Direct3D9_Adapter::Release()
{
	delete this;
}

const CRESULT Direct3D9_Adapter::GetDesc(GraphicsAdapterDesc &desc)
{
	desc.description = d3d_adapter.Description;
	return CE_OK;
}