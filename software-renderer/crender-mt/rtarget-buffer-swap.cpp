/*****************************************************************************
*
* rtarget-buffer-swap.cpp
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#include <xmmintrin.h>
#include <emmintrin.h>

#include "rtarget-data.h"

using namespace Ceng;

CRESULT CR_NewTargetData::ToFrameBuffer(const Ceng::Rectangle &sourceArea,
										const Ceng::BufferData &destBuffer)
{

	switch(tilingMode)
	{
	case Ceng::BUFFER_TILING::SCANLINES:

		//return BufferSwap_Scanlines(startX,startY,targetWidth,targetHeight,destFormat,destPitch,destBuffer);

		break;
	case Ceng::BUFFER_TILING::LQUADS:

		if (dataLayout == Ceng::BUFFER_LAYOUT::AOS)
		{
			//return BufferSwap_Lquads_AOS(startX,startY,targetWidth,targetHeight,destFormat,destPitch,destBuffer);
		}
		else
		{
			return BufferSwap_Lquads_SOA(sourceArea,destBuffer);
			/*
			return BufferSwap_Lquads_SOA(startX,startY,targetWidth,targetHeight,
											destFormat,
											destBufferWidth,destBufferHeight,
											destPitch,destBuffer);
											*/
		}

		break;
	case Ceng::BUFFER_TILING::TILED8_LQUADS:

		//return BufferSwap_Tiled8(startX,startY,targetWidth,targetHeight,destFormat,destPitch,destBuffer);

		break;
	case Ceng::BUFFER_TILING::TILED16_LQUADS:
		break;
	};

	return CE_OK;
}

CRESULT CR_NewTargetData::BufferSwap_Scanlines(const Ceng::UINT32 startX,const Ceng::UINT32 startY,
											const Ceng::UINT32 targetWidth,const Ceng::UINT32 targetHeight,
											const Ceng::IMAGE_FORMAT::value destFormat,
											const POINTER destPitch,void *destBuffer)
{

	UINT32 localWidth = targetWidth;

	if (localWidth > windowWidth)
	{
		localWidth = windowWidth;
	}

	UINT32 localHeight = targetHeight;

	if (localHeight > windowHeight)
	{
		localHeight = windowHeight;
	}
	
	UINT32 colorDepth = channels[0].bytesPerPixel;
	UINT32 sourcePitch = channels[0].unitYstep;

	UINT32 sourceOffset = 0;
	UINT32 destOffset = 0;

	UINT8 *destPtr = (UINT8*)destBuffer;
	UINT8 *sourcePtr = (UINT8*)(baseAddress + channels[0].bufferOffset);

	UINT32 x,y;

	for(y=0; y < localHeight; y++)
	{
		destOffset = y*destPitch;
		sourceOffset = sourcePitch*(startY+y) + colorDepth*startX;

		for(x=0; x < localWidth; x++)
		{
			destPtr[destOffset] = sourcePtr[sourceOffset];
			destPtr[destOffset+1] = sourcePtr[sourceOffset+1];
			destPtr[destOffset+2] = sourcePtr[sourceOffset+2];
			destPtr[destOffset+3] = sourcePtr[sourceOffset+3];

			destOffset += colorDepth;
			sourceOffset += colorDepth;
		}
	}

	return CE_OK;
}

