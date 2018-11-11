/*****************************************************************************
*
* ras-coverage-test.cpp
*
* By Jari Korkala 11/2012
*
*****************************************************************************/

#include "rasterizer-cr.h"

using namespace Ceng;

namespace Ceng
{
	//***********************************************
	// Default coverage test function

	UINT64 TILE_COVERAGE_TEST(CR_QuadEdges quadEdges[3],
								const CR_QuadEdges quadEdgeStepX[3],
								const CR_QuadEdges quadEdgeStepY[3],
								const CR_QuadEdges quadStepTileBack[3])
	{
		UINT32 quadX,quadY;
		
		UINT32 signMask;
		UINT64 tileMask;
		INT32 shiftValue;
		
		UINT32 temp; // Unsigned to force logical right shift
		
		tileMask = 0;
		shiftValue = 0;
		
		for(quadY=0;quadY<8;quadY+=2)
		{
			for(quadX=0;quadX<8;quadX+=2)
			{
				signMask = 0;
				
				temp = (UINT32)(quadEdges[0].pixel[0] & quadEdges[1].pixel[0] & quadEdges[2].pixel[0]);
				signMask |= temp >> 31;
				
				temp = (UINT32)(quadEdges[0].pixel[1] & quadEdges[1].pixel[1] & quadEdges[2].pixel[1]);
				signMask |= (temp >> 31) << 1;
				
				temp = (UINT32)(quadEdges[0].pixel[2] & quadEdges[1].pixel[2] & quadEdges[2].pixel[2]);
				signMask |= (temp >> 31) << 2;
				
				temp = (UINT32)(quadEdges[0].pixel[3] & quadEdges[1].pixel[3] & quadEdges[2].pixel[3]);
				signMask |= (temp >> 31) << 3;
				
				tileMask |= UINT64(signMask) << shiftValue;
				shiftValue += 4;
				
				if (quadX < 6)
				{
					quadEdges[0].pixel[0] += quadEdgeStepX[0].pixel[0];
					quadEdges[0].pixel[1] += quadEdgeStepX[0].pixel[1];
					quadEdges[0].pixel[2] += quadEdgeStepX[0].pixel[2];
					quadEdges[0].pixel[3] += quadEdgeStepX[0].pixel[3];
					
					quadEdges[1].pixel[0] += quadEdgeStepX[1].pixel[0];
					quadEdges[1].pixel[1] += quadEdgeStepX[1].pixel[1];
					quadEdges[1].pixel[2] += quadEdgeStepX[1].pixel[2];
					quadEdges[1].pixel[3] += quadEdgeStepX[1].pixel[3];
					
					quadEdges[2].pixel[0] += quadEdgeStepX[2].pixel[0];
					quadEdges[2].pixel[1] += quadEdgeStepX[2].pixel[1];
					quadEdges[2].pixel[2] += quadEdgeStepX[2].pixel[2];
					quadEdges[2].pixel[3] += quadEdgeStepX[2].pixel[3];
				}
			}
			
			quadEdges[0].pixel[0] += quadEdgeStepY[0].pixel[0];
			quadEdges[0].pixel[1] += quadEdgeStepY[0].pixel[1];
			quadEdges[0].pixel[2] += quadEdgeStepY[0].pixel[2];
			quadEdges[0].pixel[3] += quadEdgeStepY[0].pixel[3];
			
			quadEdges[1].pixel[0] += quadEdgeStepY[1].pixel[0];
			quadEdges[1].pixel[1] += quadEdgeStepY[1].pixel[1];
			quadEdges[1].pixel[2] += quadEdgeStepY[1].pixel[2];
			quadEdges[1].pixel[3] += quadEdgeStepY[1].pixel[3];
			
			quadEdges[2].pixel[0] += quadEdgeStepY[2].pixel[0];
			quadEdges[2].pixel[1] += quadEdgeStepY[2].pixel[1];
			quadEdges[2].pixel[2] += quadEdgeStepY[2].pixel[2];
			quadEdges[2].pixel[3] += quadEdgeStepY[2].pixel[3];
		}
		
		quadEdges[0].pixel[0] += quadStepTileBack[0].pixel[0];
		quadEdges[0].pixel[1] += quadStepTileBack[0].pixel[1];
		quadEdges[0].pixel[2] += quadStepTileBack[0].pixel[2];
		quadEdges[0].pixel[3] += quadStepTileBack[0].pixel[3];
		
		quadEdges[1].pixel[0] += quadStepTileBack[1].pixel[0];
		quadEdges[1].pixel[1] += quadStepTileBack[1].pixel[1];
		quadEdges[1].pixel[2] += quadStepTileBack[1].pixel[2];
		quadEdges[1].pixel[3] += quadStepTileBack[1].pixel[3];
		
		quadEdges[2].pixel[0] += quadStepTileBack[2].pixel[0];
		quadEdges[2].pixel[1] += quadStepTileBack[2].pixel[1];
		quadEdges[2].pixel[2] += quadStepTileBack[2].pixel[2];
		quadEdges[2].pixel[3] += quadStepTileBack[2].pixel[3];
		
		return tileMask;
	}

