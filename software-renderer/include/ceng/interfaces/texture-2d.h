/*****************************************************************************
*
* texture-2d.h
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#ifndef _CENG_TEXTURE_2D_H
#define _CENG_TEXTURE_2D_H

#include "../datatypes/return-val.h"

#include "render-resource.h"

namespace Ceng
{
	struct BufferData2D;

	class RenderTarget;

	class Texture2D : public RenderResource
	{
	protected:
		
		bool renderTarget;
		
		virtual ~Texture2D()
		{
		}
	
	public:
		Texture2D()
		{
			renderTarget = false;
		}
		
		virtual const CRESULT GetBufferData2D(Ceng::BufferData2D *dataPtr)=0;
		virtual const CRESULT GetRenderTarget(Ceng::RenderTarget **targetPtr)=0;
	};
	
};

#endif