CRESULT CR_NewTargetData::BufferSwap_Lquads_AOS(const Ceng::UINT32 startX,const Ceng::UINT32 startY,
											const Ceng::UINT32 width,const Ceng::UINT32 height,
											const Ceng::IMAGE_FORMAT::value destFormat,
											const POINTER destPitch,void *destBuffer)
{
	// Only copy the visible part of the frame buffer
	// in windowed mode

	UINT32 localWidth = width;

	if (localWidth > bufferWidth)
	{
		localWidth = bufferWidth;
	}

	/*
	if (localWidth > windowWidth)
	{
		localWidth = windowWidth;
	}
	*/

	// Pad to nearest full quad if possible
	if (localWidth & 1)
	{
		int padding = localWidth & 1;

		if ((localWidth + padding) <= width)
		{
			localWidth += padding;
		}
	}

	UINT32 localHeight = height;

	if (localHeight > bufferHeight)
	{
		localHeight = bufferHeight;
	}

	/*
	if (localHeight > windowHeight)
	{
		localHeight = windowHeight;
	}
	*/

	// Pad to nearest multiple of 8 if possible
	if (localHeight & 1)
	{
		int padding = localHeight & 1;

		if ((localHeight + padding) <= height)
		{
			localHeight += padding;
		}
	}

	UINT32 horizQuads = localWidth >> 1;
	UINT32 vertQuads = localHeight >> 1;

	UINT32 quadX,quadY;

	UINT32 colorDepth = channels[0].bytesPerPixel;
	UINT32 sourcePitch = channels[0].unitYstep;

	UINT32 sourceOffset = 0;
	UINT32 destOffset = 0;

	UINT8 *destPtr = (UINT8*)destBuffer;
	UINT8 *sourcePtr = (UINT8*)(baseAddress + channels[0].bufferOffset);

	for(quadY=0; quadY < vertQuads; quadY++)
	{
		destOffset = 2*quadY*destPitch;
		
		sourceOffset = sourcePitch*((startY+2*quadY) >> 1) + 4*colorDepth*startX;

		for(quadX=0; quadX < horizQuads; quadX++)
		{
			UINT32 *sourceTemp = (UINT32*)&sourcePtr[sourceOffset];
			UINT32 *destTemp = (UINT32*)&destPtr[destOffset];

			destTemp[0] = sourceTemp[0];
			destTemp[1] = sourceTemp[1];

			destTemp = (UINT32*)&destPtr[destOffset + destPitch];

			destTemp[0] = sourceTemp[2];
			destTemp[1] = sourceTemp[3];

			destOffset += 2*colorDepth;
			sourceOffset += 4*colorDepth;
		}

		if (localWidth & 1)
		{
		}

		destOffset -= localWidth*colorDepth;
		destOffset += 2*destPitch;
	}

	if (localHeight & 1)
	{

	}

	return CE_OK;
}

