/*****************************************************************************
*
* rtarget-clear.cpp
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#include "rtarget-data.h"

using namespace Ceng;

CRESULT CR_NewTargetData::ClearTarget(const CE_Color &color,const Ceng::Rectangle *scissorRect)
{
	// Clip scissor rectangle against render target boundaries

	Rectangle localRect;

	localRect.top = 0;
	localRect.left = 0;

	localRect.bottom = bufferHeight;
	localRect.right = bufferWidth;

	if (scissorRect != NULL)
	{
		if (scissorRect->top < localRect.bottom)
		{
			if (scissorRect->top > localRect.top)
			{
				localRect.top = scissorRect->top;
			}
		}

		if (scissorRect->left < localRect.right)
		{
			if (scissorRect->left > localRect.left)
			{
				localRect.left = scissorRect->left;
			}
		}

		if (scissorRect->bottom > localRect.top)
		{
			if (scissorRect->bottom < localRect.bottom)
			{
				localRect.bottom = scissorRect->bottom;
			}
		}

		if (scissorRect->right > localRect.left)
		{
			if (scissorRect->right < localRect.right)
			{
				localRect.right = scissorRect->right;
			}
		}
	}

	// NOTE: Assumes BUFFER_TILING::LQUADS and ARGB32

	_declspec(align(16)) UINT32 colorValueVec[4];

	UINT32 tempColor = color.ExtractColor32();
	UINT8 *channel = (UINT8*)&tempColor;

	UINT8 *colorVec = (UINT8*)&colorValueVec[0];

	colorVec[0] = channel[0];
	colorVec[1] = channel[0];
	colorVec[2] = channel[0];
	colorVec[3] = channel[0];

	// NOTE: swap green and red so that post-processing
	//       is faster

	colorVec[4+0] = channel[2];
	colorVec[4+1] = channel[2];
	colorVec[4+2] = channel[2];
	colorVec[4+3] = channel[2];

	colorVec[8+0] = channel[1];
	colorVec[8+1] = channel[1];
	colorVec[8+2] = channel[1];
	colorVec[8+3] = channel[1];

	colorVec[12+0] = channel[3];
	colorVec[12+1] = channel[3];
	colorVec[12+2] = channel[3];
	colorVec[12+3] = channel[3];

	INT32 width = localRect.right - localRect.left;
	INT32 height = localRect.bottom - localRect.top;


	INT32 xFullStart = (localRect.left + 1) >> 1; // Ceil to even
	INT32 xFullEnd = localRect.right >> 1; // Floor to even

	INT32 yFullStart = (localRect.top + 1) >> 1;
	INT32 yFullEnd = localRect.bottom >> 1;

	UINT32 k,j;

	UINT32 *colorAddress;
	UINT8 *partialWrite;

	if (localRect.top & 1)
	{
		// Start from first row with any coverage

		partialWrite = (UINT8*)(baseAddress + (localRect.top >> 1)*tileYstep);

		if (localRect.left & 1)
		{
			// Write bottom-right pixel of a quad

			j = localRect.left >> 1;

			partialWrite[16*j+3] = channel[0];
			partialWrite[16*j+4+3] = channel[2];
			partialWrite[16*j+8+3] = channel[1];
			partialWrite[16*j+12+3] = channel[3];
		}

		for(j = xFullStart; j < xFullEnd; j++)
		{
			// Write bottom-left and bottom-right pixels of a quad

			partialWrite[16*j+2] = channel[0];
			partialWrite[16*j+3] = channel[0];

			partialWrite[16*j+4+2] = channel[2];
			partialWrite[16*j+4+3] = channel[2];

			partialWrite[16*j+8+2] = channel[1];
			partialWrite[16*j+8+3] = channel[1];

			partialWrite[16*j+12+2] = channel[3];
			partialWrite[16*j+12+3] = channel[3];
		}

		if (localRect.right & 1)
		{
			// NOTE: localRect.right is exclusive, so right-1 is last
			//       filled coordinate

			// Write bottom-left pixel of a quad

			j = (localRect.right-1) >> 1;

			partialWrite[16*j+2] = channel[0];
			partialWrite[16*j+4+2] = channel[2];
			partialWrite[16*j+8+2] = channel[1];
			partialWrite[16*j+12+2] = channel[3];
		}
	}

	for(k = yFullStart; k < yFullEnd; k++)
	{
		colorAddress = (UINT32*)(baseAddress + k*tileYstep);

		if (localRect.left & 1)
		{
			// Write top-right and bottom-right pixels of a quad

			partialWrite = (UINT8*)&colorAddress[4*(localRect.left>>1)];
			
			partialWrite[1] = channel[0];
			partialWrite[3] = channel[0];

			partialWrite[4+1] = channel[2];
			partialWrite[4+3] = channel[2];

			partialWrite[8+1] = channel[1];
			partialWrite[8+3] = channel[1];
			
			partialWrite[12+1] = channel[3];
			partialWrite[12+3] = channel[3];
		}

		for(j = xFullStart; j < xFullEnd; j++)
		{
			// Write full quads
			colorAddress[4*j] = colorValueVec[0];
			colorAddress[4*j+1] = colorValueVec[1];
			colorAddress[4*j+2] = colorValueVec[2];
			colorAddress[4*j+3] = colorValueVec[3];
		}

		if (localRect.right & 1)
		{
			j = (localRect.right-1) >> 1;

			partialWrite = (UINT8*)&colorAddress[4*j];
			
			partialWrite[0] = channel[0];
			partialWrite[2] = channel[0];

			partialWrite[4] = channel[2];
			partialWrite[4+2] = channel[2];

			partialWrite[8] = channel[1];
			partialWrite[8+2] = channel[1];

			partialWrite[12] = channel[3];
			partialWrite[12+2] = channel[3];
		}
	}

	if ( localRect.bottom & 1 )
	{
		// Last visible row is even

		partialWrite = (UINT8*)(baseAddress + (localRect.bottom >> 1)*tileYstep);
	
		if (localRect.left & 1)
		{
			j = localRect.left >> 1;

			partialWrite[16*j + 1] = channel[0];
			partialWrite[16*j + 4+1] = channel[2];
			partialWrite[16*j + 8+1] = channel[1];
			partialWrite[16*j + 12+1] = channel[3];
		}
		
		for(j = xFullStart; j < xFullEnd; j++)
		{
			partialWrite[16*j] = channel[0];
			partialWrite[16*j+1] = channel[0];

			partialWrite[16*j+4] = channel[2];
			partialWrite[16*j+4+1] = channel[2];

			partialWrite[16*j+8] = channel[1];
			partialWrite[16*j+8+1] = channel[1];

			partialWrite[16*j+12] = channel[3];
			partialWrite[16*j+12+1] = channel[3];
		}

		if (localRect.right & 1)
		{
			j = (localRect.right-1) >> 1;

			partialWrite[16*j] = channel[0];
			partialWrite[16*j+4] = channel[2];
			partialWrite[16*j+8] = channel[1];
			partialWrite[16*j+12] = channel[3];
		}
	}

	return CE_OK;
}
		
CRESULT CR_NewTargetData::ClearDepth(const FLOAT32 depth)
{
	UINT32 k,j,pixel,channel;

	POINTER depthAddress;

	UINT32 *depthTarget;

	UINT32 depthVecs = channels[0].linearBytes >> 4;

	_declspec(align(16)) UINT32 depthValueVec[4];

	// Set up depth fill vector

	UINT16 *depthVec16;
	UINT16 depth16;

	UINT32 *depthVec24;
	UINT32 depth24;

	FLOAT32 *depthVecF32;
	FLOAT64 *depthVecF64;

	switch(bufferFormat)
	{
	case Ceng::IMAGE_FORMAT::D16:
	case Ceng::IMAGE_FORMAT::D16_S8:
	case Ceng::IMAGE_FORMAT::D16_S16:
		depthVec16 = (UINT16*)depthValueVec;
		depth16 = UINT16(FLOAT32(65535.0)*depth);

		depthVec16[0] = depth16;
		depthVec16[1] = depth16;
		depthVec16[2] = depth16;
		depthVec16[3] = depth16;

		depthVec16[4] = depth16;
		depthVec16[5] = depth16;
		depthVec16[6] = depth16;
		depthVec16[7] = depth16;
		break;
	case Ceng::IMAGE_FORMAT::D24_S8:

		depthVec24 = (UINT32*)depthValueVec;
		depth24 = UINT32(FLOAT32(16777215.0)*depth);

		depthVec24[0] = depth24;
		depthVec24[1] = depth24;
		depthVec24[2] = depth24;
		depthVec24[3] = depth24;

		break;
	case Ceng::IMAGE_FORMAT::D32F:
	case Ceng::IMAGE_FORMAT::D32F_S8:
	case Ceng::IMAGE_FORMAT::D32F_S16:
	case Ceng::IMAGE_FORMAT::D32F_IW:
	case Ceng::IMAGE_FORMAT::D32F_IW_S8:
	case Ceng::IMAGE_FORMAT::D32F_IW_S16:
	case Ceng::IMAGE_FORMAT::D32F_W:
	case Ceng::IMAGE_FORMAT::D32F_W_S8:
	case Ceng::IMAGE_FORMAT::D32F_W_S16:

		depthVecF32 = (FLOAT32*)depthValueVec;
		
		depthVecF32[0] = depth;
		depthVecF32[1] = depth;
		depthVecF32[2] = depth;
		depthVecF32[3] = depth;
		break;
	case Ceng::IMAGE_FORMAT::D64F:
	case Ceng::IMAGE_FORMAT::D64F_S8:
	case Ceng::IMAGE_FORMAT::D64F_S16:
	case Ceng::IMAGE_FORMAT::D64F_IW:
	case Ceng::IMAGE_FORMAT::D64F_IW_S8:
	case Ceng::IMAGE_FORMAT::D64F_IW_S16:
	case Ceng::IMAGE_FORMAT::D64F_W:
	case Ceng::IMAGE_FORMAT::D64F_W_S8:
	case Ceng::IMAGE_FORMAT::D64F_W_S16:

		depthVecF64 = (FLOAT64*)depthValueVec;
		
		depthVecF64[0] = FLOAT64(depth);
		depthVecF64[1] = FLOAT64(depth);
		break;
	}

	// NOTE: Assumes BUFFER_TILING::TILED8_LQUADS and DF32_S8

	UINT32 horizTiles = windowWidth >> 3;

	if (windowWidth & 7)
	{
		horizTiles++;
	}

	UINT32 vertTiles = windowHeight >> 3;
	
	UINT32 tileStep = tileXstep;

	if (windowHeight & 7)
	{
		vertTiles++;
	}

	__asm
	{
		movaps xmm0,depthValueVec;
	}

	for(k=0;k<vertTiles;k++)
	{
		depthAddress = baseAddress + k*tileYstep;

		__asm
		{
			mov edi,depthAddress;
		}

		for(j=0;j< horizTiles;j++)
		{
			__asm
			{
				movntps [edi+64],xmm0;
				movntps [edi+64+16],xmm0;
				movntps [edi+64+32],xmm0;
				movntps [edi+64+48],xmm0;

				movntps [edi+128],xmm0;
				movntps [edi+128+16],xmm0;
				movntps [edi+128+32],xmm0;
				movntps [edi+128+48],xmm0;

				movntps [edi+128+64],xmm0;
				movntps [edi+128+64+16],xmm0;
				movntps [edi+128+64+32],xmm0;
				movntps [edi+128+64+48],xmm0;

				add edi,tileStep;
			}
		}
	}

	return CE_OK;
}

CRESULT CR_NewTargetData::ClearDepthStencil(const Ceng::FLOAT32 depth,const Ceng::UINT32 stencil)
{
	Ceng::BOOL stencilEnabled = false;

	if (channelCount == 2)
	{
		stencilEnabled = true;
	}

	UINT32 k,j,pixel,channel;

	POINTER depthAddress;

	UINT32 *depthTarget;
	UINT32 *stencilTarget;

	UINT32 depthVecs = channels[0].linearBytes >> 4;
	UINT32 stencilVecs = channels[1].linearBytes >> 4;

	_declspec(align(16)) UINT32 depthValueVec[4];
	_declspec(align(16)) UINT32 stencilValueVec[4];

	// Set up depth fill vector

	UINT16 *depthVec16;
	UINT16 depth16;

	UINT32 *depthVec24;
	UINT32 depth24;

	FLOAT32 *depthVecF32;
	FLOAT64 *depthVecF64;

	switch(bufferFormat)
	{
	case Ceng::IMAGE_FORMAT::D16:
	case Ceng::IMAGE_FORMAT::D16_S8:
	case Ceng::IMAGE_FORMAT::D16_S16:
		depthVec16 = (UINT16*)depthValueVec;
		depth16 = UINT16(FLOAT32(65535.0)*depth);

		depthVec16[0] = depth16;
		depthVec16[1] = depth16;
		depthVec16[2] = depth16;
		depthVec16[3] = depth16;

		depthVec16[4] = depth16;
		depthVec16[5] = depth16;
		depthVec16[6] = depth16;
		depthVec16[7] = depth16;
		break;
	case Ceng::IMAGE_FORMAT::D24_S8:

		depthVec24 = (UINT32*)depthValueVec;
		depth24 = UINT32(FLOAT32(16777215.0)*depth);

		depthVec24[0] = depth24;
		depthVec24[1] = depth24;
		depthVec24[2] = depth24;
		depthVec24[3] = depth24;

		break;
	case Ceng::IMAGE_FORMAT::D32F:
	case Ceng::IMAGE_FORMAT::D32F_S8:
	case Ceng::IMAGE_FORMAT::D32F_S16:
	case Ceng::IMAGE_FORMAT::D32F_IW:
	case Ceng::IMAGE_FORMAT::D32F_IW_S8:
	case Ceng::IMAGE_FORMAT::D32F_IW_S16:
	case Ceng::IMAGE_FORMAT::D32F_W:
	case Ceng::IMAGE_FORMAT::D32F_W_S8:
	case Ceng::IMAGE_FORMAT::D32F_W_S16:

		depthVecF32 = (FLOAT32*)depthValueVec;
		
		depthVecF32[0] = depth;
		depthVecF32[1] = depth;
		depthVecF32[2] = depth;
		depthVecF32[3] = depth;
		break;
	case Ceng::IMAGE_FORMAT::D64F:
	case Ceng::IMAGE_FORMAT::D64F_S8:
	case Ceng::IMAGE_FORMAT::D64F_S16:
	case Ceng::IMAGE_FORMAT::D64F_IW:
	case Ceng::IMAGE_FORMAT::D64F_IW_S8:
	case Ceng::IMAGE_FORMAT::D64F_IW_S16:
	case Ceng::IMAGE_FORMAT::D64F_W:
	case Ceng::IMAGE_FORMAT::D64F_W_S8:
	case Ceng::IMAGE_FORMAT::D64F_W_S16:

		depthVecF64 = (FLOAT64*)depthValueVec;
		
		depthVecF64[0] = FLOAT64(depth);
		depthVecF64[1] = FLOAT64(depth);
		break;
	}

	// Set up stencil fill vector

	UINT8 *stencilVec8;
	UINT8 *stencilValue8;

	UINT16 *stencilVec16;
	UINT16 *stencilValue16;

	switch(bufferFormat)
	{
	case Ceng::IMAGE_FORMAT::D16_S8:
	case Ceng::IMAGE_FORMAT::D32F_S8:
	case Ceng::IMAGE_FORMAT::D32F_IW_S8:
	case Ceng::IMAGE_FORMAT::D32F_W_S8:
	case Ceng::IMAGE_FORMAT::D64F_S8:
	case Ceng::IMAGE_FORMAT::D64F_IW_S8:
	case Ceng::IMAGE_FORMAT::D64F_W_S8:

		stencilVec8 = (UINT8*)stencilValueVec;
		stencilValue8 = (UINT8*)&stencil;
		
		stencilVec8[0] = stencilValue8[0];
		stencilVec8[1] = stencilValue8[0];
		stencilVec8[2] = stencilValue8[0];
		stencilVec8[3] = stencilValue8[0];
		
		stencilVec8[4] = stencilValue8[0];
		stencilVec8[5] = stencilValue8[0];
		stencilVec8[6] = stencilValue8[0];
		stencilVec8[7] = stencilValue8[0];
		
		stencilVec8[8] = stencilValue8[0];
		stencilVec8[9] = stencilValue8[0];
		stencilVec8[10] = stencilValue8[0];
		stencilVec8[11] = stencilValue8[0];
		
		stencilVec8[12] = stencilValue8[0];
		stencilVec8[13] = stencilValue8[0];
		stencilVec8[14] = stencilValue8[0];
		stencilVec8[15] = stencilValue8[0];
		break;
	case Ceng::IMAGE_FORMAT::D16_S16:
	case Ceng::IMAGE_FORMAT::D32F_S16:
	case Ceng::IMAGE_FORMAT::D32F_IW_S16:
	case Ceng::IMAGE_FORMAT::D32F_W_S16:
	case Ceng::IMAGE_FORMAT::D64F_S16:
	case Ceng::IMAGE_FORMAT::D64F_IW_S16:
	case Ceng::IMAGE_FORMAT::D64F_W_S16:

		stencilVec16 = (UINT16*)stencilValueVec;
		stencilValue16 = (UINT16*)&stencil;
		
		stencilVec16[0] = stencilValue16[0];
		stencilVec16[1] = stencilValue16[0];
		stencilVec16[2] = stencilValue16[0];
		stencilVec16[3] = stencilValue16[0];
		
		stencilVec16[4] = stencilValue16[0];
		stencilVec16[5] = stencilValue16[0];
		stencilVec16[6] = stencilValue16[0];
		stencilVec16[7] = stencilValue16[0];
		break;
	}

	for(k=0;k<rows;k++)
	{
		depthAddress = baseAddress + k*tileYstep;

		for(j=0;j<columns;j++)
		{
			(*clear_bufferBlock)((void*)depthAddress,(void*)depthValueVec,depthVecs);

			if (stencilEnabled)
			{
				(*clear_bufferBlock)((void*)(depthAddress + channels[1].bufferOffset),
										(void*)stencilValueVec,stencilVecs);
			}

			depthAddress += tileXstep;			
		}
	}

	return CE_OK;
}