	//**************************************
	// X86 SSE2 version

	UINT64 TILE_COVERAGE_TEST_X86_SSE2(CR_QuadEdges quadEdges[3],
										const CR_QuadEdges quadEdgeStepX[3],
										const CR_QuadEdges quadEdgeStepY[3],
										const CR_QuadEdges quadStepTileBack[3])
	{
		UINT64 tileMask=0;

		Ceng::UINT32 rowMask[2];

		__m128i edge[3];
		
		edge[0] = _mm_load_si128((__m128i*)&quadEdges[0]);
		edge[1] = _mm_load_si128((__m128i*)&quadEdges[1]);
		edge[2] = _mm_load_si128((__m128i*)&quadEdges[2]);

		__m128i stepX[3];

		stepX[0] = _mm_load_si128((__m128i*)&quadEdgeStepX[0]);
		stepX[1] = _mm_load_si128((__m128i*)&quadEdgeStepX[1]);
		stepX[2] = _mm_load_si128((__m128i*)&quadEdgeStepX[2]);

		// First row of tiles

		// First quad
		__m128i mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		__m128 *maskF = (__m128*)&mask;
		rowMask[0] = _mm_movemask_ps(*maskF);

		// Second quad

		edge[0] = _mm_add_epi32(edge[0], stepX[0]);
		edge[1] = _mm_add_epi32(edge[1], stepX[1]);
		edge[2] = _mm_add_epi32(edge[2], stepX[2]);

		mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		rowMask[0] |= _mm_movemask_ps(*maskF) << 4;

		// Third quad

		edge[0] = _mm_add_epi32(edge[0], stepX[0]);
		edge[1] = _mm_add_epi32(edge[1], stepX[1]);
		edge[2] = _mm_add_epi32(edge[2], stepX[2]);

		mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		rowMask[0] |= _mm_movemask_ps(*maskF) << 8;

		// Fourth quad

		edge[0] = _mm_add_epi32(edge[0], stepX[0]);
		edge[1] = _mm_add_epi32(edge[1], stepX[1]);
		edge[2] = _mm_add_epi32(edge[2], stepX[2]);

		mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		rowMask[0] |= _mm_movemask_ps(*maskF) << 12;

		// Second row

		__m128i stepY[3];

		stepY[0] = _mm_load_si128((__m128i*)&quadEdgeStepY[0]);
		stepY[1] = _mm_load_si128((__m128i*)&quadEdgeStepY[1]);
		stepY[2] = _mm_load_si128((__m128i*)&quadEdgeStepY[2]);

		edge[0] = _mm_add_epi32(edge[0], stepY[0]);
		edge[1] = _mm_add_epi32(edge[1], stepY[1]);
		edge[2] = _mm_add_epi32(edge[2], stepY[2]);
		
		// First quad
		mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		rowMask[0] |= _mm_movemask_ps(*maskF) << 16;

		// Second quad

		edge[0] = _mm_add_epi32(edge[0], stepX[0]);
		edge[1] = _mm_add_epi32(edge[1], stepX[1]);
		edge[2] = _mm_add_epi32(edge[2], stepX[2]);

		mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		rowMask[0] |= _mm_movemask_ps(*maskF) << 20;

		// Third quad

		edge[0] = _mm_add_epi32(edge[0], stepX[0]);
		edge[1] = _mm_add_epi32(edge[1], stepX[1]);
		edge[2] = _mm_add_epi32(edge[2], stepX[2]);

		mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		rowMask[0] |= _mm_movemask_ps(*maskF) << 24;

		// Fourth quad

		edge[0] = _mm_add_epi32(edge[0], stepX[0]);
		edge[1] = _mm_add_epi32(edge[1], stepX[1]);
		edge[2] = _mm_add_epi32(edge[2], stepX[2]);

		mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		rowMask[0] |= Ceng::UINT32(_mm_movemask_ps(*maskF)) << 28;

		// Third row

		edge[0] = _mm_add_epi32(edge[0], stepY[0]);
		edge[1] = _mm_add_epi32(edge[1], stepY[1]);
		edge[2] = _mm_add_epi32(edge[2], stepY[2]);

		// First quad
		mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		rowMask[1] = _mm_movemask_ps(*maskF);

		// Second quad

		edge[0] = _mm_add_epi32(edge[0], stepX[0]);
		edge[1] = _mm_add_epi32(edge[1], stepX[1]);
		edge[2] = _mm_add_epi32(edge[2], stepX[2]);

		mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		rowMask[1] |= _mm_movemask_ps(*maskF) << 4;

		// Third quad

		edge[0] = _mm_add_epi32(edge[0], stepX[0]);
		edge[1] = _mm_add_epi32(edge[1], stepX[1]);
		edge[2] = _mm_add_epi32(edge[2], stepX[2]);

		mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		rowMask[1] |= _mm_movemask_ps(*maskF) << 8;

		// Fourth quad

		edge[0] = _mm_add_epi32(edge[0], stepX[0]);
		edge[1] = _mm_add_epi32(edge[1], stepX[1]);
		edge[2] = _mm_add_epi32(edge[2], stepX[2]);

		mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		rowMask[1] |= _mm_movemask_ps(*maskF) << 12;


		// Fourth row

		edge[0] = _mm_add_epi32(edge[0], stepY[0]);
		edge[1] = _mm_add_epi32(edge[1], stepY[1]);
		edge[2] = _mm_add_epi32(edge[2], stepY[2]);

		// First quad
		mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		rowMask[1] |= _mm_movemask_ps(*maskF) << 16;

		// Second quad

		edge[0] = _mm_add_epi32(edge[0], stepX[0]);
		edge[1] = _mm_add_epi32(edge[1], stepX[1]);
		edge[2] = _mm_add_epi32(edge[2], stepX[2]);

		mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		rowMask[1] |= _mm_movemask_ps(*maskF) << 20;

		// Third quad

		edge[0] = _mm_add_epi32(edge[0], stepX[0]);
		edge[1] = _mm_add_epi32(edge[1], stepX[1]);
		edge[2] = _mm_add_epi32(edge[2], stepX[2]);

		mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		rowMask[1] |= _mm_movemask_ps(*maskF) << 24;

		// Fourth quad

		edge[0] = _mm_add_epi32(edge[0], stepX[0]);
		edge[1] = _mm_add_epi32(edge[1], stepX[1]);
		edge[2] = _mm_add_epi32(edge[2], stepX[2]);

		mask = _mm_and_si128(edge[0], edge[1]);
		mask = _mm_and_si128(mask, edge[2]);

		rowMask[1] |= Ceng::UINT32(_mm_movemask_ps(*maskF)) << 28;

		tileMask = (UINT64(rowMask[1]) << 32) + UINT64(rowMask[0]);
		/*
		__asm
		{
			mov eax,quadEdges;
			mov esi,quadEdgeStepY;

			movdqa xmm0,[eax]; // xmm0 = edgeFunction[0]
			movdqa xmm1,[eax+16]; // xmm1 = edgeFunction[1]
			movdqa xmm2,[eax+32]; // xmm2 = edgeFunction[2]

			mov eax,quadEdgeStepX;

			movdqa xmm3,[eax]; // xmm3 = quadEdgeStepX[0]
			movdqa xmm4,[eax+16]; // xmm4 = quadEdgeStepX[1]
			movdqa xmm5,[eax+32]; // xmm5 = quadEdgeStepX[2]

			//********************************
			// First row of tiles

			// First quad
			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2; // xmm6 = xmm0 & xmm1 & xmm2

			movmskps ecx,xmm6;
			mov eax,ecx;

			// second quad
			paddd xmm0,xmm3; // xmm0 += quadEdgeStepX[0]
			paddd xmm1,xmm4; // xmm1 += quadEdgeStepX[1]
			paddd xmm2,xmm5; // xmm2 += quadEdgeStepX[2]

			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2;

			movmskps ecx,xmm6;
			shl ecx,4;
			or eax,ecx;

			// third quad
			paddd xmm0,xmm3;
			paddd xmm1,xmm4;
			paddd xmm2,xmm5;

			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2;

			movmskps ecx,xmm6;
			shl ecx,8;
			or eax,ecx;

			// fourth quad
			paddd xmm0,xmm3;
			paddd xmm1,xmm4;
			paddd xmm2,xmm5;

			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2;

			movmskps ecx,xmm6;
			shl ecx,12;
			or eax,ecx;

			//**************************
			// Second row

			paddd xmm0,[esi]; // += quadEdgeStepY[0]
			paddd xmm1,[esi+16]; // += quadEdgeStepY[1]
			paddd xmm2,[esi+32]; // += quadEdgeStepY[2]

			// First quad
			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2;

			movmskps ecx,xmm6;
			shl ecx,16;
			or eax,ecx;

			// second quad
			paddd xmm0,xmm3;
			paddd xmm1,xmm4;
			paddd xmm2,xmm5;

			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2;

			movmskps ecx,xmm6;
			shl ecx,20;
			or eax,ecx;

			// third quad
			paddd xmm0,xmm3;
			paddd xmm1,xmm4;
			paddd xmm2,xmm5;

			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2;

			movmskps ecx,xmm6;
			shl ecx,24;
			or eax,ecx;

			// fourth quad
			paddd xmm0,xmm3;
			paddd xmm1,xmm4;
			paddd xmm2,xmm5;

			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2;

			movmskps ecx,xmm6;
			shl ecx,28;
			or eax,ecx;

			//**************************
			// Third row

			paddd xmm0,[esi]; // += quadStepY
			paddd xmm1,[esi+16];
			paddd xmm2,[esi+32];

			// First quad
			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2;

			movmskps ecx,xmm6;
			mov edx,ecx;

			// second quad
			paddd xmm0,xmm3;
			paddd xmm1,xmm4;
			paddd xmm2,xmm5;

			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2;

			movmskps ecx,xmm6;
			shl ecx,4;
			or edx,ecx;

			// third quad
			paddd xmm0,xmm3;
			paddd xmm1,xmm4;
			paddd xmm2,xmm5;

			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2;

			movmskps ecx,xmm6;
			shl ecx,8;
			or edx,ecx;

			// fourth quad
			paddd xmm0,xmm3;
			paddd xmm1,xmm4;
			paddd xmm2,xmm5;

			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2;

			movmskps ecx,xmm6;
			shl ecx,12;
			or edx,ecx;

			//**************************
			// Fourth row

			paddd xmm0,[esi]; // += quadStepY
			paddd xmm1,[esi+16];
			paddd xmm2,[esi+32];

			// First quad
			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2;

			movmskps ecx,xmm6;
			shl ecx,16;
			or edx,ecx;

			// second quad
			paddd xmm0,xmm3;
			paddd xmm1,xmm4;
			paddd xmm2,xmm5;

			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2;

			movmskps ecx,xmm6;
			shl ecx,20;
			or edx,ecx;

			// third quad
			paddd xmm0,xmm3;
			paddd xmm1,xmm4;
			paddd xmm2,xmm5;

			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2;

			movmskps ecx,xmm6;
			shl ecx,24;
			or edx,ecx;

			// fourth quad
			paddd xmm0,xmm3;
			paddd xmm1,xmm4;
			paddd xmm2,xmm5;

			movdqa xmm6,xmm0;
			pand xmm6,xmm1;
			pand xmm6,xmm2;

			movmskps ecx,xmm6;
			shl ecx,28;
			or edx,ecx;

			// Write result
			lea edi,tileMask;
			mov [edi],eax;
			mov [edi+4],edx;
		}
		*/

		return tileMask;
	}

} // namespace Ceng