CRESULT CR_NewTargetData::BufferSwap_Lquads_SOA(const Ceng::Rectangle &sourceArea,
												const Ceng::BufferData &destBuffer)
{
	// Clip source area

	Ceng::Rectangle area;

	area.left = sourceArea.left;

	if (area.left < 0)
	{
		area.left = 0;
	}

	area.top = sourceArea.top;

	if (area.top < 0)
	{
		area.top = 0;
	}

	area.right = sourceArea.right;

	if (area.right > Ceng::INT32(bufferWidth))
	{
		area.right = bufferWidth;
	}

	area.bottom = sourceArea.bottom;

	if (area.bottom > Ceng::INT32(bufferHeight))
	{
		area.bottom = bufferHeight;
	}

	// Abort if a null rect is specified

	if (area.left == area.right)
	{
		return CE_OK;
	}

	if (area.top == area.bottom)
	{
		return CE_OK;
	}

	UINT32 localWidth = area.right - area.left;
	UINT32 localHeight = area.bottom - area.top;

	
	// Pad to nearest full quad if possible
	if (localWidth & 1)
	{
		int padding = localWidth & 1;

		if ((localWidth + padding) <= bufferWidth)
		{
			localWidth += padding;
		}
	}
	
	// Pad to nearest multiply of 8 if possible
	if (localHeight & 1)
	{
		int padding = localHeight & 1;

		if ((localHeight + padding) <= bufferHeight)
		{
			localHeight += padding;
		}
	}

	UINT32 horizQuads = localWidth >> 1;
	UINT32 vertQuads = localHeight >> 1;

	UINT32 quadX,quadY;

	UINT32 colorDepth = channels[0].bytesPerPixel;
	UINT32 sourcePitch = channels[0].unitYstep;

	UINT32 destPitch = destBuffer.pitch;

	UINT32 sourceOffset = 0;
	UINT32 destOffset = 0;

	UINT8 *destPtr = (UINT8*)destBuffer.data;
	UINT8 *sourcePtr = (UINT8*)(baseAddress + channels[0].bufferOffset);

	for(quadY=0; quadY < vertQuads; quadY++)
	{
		destOffset = 2*quadY*destBuffer.pitch;
		
		sourceOffset = sourcePitch*( (area.top+2*quadY) >> 1) + 4*colorDepth*area.left;

		for(quadX=0; quadX < horizQuads; quadX++)
		{
			// Colors come in vertical layout:

			// {alpha[3],alpha[2],alpha[1],alpha[0]} , 
			// { green[3] , ... , green[0] } ,
			// { red[3] , ... , red[0] }
			// { blue[3] , ... , blue[0] }

			// source = { {a3,a2,a1,a0}, {r3,r2,r1,r0}, { g3, g2, g1, g0 }, { b3, b2, b1, b0 } 
			__m128i source = _mm_load_si128((__m128i*)&sourcePtr[sourceOffset]);

			// source = { {a3,a2,a1,a0}, {g3,g2,g1,g0}, { r3, r2, r1, r0 }, { b3, b2, b1, b0 } 
			source = _mm_shuffle_epi32(source, _MM_SHUFFLE(3, 1, 2, 0));

			// sourceMix = { {x,x,x,x}, {x,x,x,x}, {a3,a2,a1,a0}, {g3,g2,g1,g0} }
			__m128i sourceMix = _mm_shuffle_epi32(source, (1 << 3) | (1 << 2) | (1 << 1));

			// ar_gb_low = { {a3,r3,a2,r2} , {a1,r1,a0,r0} , {g3,b3,g2,b2} , {g1,b1,g0,b0} }
			__m128i ar_gb_low = _mm_unpacklo_epi8(source, sourceMix);
		
			// ar_gb_high = { {x,x,x,x} , {x,x,x,x} , {a3,r3,a2,r2} , {a1,r1,a0,r0} }
			//__m128i ar_gb_high = _mm_shuffle_epi32(ar_gb_low, (1 << 3) | (1 << 2) | (1 << 1));
			__m128i ar_gb_high = _mm_srli_si128(ar_gb_low, 8);

			// output = { {a3,r3,g3,b3} , {a2,r2,g2,b2} , {a1,r1,g1,b1} , {a0,r0,g0,b0} }
			__m128i output = _mm_unpacklo_epi16(ar_gb_low, ar_gb_high);		

			__m128 *outputF = (__m128*)&output;

			_mm_storel_pi((__m64*)&destPtr[destOffset], *outputF);
			_mm_storeh_pi((__m64*)&destPtr[destOffset + destPitch], *outputF);

			/*
			destPtr[destOffset] = sourcePtr[sourceOffset]; // blue
			destPtr[destOffset+1] = sourcePtr[sourceOffset+8]; // green
			destPtr[destOffset+2] = sourcePtr[sourceOffset+4]; // red
			destPtr[destOffset+3] = sourcePtr[sourceOffset+12]; // alpha

			// Top-right pixel
			destPtr[destOffset+4] = sourcePtr[sourceOffset+1];
			destPtr[destOffset+5] = sourcePtr[sourceOffset+1+8];
			destPtr[destOffset+6] = sourcePtr[sourceOffset+1+4];
			destPtr[destOffset+7] = sourcePtr[sourceOffset+1+12];

			// Bottom-left pixel
			destPtr[destOffset+destBuffer.pitch] = sourcePtr[sourceOffset+2];
			destPtr[destOffset+destBuffer.pitch+1] = sourcePtr[sourceOffset+2+8];
			destPtr[destOffset+destBuffer.pitch+2] = sourcePtr[sourceOffset+2+4];
			destPtr[destOffset+destBuffer.pitch+3] = sourcePtr[sourceOffset+2+12];

			// Bottom-right pixel
			destPtr[destOffset+destBuffer.pitch+4] = sourcePtr[sourceOffset+3];
			destPtr[destOffset+destBuffer.pitch+5] = sourcePtr[sourceOffset+3+8];
			destPtr[destOffset+destBuffer.pitch+6] = sourcePtr[sourceOffset+3+4];
			destPtr[destOffset+destBuffer.pitch+7] = sourcePtr[sourceOffset+3+12];
			*/
			
		
			destOffset += 2*colorDepth;
			sourceOffset += 4*colorDepth;
		}

		if (localWidth & 1)
		{
		}

		destOffset -= localWidth*colorDepth;
		destOffset += 2*destBuffer.pitch;
	}

	if (localHeight & 1)
	{

	}

	return CE_OK;
}

