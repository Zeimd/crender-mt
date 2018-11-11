/*****************************************************************************
*
* depth-test-port.cpp
*
* By Jari Korkala 11/2012
*
*****************************************************************************/

#include "rasterizer-cr.h"

using namespace Ceng;

namespace Ceng
{
	void DEPTH_STENCIL_EMPTY(void *depthBuffer,void *stencilBuffer,
							 void *depthValues,
							 const void *depthStepX,const void *depthStepY,
							 const void *stencilRef,
							 const void *stencilWriteMask,const void *stencilReadMask,											
							 const Ceng::BOOL depthWrite,
							 const UINT64 tileMask,
							 UINT64 *depthMask)
	{
		*depthMask = tileMask;
	}

	//*****************************************************************************
	// format = DF32 , no stencil

	void DF32_LEQ(void *depthBuffer,void *stencilBuffer,
				  void *depthValues,const void *depthStepX,
				  const void *depthStepY,const void *stencilRef,
				  const void *stencilWriteMask,const void *stencilReadMask,											
				  const Ceng::BOOL depthWrite,
				  const UINT64 tileMask,
				  UINT64 *depthMask)
	{
		FLOAT32 *depthPtr = (FLOAT32*)depthBuffer;
		VectorF4 *packedZ = (VectorF4*)depthValues;
		
		VectorF4 *stepX = (VectorF4*)depthStepX;
		VectorF4 *stepY = (VectorF4*)depthStepY;
		
		UINT32 quadX,quadY;
		INT32 shiftValue = 0;
		
		UINT32 quadMask = 0;
		UINT32 depthTemp = 0;
		
		UINT32 depthIndex = 0;
		
		*depthMask = 0;
		
		for(quadY=0;quadY<8;quadY+=2)
		{
			for(quadX=0;quadX<8;quadX+=2)
			{
				quadMask = UINT32(tileMask >> shiftValue) & 15;
				depthTemp = 0;
				
				if (quadMask & 1)
				{
					if (packedZ->x <= depthPtr[depthIndex])
					{
						if (depthWrite)
						{
							depthPtr[depthIndex] = packedZ->x;
						}

						depthTemp += 1;
					}
				}
				
				if (quadMask & 2)
				{
					if (packedZ->y <= depthPtr[depthIndex+1])
					{
						if (depthWrite)
						{
							depthPtr[depthIndex+1] = packedZ->y;
						}

						depthTemp += 2;
					}
				}
				
				if (quadMask & 4)
				{
					if (packedZ->z <= depthPtr[depthIndex+2])
					{
						if (depthWrite)
						{
							depthPtr[depthIndex+2] = packedZ->z;
						}

						depthTemp += 4;
					}
				}
				
				if (quadMask & 8)
				{
					if (packedZ->w <= depthPtr[depthIndex+3])
					{
						if (depthWrite)
						{
							depthPtr[depthIndex+3] = packedZ->w;
						}

						depthTemp += 8;
					}
				}
				
				*depthMask += (UINT64(depthTemp) << shiftValue);
				
				shiftValue += 4;
				depthIndex += 4;
				
				if (quadX < 6)
				{
					*packedZ += *stepX;
				}
			}
			
			if (quadY < 6)
			{
				*packedZ += *stepY;
			}
		}
	}
}