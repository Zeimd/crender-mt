/*****************************************************************************
*
* d3d9-adapter.h
*
* Created By Jari Korkala 3/3/2015
*
*****************************************************************************/

#ifndef CENG_D3D9_ADAPTER_H
#define CENG_D3D9_ADAPTER_H

#include <d3d9.h>
#include <d3d9types.h>

#include <ceng/interfaces/graphics-adapter.h>

namespace Ceng
{
	class Direct3D9_Adapter : public GraphicsAdapter
	{
	public:

		Ceng::UINT32 index;
		D3DADAPTER_IDENTIFIER9 d3d_adapter;

		virtual ~Direct3D9_Adapter();
	public:

		Direct3D9_Adapter();
		Direct3D9_Adapter(const Ceng::UINT32 index,const D3DADAPTER_IDENTIFIER9 &source);

		virtual void Release() override;

		virtual const CRESULT GetDesc(GraphicsAdapterDesc &desc) override;
		
	};
}

#endif