CRESULT CR_NewTargetData::BufferSwap_Tiled8(const Ceng::UINT32 startX,const Ceng::UINT32 startY,
											const Ceng::UINT32 targetWidth,const Ceng::UINT32 targetHeight,
											const Ceng::IMAGE_FORMAT::value destFormat,
											const POINTER destPitch,void *destBuffer)
{
	// Only copy the visible part of the frame buffer
	// in windowed mode

	UINT32 localWidth = targetWidth;

	if (localWidth > windowWidth)
	{
		localWidth = windowWidth;
	}

	
	// Pad to nearest multiply of 8 if possible
	if (localWidth & 7)
	{
		int padding = 8 - (localWidth & 7);

		if ((localWidth + padding) <= targetWidth)
		{
			localWidth += padding;
		}
	}
	

	UINT32 localHeight = targetHeight;

	if (localHeight > windowHeight)
	{
		localHeight = windowHeight;
	}

	
	// Pad to nearest multiply of 8 if possible
	if (localHeight & 7)
	{
		int padding = 8 - (localHeight & 7);

		if ((localHeight + padding) <= targetHeight)
		{
			localHeight += padding;
		}
	}
	

	UINT32 horizTiles = localWidth >> 3;
	UINT32 vertTiles = localHeight >> 3;

	UINT32 tileX,tileY;
	UINT32 quadX,quadY;

	UINT32 colorDepth = channels[0].bytesPerPixel;
	UINT32 sourcePitch = channels[0].tileYstep;

	UINT32 sourceOffset = 0;
	UINT32 destOffset = 0;

	UINT8 *destPtr = (UINT8*)destBuffer;
	UINT8 *sourcePtr = (UINT8*)(baseAddress + channels[0].bufferOffset);

	for(tileY=0;tileY<vertTiles;tileY++)
	{
		destOffset = 8*tileY*destPitch;

		sourceOffset = sourcePitch*((startY+8*tileY) >> 3) + 16*4*colorDepth*startX;

		for(tileX=0;tileX<horizTiles;tileX++)
		{	
			for(quadY=0;quadY<4;quadY++)
			{
				for(quadX=0;quadX<4;quadX++)
				{
					
					// Top-left pixel
					destPtr[destOffset] = sourcePtr[sourceOffset];
					destPtr[destOffset+1] = sourcePtr[sourceOffset+1];
					destPtr[destOffset+2] = sourcePtr[sourceOffset+2];
					destPtr[destOffset+3] = sourcePtr[sourceOffset+3];

					// Top-right pixel
					destPtr[destOffset+4] = sourcePtr[sourceOffset+4];
					destPtr[destOffset+5] = sourcePtr[sourceOffset+5];
					destPtr[destOffset+6] = sourcePtr[sourceOffset+6];
					destPtr[destOffset+7] = sourcePtr[sourceOffset+7];

					// Bottom-left pixel
					destPtr[destOffset+destPitch] = sourcePtr[sourceOffset+8];
					destPtr[destOffset+destPitch+1] = sourcePtr[sourceOffset+9];
					destPtr[destOffset+destPitch+2] = sourcePtr[sourceOffset+10];
					destPtr[destOffset+destPitch+3] = sourcePtr[sourceOffset+11];

					// Bottom-right pixel
					destPtr[destOffset+destPitch+4] = sourcePtr[sourceOffset+12];
					destPtr[destOffset+destPitch+5] = sourcePtr[sourceOffset+13];
					destPtr[destOffset+destPitch+6] = sourcePtr[sourceOffset+14];
					destPtr[destOffset+destPitch+7] = sourcePtr[sourceOffset+15];

					destOffset += 2*colorDepth;
					sourceOffset += 4*colorDepth;
				}

				destOffset -= 8*colorDepth;
				destOffset += 2*destPitch;

			}

			destOffset -= 8*destPitch;
			destOffset += 8*colorDepth;
		}

		if (localWidth & 7)
		{
			UINT32 destStep = (localWidth >> 1)*colorDepth;
			UINT32 sourceStep = 4*destStep;

			for(quadY=0;quadY<8;quadY+=2)
			{
				for(quadX=0;quadX<(localWidth & 7);quadX+=2)
				{
					memcpy(&destPtr[destOffset],&sourcePtr[sourceOffset],2*colorDepth);
					memcpy(&destPtr[destOffset+destPitch],&sourcePtr[sourceOffset+2*colorDepth],2*colorDepth);

					destOffset += 2*colorDepth;
					sourceOffset += 4*colorDepth;
				}

				if (localWidth & 1)
				{
					memcpy(&destPtr[destOffset],&sourcePtr[sourceOffset],colorDepth);
					memcpy(&destPtr[destOffset+destPitch],&sourcePtr[sourceOffset+2*colorDepth],colorDepth);
				}
				
				destOffset -= destStep;
				destOffset += 2*destPitch;

				sourceOffset += sourceStep;
			}
		}

	}
	

	// If resY is not divisible by 8, last row is a special case
	if (localHeight & 7)
	{
		destOffset = 8*tileY*destPitch;

		sourceOffset = sourcePitch*((startY+8*tileY) >> 3) + 16*4*colorDepth*startX;

		for(tileX=0;tileX<horizTiles;tileX++)
		{	
			for(quadY=0;quadY<(localHeight & 7);quadY+=2)
			{
				for(quadX=0;quadX<4;quadX++)
				{
					memcpy(&destPtr[destOffset],&sourcePtr[sourceOffset],2*colorDepth);
					memcpy(&destPtr[destOffset+destPitch],&sourcePtr[sourceOffset+2*colorDepth],2*colorDepth);

					destOffset += 2*colorDepth;
					sourceOffset += 4*colorDepth;
				}

				destOffset -= 8*colorDepth;
				destOffset += 2*destPitch;
			}

			if (localHeight & 1)
			{
				for(quadX=0;quadX<4;quadX++)
				{
					memcpy(&destPtr[destOffset],&sourcePtr[sourceOffset],2*colorDepth);

					destOffset += 2*colorDepth;
					sourceOffset += 4*colorDepth;
				}
			}

			destOffset -= 8*destPitch;
			destOffset += 8*colorDepth;
		}

		if (localWidth & 7)
		{
			for(quadY=0;quadY<(localHeight & 7);quadY+=2)
			{
				for(quadX=0;quadX<(localWidth & 7);quadX+=2)
				{
					memcpy(&destPtr[destOffset],&sourcePtr[sourceOffset],2*colorDepth);
						
					destOffset += 2*colorDepth;
					sourceOffset += 4*colorDepth;
				}

				if (localWidth & 1)
				{
					memcpy(&destPtr[destOffset],&sourcePtr[sourceOffset],colorDepth);
				}

				destOffset += 2*colorDepth;
				sourceOffset += 4*colorDepth;

				destOffset -= 8*colorDepth;
				destOffset += 2*destPitch;
			}
		}
	}	

	
		
		//******************************
		// New 8,2 tile copy

		// TODO: This should work in windowed mode when bufferSize[i] < frameSize[i], but doesn't
	
		/*
		for(j=0;j<height;j+=8)
		{
			destOffset = texturePitch*j;
			
			sourceOffset = sourcePitch*((ymin+j)>>3) +
				16*4*colorDepth*((xmin)>>3);
				
			//sourceOffset += 4*colorDepth*(((xmin)&7) >> 1);
			//sourceOffset += 16*colorDepth*(((ymin+j)&7) >> 1);
			
			//sourceOffset += colorDepth*((xmin) & 1);
			//sourceOffset += 2*colorDepth*((ymin+j) & 1);
			
			for(i=0;i<width;i+=16)
			{
				destOffset = texturePitch*j;
				destOffset += colorDepth*i;

				__asm
				{
					mov edi,localTexturePtr;	
					add edi,destOffset;

					//mov edx,texturePitch;
				}

				for(n=0;n<8;n+=2)
				{
					__asm
					{	
						mov esi,bufferPtr;
						add esi,sourceOffset;

						prefetchnta [esi+512];
						prefetchnta [esi+768];

						movaps xmm0,[esi];
						movaps xmm1,[esi+16];
						
						movaps xmm5,xmm1;
						
						movhlps xmm5,xmm0;
						movlhps xmm0,xmm1;					
						
						movaps xmm1,[esi+32];
						movaps xmm2,[esi+48];
						
						movaps xmm6,xmm2;
						
						movhlps xmm6,xmm1;
						movlhps xmm1,xmm2;					
						
						movaps xmm2,[esi+256];
						movaps xmm3,[esi+256+16];
						
						movaps xmm7,xmm3;
						
						movhlps xmm7,xmm2;
						movlhps xmm2,xmm3;
						
						movaps xmm3,[esi+256+32];
						movaps xmm4,[esi+256+48];
						
						movlhps xmm3,xmm4;
						
						movntps [edi],xmm0;
						movntps [edi+16],xmm1;
						movntps [edi+32],xmm2;
						movntps [edi+48],xmm3;
						
						//add edi,texturePitch;
						//add edi,edx;
						
						movaps xmm3,[esi+256+32];
						movhlps xmm4,xmm3;
						
						movntps [edi],xmm5;
						movntps [edi+16],xmm6;
						movntps [edi+32],xmm7;
						movntps [edi+48],xmm4;

						//add edi,texturePitch;
						//add edi,edx;

					} // asm

					// Step to next row of quads within tile
					sourceOffset += 16*colorDepth;

				} // for n	

				sourceOffset += 64*colorDepth;

			} // for i

			if (width & 15)
			{
			}

		} // for j

		if (height & 7)
		{
		}
		
		
		__asm
		{
			// Use store fence to ensure that all non-temporal
			// writes are visible to graphics card

			sfence;
		}
		*/

	return CE_OK;
}

