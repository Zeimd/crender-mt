/*****************************************************************************
*
* rtarget-depth-test.cpp
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#include <xmmintrin.h>
#include <emmintrin.h>

#include "rtarget-data.h"

using namespace Ceng;

const _declspec(align(16)) Ceng::UINT8 toSignedByte[16] = {128,128,128,128,
												128,128,128,128,
												128,128,128,128,
												128,128,128,128};

const _declspec(align(16)) Ceng::INT32 coverageTable4[16][4] =
	{
		{0,0,0,0} ,
		{-1,0,0,0} ,
		{0,-1,0,0} ,
		{-1,-1,0,0} ,
		{0,0,-1,0} ,
		{-1,0,-1,0} ,
		{0,-1,-1,0} ,
		{-1,-1,-1,0} ,
		{0,0,0,-1} ,
		{-1,0,0,-1} ,
		{0,-1,0,-1} ,
		{-1,-1,0,-1} ,
		{0,0,-1,-1} ,
		{-1,0,-1,-1},
		{0,-1,-1,-1},
		{-1,-1,-1,-1}
	};

const _declspec(align(16)) Ceng::INT8 coverageTable16[16][4] = 
	{ 
		{0,0,0,0} ,
		{-1,0,0,0} ,
		{0,-1,0,0} ,
		{-1,-1,0,0} ,
		{0,0,-1,0} ,
		{-1,0,-1,0} ,
		{0,-1,-1,0} ,
		{-1,-1,-1,0} ,
		{0,0,0,-1} ,
		{-1,0,0,-1} ,
		{0,-1,0,-1} ,
		{-1,-1,0,-1} ,
		{0,0,-1,-1} ,
		{-1,0,-1,-1},
		{0,-1,-1,-1},
		{-1,-1,-1,-1}
	};

enum X86_VERSION
{
	SSE2 = 0 ,
	SSE = 1 ,
	SSE3 = 2 ,
	SUPL_SSE3 = 3 ,
	SSE_41 = 4 ,
	SSE_42 = 5 ,
	AVX = 6 ,
	MMX = 7 ,
	CMOV = 8 ,
};





/**
 * Returns coverage mask as byte[16]
 *
 * xmm0,xmm1,xmm2,xmm5 reserved
 *
 * xmm3 = output
 */
void GetCoverageMask16_x86_SSE2(Ceng::UINT32 coverageMask,Ceng::IMAGE_FORMAT::value format)
{
	switch(format)
	{
	case Ceng::IMAGE_FORMAT::D24_S8:



		/*
		__asm
		{
			mov eax,coverageMask;
			lea esi,coverageTable4;

			and eax,15;

			movaps xmm3,[esi+4*eax];
		}
		*/

		break;
	case Ceng::IMAGE_FORMAT::D32F_S8:
	case Ceng::IMAGE_FORMAT::D32F_IW_S8:
	case Ceng::IMAGE_FORMAT::D32F_W_S8:

		__asm
		{
			mov eax,coverageMask;
			lea esi,coverageTable16;

			mov ecx,eax;
			and ecx,15;
			movd xmm3,[esi+4*ecx];

			shr eax,4;
			mov ecx,eax;
			and ecx,15;
			movd xmm4,[esi+4*ecx];
			pslldq xmm4,4;
			por xmm3,xmm4;

			shr eax,4;
			mov ecx,eax;
			and ecx,15;
			movd xmm4,[esi+4*ecx];
			pslldq xmm4,8;
			por xmm3,xmm4;

			shr eax,4;
			and eax,15;
			movd xmm4,[esi+4*eax];
			pslldq xmm4,12;
			por xmm3,xmm4;
		}

		break;
	}
}

namespace Ceng
{
	enum DEPTH_FORMAT
	{
		DF_FLOAT32 = 0,
		DF_UINT24 = 1 ,
		DF_UINT16 = 2 ,
		DF_FLOAT64 = 3 ,
	};
}

template<DEPTH_FORMAT format,Ceng::TEST_TYPE::value depthTest,X86_VERSION codeVersion>
inline void DepthTest()
{
	// reserved: xmm0,xmm1,xmm2,xmm3,xmm4

	//*********************
	// in:

	// const xmm0 = depth variables

	// xmm4 = stencil pass mask

	// const xmm5 = depth buffer data

	//**********************
	// out: 

	// xmm4 = pure depth pass mask
	// xmm6 = coverage & stencil pass & depth pass 
}

template<>
inline void DepthTest<DEPTH_FORMAT::DF_FLOAT32,Ceng::TEST_TYPE::LESS,X86_VERSION::SSE2>()
{
	__asm
	{
		movaps xmm6,xmm1;
		cmpltps xmm6,xmm5; // xmm6 < xmm5
	}
}

template<>
inline void DepthTest<DEPTH_FORMAT::DF_FLOAT32,Ceng::TEST_TYPE::LESS_EQ,X86_VERSION::SSE2>()
{
	__asm
	{
		movaps xmm6,xmm1;
		cmpleps xmm6,xmm5; // xmm6 <= xmm5
	}
}

template<>
inline void DepthTest<DEPTH_FORMAT::DF_FLOAT32,Ceng::TEST_TYPE::EQUAL,X86_VERSION::SSE2>()
{
	__asm
	{
		movaps xmm6,xmm1;
		cmpeqps xmm6,xmm5; 
	}
}

template<>
inline void DepthTest<DEPTH_FORMAT::DF_FLOAT32,Ceng::TEST_TYPE::ABOVE_EQ,X86_VERSION::SSE2>()
{
	__asm
	{
		movaps xmm6,xmm5;
		cmpltps xmm6,xmm1; // xmm0 >= xmm6
	}
}

template<>
inline void DepthTest<DEPTH_FORMAT::DF_FLOAT32,Ceng::TEST_TYPE::ABOVE,X86_VERSION::SSE2>()
{
	__asm
	{
		movaps xmm6,xmm5;
		cmpleps xmm6,xmm1; // xmm0 > xmm6
	}
}

template<>
inline void DepthTest<DEPTH_FORMAT::DF_FLOAT32,Ceng::TEST_TYPE::ALWAYS_PASS,X86_VERSION::SSE2>()
{
	__asm
	{
		cmpeqps xmm6,xmm6;
	}
}

template<>
inline void DepthTest<DEPTH_FORMAT::DF_FLOAT32,Ceng::TEST_TYPE::NEVER_PASS,X86_VERSION::SSE2>()
{
	__asm
	{
		xorps xmm6,xmm6;
	}
}

//***********************************************************************************
// PrepareDepth

template<bool depthEnable,DEPTH_FORMAT depthFormat,X86_VERSION codeVersion>
inline void PrepareDepth(void *depthValues)
{
}

template<>
inline void PrepareDepth<true,DEPTH_FORMAT::DF_FLOAT32,X86_VERSION::SSE2>(void *depthValues)
{
	__asm
	{
		mov eax,depthValues;
		movaps xmm1,[eax];
	}
}

//***********************************************************************************
// DepthStepY

template<bool depthEnable,DEPTH_FORMAT depthFormat,X86_VERSION codeVersion>
inline void DepthStepY(void *depthStepY)
{
}

template<>
inline void DepthStepY<true,DEPTH_FORMAT::DF_FLOAT32,X86_VERSION::SSE2>(void *depthStepY)
{
	__asm
	{
		mov eax,depthStepY;
		addps xmm1,[eax];
	}
}

//*****************************************************************************************
// template DepthWrite

template<bool depthWrite,X86_VERSION codeVersion>
inline void DepthWrite(POINTER depthAddress)
{
}

template<>
inline void DepthWrite<true,X86_VERSION::SSE2>(POINTER depthAddress)
{
	// xmm1 = depth variables
	// xmm5 = depth buffer values
	// xmm6 = depth pass mask
	
	__asm
	{
		movdqa xmm0,xmm1;
		pand xmm0,xmm6;

		pandn xmm6,xmm5;
		por xmm0,xmm6;

		mov edi,depthAddress;
		movdqa [edi],xmm0;
	}
}

template<Ceng::UINT32 stencilBits,Ceng::STENCIL_ACTION::value action,X86_VERSION codeVersion>
inline void DoStencilAction()
{
	// reserved: xmm0,xmm2,xmm4

	// xmm0 = stencil values
	// const xmm3 = stencil reference

	// xmm7 = action mask
}

template<>
inline void DoStencilAction<8,Ceng::STENCIL_ACTION::INCREMENT,X86_VERSION::SSE2>()
{
	__asm
	{
		psubb xmm0,xmm7;
	}
}

template<>
inline void DoStencilAction<8,Ceng::STENCIL_ACTION::INCREMENT_SAT,X86_VERSION::SSE2>()
{
	__asm
	{
		psubusb xmm0,xmm7;
	}
}

template<>
inline void DoStencilAction<8,Ceng::STENCIL_ACTION::DECREMENT,X86_VERSION::SSE2>()
{
	__asm
	{
		paddb xmm0,xmm7;
	}
}

template<>
inline void DoStencilAction<8,Ceng::STENCIL_ACTION::DECREMENT_SAT,X86_VERSION::SSE2>()
{
	__asm
	{
		paddusb xmm0,xmm7;
	}
}

template<>
inline void DoStencilAction<8,Ceng::STENCIL_ACTION::BIT_INVERT,X86_VERSION::SSE2>()
{
	__asm
	{
		pxor xmm0,xmm7;
	}
}

template<>
inline void DoStencilAction<8,Ceng::STENCIL_ACTION::ZERO,X86_VERSION::SSE2>()
{
	__asm
	{
		pandn xmm7,xmm0;
		movdqa xmm0,xmm7;			
	}
}

template<>
inline void DoStencilAction<8,Ceng::STENCIL_ACTION::SET_REF,X86_VERSION::SSE2>()
{
	__asm
	{
		movdqa xmm6,xmm7;

		pand xmm6,xmm3; // xmm6 = mask & reference

		pandn xmm7,xmm0; // xmm7 = ~(mask) & value

		por xmm7,xmm6;
		movdqa xmm0,xmm7;
	}
}


template<Ceng::UINT32 stencilBits,Ceng::STENCIL_ACTION::value stencilFailFunc,
	Ceng::STENCIL_ACTION::value depthFailFunc,Ceng::STENCIL_ACTION::value depthPassFunc,
	X86_VERSION codeVersion>
inline void StencilAction()
{
	// Stencil fail
	__asm
	{
		pcmpeqb xmm7,xmm7;
		pxor xmm7,xmm2;
		pand xmm7,xmm5; // xmm7 = ~(stencil pass) & coverage
	}

	DoStencilAction<stencilBits,stencilFailFunc,codeVersion>();

				
	// stencil pass + depth pass
	__asm
	{
		movdqa xmm7,xmm2;
		pand xmm7,xmm4;
		pand xmm7,xmm5; // xmm7 = (stencil pass) & (depth pass) & coverage
	}

	DoStencilAction<stencilBits,depthPassFunc,codeVersion>();

				
	// stencil pass + depth fail
	__asm
	{
		movdqa xmm7,xmm4;
		pandn xmm7,xmm2;
		pand xmm7,xmm5; // xmm7 = (stencil pass) & ~(depth pass) & coverage
	}
	
	DoStencilAction<stencilBits,depthFailFunc,codeVersion>();
}

//****************************************************************************************
// template LoadStencilValues

template<bool stencilEnable,X86_VERSION codeVersion>
inline void LoadStencilValues(POINTER stencilAddress,void *stencilCompareRef,void *stencilReadMask)
{
}

template<>
inline void LoadStencilValues<true,X86_VERSION::SSE2>(POINTER stencilAddress,void *stencilCompareRef,void *stencilReadMask)
{
	__asm
	{
		mov eax,stencilAddress;
		movdqa xmm2,[eax];

		mov eax,stencilCompareRef;
		movdqa xmm3,[eax];

		mov eax,stencilReadMask;
		movdqa xmm4,[eax];
	}
}

//****************************************************************************************
// template PrepareStencilValues

template<bool stencilEnable,Ceng::UINT32 stencilBits,X86_VERSION codeVersion>
inline void PrepareStencilValues()
{
}

template<>
inline void PrepareStencilValues<true,8,X86_VERSION::SSE2>()
{
	__asm
	{
		pand xmm2,xmm4;
		psubb xmm2,toSignedByte;
	}
}

//****************************************************************************************
// Template StencilTest

template<bool stencilEnable,Ceng::UINT32 stencilBits,Ceng::TEST_TYPE::value stencilTest,X86_VERSION codeVersion>
inline void StencilTest()
{
}

template<>
inline void StencilTest<true,8,Ceng::TEST_TYPE::LESS,X86_VERSION::SSE2>()
{
	__asm
	{
		pcmpgtb xmm2,xmm3; // xmm3 < xmm2
	}
}

template<>
inline void StencilTest<true,8,Ceng::TEST_TYPE::LESS_EQ,X86_VERSION::SSE2>()
{
	__asm
	{
		movdqa xmm4,xmm2;
							
		pcmpgtb xmm2,xmm3; // xmm3 < xmm2
		pcmpeqb xmm4,xmm3; // xmm3 == xmm2

		por xmm2,xmm4;
	}
}

template<>
inline void StencilTest<true,8,Ceng::TEST_TYPE::EQUAL,X86_VERSION::SSE2>()
{
	__asm
	{
		pcmpeqb xmm2,xmm3;
	}
}

template<>
inline void StencilTest<true,8,Ceng::TEST_TYPE::ABOVE_EQ,X86_VERSION::SSE2>()
{
	__asm
	{
		movdqa xmm4,xmm3;

		pcmpgtb xmm3,xmm2;
		pcmpeqb xmm4,xmm2;

		por xmm4,xmm3;
		movdqa xmm2,xmm4;
	}
}

template<>
inline void StencilTest<true,8,Ceng::TEST_TYPE::ABOVE,X86_VERSION::SSE2>()
{
	__asm
	{
		pcmpgtb xmm3,xmm2;
		movdqa xmm2,xmm3;
	}
}

template<>
inline void StencilTest<true,8,Ceng::TEST_TYPE::NOT_EQUAL,X86_VERSION::SSE2>()
{	
	__asm
	{
		pcmpeqb xmm4,xmm4;

		pcmpeqb xmm2,xmm3;
		pxor xmm2,xmm4;
	}
}

template<>
inline void StencilTest<true,8,Ceng::TEST_TYPE::ALWAYS_PASS,X86_VERSION::SSE2>()
{
	__asm
	{
		pcmpeqb xmm2,xmm2;
	}
}

template<>
inline void StencilTest<true,8,Ceng::TEST_TYPE::NEVER_PASS,X86_VERSION::SSE2>()
{
	__asm
	{
		pxor xmm2,xmm2;
	}
}

//****************************************************************************************
// template TrivialStencilTest
template<bool stencilEnable,X86_VERSION codeVersion>
inline void TrivialStencilTest()
{
}

template<>
void TrivialStencilTest<false,X86_VERSION::SSE2>()
{
	__asm
	{
		pcmpeqb xmm2,xmm2;
	}
}

//****************************************************************************************
// template FullTest 

/*
template<bool stencilEnable,bool stencilWrite,bool depthEnable,bool depthWrite,
			UINT32 stencilBits,UINT32 xIters,DEPTH_FORMAT depthFormat,
			Ceng::TEST_TYPE::value stencilTest,Ceng::TEST_TYPE::value depthTest,
			Ceng::STENCIL_ACTION::value stencilFailFunc,Ceng::STENCIL_ACTION::value depthFailFunc,
			Ceng::STENCIL_ACTION::value depthPassFunc,
			X86_VERSION codeVersion>
void FullTest(const UINT32 tileX,const UINT32 tileY,void *depthValues,void *depthStepX,void *depthStepY,
				void *stencilCompareRef,void *stencilRef,void *stencilReadMask,void *stencilWriteMask,
			  UINT64 coverageMask,UINT64 *resultMask)
{

	Ceng::UINT32 quadX;
	Ceng::UINT32 quadY = 0;

	POINTER depthAddress;
	POINTER stencilAddress;

	// Load quad depth values if there won't be a register spill
	PrepareDepth<depthEnable,depthFormat,codeVersion>(depthVariables);

	do
	{
		quadX = 0;

		do
		{
			
			// Stencil test as many quads as fit into 16 bytes
			LoadStencilValues<stencilEnable,codeVersion>(stencilAddress,stencilCompareRef,stencilReadMask);

			PrepareStencilValues<stencilEnable,stencilBits,codeVersion>();

			StencilTest<stencilEnable,stencilBits,stencilTest,codeVersion>();

			// xmm2 = stencil pass mask in stencil format

			GenerateCoverageMask<depthEnable,stencilBits,codeVersion>(coverageMask);

			// Depth test a number of quads equal to those in the stencil test
			DepthTest<depthEnable,depthWrite,depthFormat,stencilBits,depthTest,codeVersion>(depthAddress);

			StencilWrite<stencilWrite,stencilBits,stencilFailFunc,depthFailFunc,depthPassFunc>(stencilAddress);

			AddressStepping<stencilBits,depthFormat>(&depthAddress,&stencilAddress);
			

			quadX++;

		} while (quadX < xIters);

		DepthStepY<depthEnable,depthFormat,codeVersion>(depthStepY);

		quadY++;
	} while (quadY < 4);
}
*/

void CR_NewTargetData::DepthStencilTestTile(const Ceng::UINT32 tileSize,const Ceng::UINT32 tileX,
											const Ceng::UINT32 tileY,
											void *depthValues,void *depthStepX,void *depthStepY,
											Ceng::UINT64 coverageMask,Ceng::UINT64 *resultMask,
											Ceng::BOOL frontFace)
{
	dsLocal.depthEnable = true;
	dsLocal.stencilEnable = true;

	if (dsLocal.depthEnable == false && dsLocal.stencilEnable == false)
	{
		*resultMask = coverageMask;
		return;
	}

	POINTER depthAddress=0,stencilAddress=0;

	depthAddress = GetTileAddress(8,0,tileX>>3,tileY>>3);

	/*
	depthAddress = depthBuffer->baseAddress + (tileX>>3) * depthBuffer->channels[0].tileXstep +
						(tileY>>3) * depthBuffer->channels[0].tileYstep;
						*/

	_declspec(align(16)) UINT8 writeMaskArray[16];
	_declspec(align(16)) UINT8 readMaskArray[16];

	_declspec(align(16)) INT8 compareRefArray[16];

	_declspec(align(16)) UINT8 stencilRefArray[16];

	_declspec(align(16)) INT8 coverageTemp[16];

	_declspec(align(16)) INT8 stencilPassTemp[16];
	_declspec(align(16)) INT8 depthPassTemp[16];

	Ceng::TEST_TYPE::value stencilTest;
	
	Ceng::STENCIL_ACTION::value stencilFailFunc,depthFailFunc,depthPassFunc;
	Ceng::STENCIL_ACTION::value action;
	
	
	UINT8 stencilRef,compareRef;
	UINT8 stencilReadMask,stencilWriteMask;

	// TODO: These should be set only once per triangle
	if (frontFace)
	{
		stencilTest = dsTest.frontFace.stencilTest;
		
		stencilReadMask = dsTest.frontFace.stencilReadMask;
		stencilWriteMask = dsTest.frontFace.stencilWriteMask;

		stencilRef = dsTest.frontFace.stencilRef;
		compareRef = stencilRef & stencilReadMask;

		stencilFailFunc = dsTest.frontFace.stencilFail;
		depthFailFunc = dsTest.frontFace.depthFail;
		depthPassFunc = dsTest.frontFace.depthPass;
	}
	else
	{
		stencilTest = dsTest.backFace.stencilTest;
		
		stencilReadMask = dsTest.backFace.stencilReadMask;
		stencilWriteMask = dsTest.backFace.stencilWriteMask;

		stencilRef = dsTest.backFace.stencilRef;
		compareRef = stencilRef & stencilReadMask;

		stencilFailFunc = dsTest.backFace.stencilFail;
		depthFailFunc = dsTest.backFace.depthFail;
		depthPassFunc = dsTest.backFace.depthPass;
	}

	UINT32 quadXiters = 4;

	int k;

	if (dsLocal.stencilEnable)
	{
		if (channels[0].mixedData)
		{
			stencilAddress = depthAddress;
		}
		else
		{
			stencilAddress = depthAddress + channels[1].bufferOffset;

			switch(bufferFormat)
			{
			case Ceng::IMAGE_FORMAT::D24_S8:

				quadXiters = 4;

				for(k=0;k<16;k++)
				{
					readMaskArray[k] = 0;
					stencilRefArray[k] = 0;
					compareRefArray[k] = 0;

					writeMaskArray[k] = 0;
				}

				readMaskArray[0] = stencilReadMask;
				readMaskArray[4] = stencilReadMask;
				readMaskArray[8] = stencilReadMask;
				readMaskArray[12] = stencilReadMask;

				stencilRefArray[0] =  stencilRef;
				stencilRefArray[4] =  stencilRef;
				stencilRefArray[8] =  stencilRef;
				stencilRefArray[12] =  stencilRef;

				compareRefArray[0] =  INT32(compareRef) - 128;
				compareRefArray[4] =  INT32(compareRef) - 128;
				compareRefArray[8] =  INT32(compareRef) - 128;
				compareRefArray[12] =  INT32(compareRef) - 128;

				writeMaskArray[0] = stencilWriteMask;
				writeMaskArray[4] = stencilWriteMask;
				writeMaskArray[8] = stencilWriteMask;
				writeMaskArray[12] = stencilWriteMask;

				break;

			case Ceng::IMAGE_FORMAT::D16_S8:
			case Ceng::IMAGE_FORMAT::D32F_S8:
			case Ceng::IMAGE_FORMAT::D32F_IW_S8:
			case Ceng::IMAGE_FORMAT::D32F_W_S8:

				quadXiters = 1;

				// TODO: should be done at test configuration

				for(k=0;k<16;k++)
				{
					readMaskArray[k] = stencilReadMask;

					stencilRefArray[k] = stencilRef;

					compareRefArray[k] = INT32(compareRef) - 128;
				}

				break;
			}
		}
	}

	
	*resultMask = 0;

	UINT32 quadX,quadY;

	
	if (dsTest.depthEnable)
	{
		__asm
		{
			mov eax,depthValues;
			movaps xmm1,[eax]; // xmm1 = packedZ
		}
	}

	UINT32 finalMask;	

	for(quadY=0;quadY<4;quadY++)
	{
		finalMask = 0;

		// Stencil test

		for(quadX=0; quadX < quadXiters; quadX++)
		{	
			
			LoadStencilValues<true,X86_VERSION::SSE2>(stencilAddress,(void*)compareRefArray,(void*)readMaskArray);

			PrepareStencilValues<true,8,X86_VERSION::SSE2>();

			StencilTest<true,8,Ceng::TEST_TYPE::EQUAL,X86_VERSION::SSE2>();

			TrivialStencilTest<true,X86_VERSION::SSE2>();

			// Store pure stencil pass mask

			__asm
			{
				movdqa stencilPassTemp,xmm2;
			}

			// xmm1 = quad depth values
			// xmm7 = depthStepX

			// xmm2 = stencil test result

			// xmm5 = depth buffer values

			// Extract coverage mask for 4 quads
			UINT32 temp = ( coverageMask >> (16*quadY + 4*quadX)) & 65535;

			GetCoverageMask16_x86_SSE2(temp,bufferFormat);

			// Store coverage mask
			__asm
			{
				movdqa coverageTemp,xmm3;
			}

			if (dsLocal.depthEnable)
			{
				// Combine coverage mask with stencil pass mask
				__asm
				{
					pand xmm2,xmm3;
				}

				// xmm2 = stencil pass & coverage

				switch(bufferFormat)
				{
				case Ceng::IMAGE_FORMAT::D32F:
				case Ceng::IMAGE_FORMAT::D32F_W:
				case Ceng::IMAGE_FORMAT::D32F_IW:

				case Ceng::IMAGE_FORMAT::D32F_S8:
				case Ceng::IMAGE_FORMAT::D32F_W_S8:
				case Ceng::IMAGE_FORMAT::D32F_IW_S8:

					__asm
					{
						// Convert stencil pass mask to int32
						movdqa xmm3,xmm2;

						// Stencil pass mask for quads 1 and 2
						punpcklbw xmm3,xmm3;

						// xmm4 = Stencil pass mask for quad 1
						movdqa xmm4,xmm3;
						punpcklwd xmm4,xmm4; 
					}

					__asm
					{
						// Read depth buffer data
						mov edi,depthAddress;
						movaps xmm5,[edi];
					}
 
					DepthTest<DEPTH_FORMAT::DF_FLOAT32,Ceng::TEST_TYPE::LESS,X86_VERSION::SSE2>();

					// Extract final visibility mask
					__asm
					{
						movaps xmm0,xmm6;
						andps xmm6,xmm4; // coverage & stencil pass & depth pass
					
						movmskps eax,xmm6;
						mov finalMask,eax;

						movaps xmm4,xmm0;
					}

					DepthWrite<true,X86_VERSION::SSE2>(depthAddress);

					// Load depth stepping
					__asm
					{
						mov esi,depthStepX;
						movaps xmm7,[esi];
					}
	
					//*******************************************
					// Second quad

					__asm
					{
						addps xmm1,xmm7; // xmm1 += depthStepX

						// xmm3 = stencil pass mask for quad 2
						punpckhwd xmm3,xmm3;

						// xmm3 = depth pass mask for quad 1
						// xmm4 = stencil pass mask for quad 2
						movaps xmm0,xmm4;
						movaps xmm4,xmm3;
						movaps xmm3,xmm0;

						// Read depth value
						mov edi,depthAddress;
						movaps xmm5,[edi+16];
					}

					DepthTest<DEPTH_FORMAT::DF_FLOAT32,Ceng::TEST_TYPE::LESS,X86_VERSION::SSE2>();

					// Extract final visibility mask
					__asm
					{
						movaps xmm0,xmm6;
						andps xmm6,xmm4;

						mov eax,finalMask;
						
						movmskps edx,xmm6;
						shl edx,4;
						or eax,edx;

						// xmm4 = depth pass mask for quad 2
						movaps xmm4,xmm0;

						mov finalMask,eax;
					}

					// Stencil pass + depth pass mask to int8
					__asm
					{
						packssdw xmm3,xmm4;
						movdqa depthPassTemp,xmm3;
					}

					DepthWrite<true,X86_VERSION::SSE2>(depthAddress+16);
					
					//***********************************
					// Third quad

					__asm
					{
						movdqa xmm3,xmm2;

						// Stencil pass mask for quads 3 and 4
						punpckhbw xmm3,xmm3;

						// xmm4 = stencil pass mask for quad 3
						movdqa xmm4,xmm3;
						punpcklwd xmm4,xmm4;

						addps xmm1,xmm7; // xmm1 += depthStepX

						mov edi,depthAddress;
						movaps xmm5,[edi+32];
					}

					DepthTest<DEPTH_FORMAT::DF_FLOAT32,Ceng::TEST_TYPE::LESS,X86_VERSION::SSE2>();

					__asm
					{
						movaps xmm0,xmm6;
						andps xmm6,xmm4;
					}

					// Extract final visibility mask	
					__asm
					{
						mov eax,finalMask;

						movmskps edx,xmm6;
						shl edx,8;
						or eax,edx;

						movaps xmm4,xmm0;

						mov finalMask,eax;
					}

					DepthWrite<true,X86_VERSION::SSE2>(depthAddress+32);

					//**************************************
					// Fourth quad

					__asm
					{
						// xmm3 = stencil pass mask for quad 4
						punpckhwd xmm3,xmm3;

						movaps xmm0,xmm4;
						movaps xmm4,xmm3;
						movaps xmm3,xmm0;

						addps xmm1,xmm7; // xmm1 += depthStepX

						mov edi,depthAddress;
						movaps xmm5,[edi+48];
					}

					DepthTest<DEPTH_FORMAT::DF_FLOAT32,Ceng::TEST_TYPE::LESS,X86_VERSION::SSE2>();

					// Extract final visibility mask
					__asm
					{
						movaps xmm0,xmm6;
						andps xmm6,xmm4;

						mov eax,finalMask;

						movmskps edx,xmm6;
						shl edx,12;
						or eax,edx;

						movaps xmm4,xmm0;

						mov finalMask,eax;
					}

					__asm
					{
						packssdw xmm3,xmm4;

						movdqa xmm4,depthPassTemp;
						packsswb xmm4,xmm3;
					}

					DepthWrite<true,X86_VERSION::SSE2>(depthAddress+48);

					break;
				}
			
			}
			else
			{
				// depth test disabled

				__asm
				{
					cmpeqps xmm4,xmm4;
				}
			}

			// xmm0 = stencil buffer values

			// xmm2 = stencil pass mask
			// xmm4 = depth pass mask

			if (dsLocal.stencilEnable)
			{
				// Reload stencil buffer values
				__asm
				{
					mov esi,stencilAddress;
					movdqa xmm0,[esi];
				}

				// Reload stencil pass mask
				__asm
				{
					movdqa xmm2,stencilPassTemp;
				}

				// Reload coverage mask
				__asm
				{
					movdqa xmm5,coverageTemp;
				}

				// Load stencil reference value
				__asm
				{
					movdqa xmm3,stencilRefArray;
				}

				StencilAction<8,Ceng::STENCIL_ACTION::KEEP,Ceng::STENCIL_ACTION::ZERO,Ceng::STENCIL_ACTION::KEEP,X86_VERSION::SSE2>();
				
				__asm
				{
					mov edi,stencilAddress;
					movdqa [edi],xmm0;
				}
			}

			switch(bufferFormat)
			{
			case Ceng::IMAGE_FORMAT::D32F:
			case Ceng::IMAGE_FORMAT::D32F_W:
			case Ceng::IMAGE_FORMAT::D32F_IW:

			case Ceng::IMAGE_FORMAT::D32F_S8:
			case Ceng::IMAGE_FORMAT::D32F_W_S8:
			case Ceng::IMAGE_FORMAT::D32F_IW_S8:

				stencilAddress += 16;
				depthAddress += 4*16;

				break;				
			}
			
		
		} // for quadX

		*resultMask |= UINT64(finalMask) << (16*quadY);

		if (dsLocal.depthEnable)
		{
			if (quadY<3)
			{
				__asm
				{
					mov esi,depthStepY;
					addps xmm1,[esi];
				}
			}
		}
	}

	//*resultMask = coverageMask;

	/*

	FLOAT32 *packedZ = (FLOAT32*)depthValues;
	
	VectorF4 *dStepX = (VectorF4*)depthStepX;
	VectorF4 *dStepY = (VectorF4*)depthStepY;

	POINTER depthAddress,stencilAddress;

	depthAddress = depthBuffer->baseAddress + (tileX>>3) * depthBuffer->channels[0].tileXstep +
						(tileY>>3) * depthBuffer->channels[0].tileYstep;

	UINT8 *stencilValue;

	if (dsLocal.stencilEnable)
	{
		if (depthBuffer->channels[0].mixedData)
		{
		}
		else
		{
			stencilAddress = depthAddress + depthBuffer->channels[1].bufferOffset;
		}

		stencilValue = (UINT8*)stencilAddress;
	}

	FLOAT32 *depth = (FLOAT32*)depthAddress;
	

	Ceng::TEST_TYPE::value stencilTest;
	
	Ceng::STENCIL_ACTION::value stencilFailFunc,depthFailFunc,depthPassFunc;
	Ceng::STENCIL_ACTION::value action;
	
	
	UINT8 stencilRef,compareRef;
	UINT8 stencilReadMask,stencilWriteMask;

	// TODO: These should be set only once per triangle
	if (frontFace)
	{
		stencilTest = dsTest.frontFace.stencilTest;
		
		stencilReadMask = dsTest.frontFace.stencilReadMask;
		stencilWriteMask = dsTest.frontFace.stencilWriteMask;

		stencilRef = dsTest.frontFace.stencilRef;
		compareRef = stencilRef & stencilReadMask;

		stencilFailFunc = dsTest.frontFace.stencilFail;
		depthFailFunc = dsTest.frontFace.depthFail;
		depthPassFunc = dsTest.frontFace.depthPass;
	}
	else
	{
		stencilTest = dsTest.backFace.stencilTest;
		
		stencilReadMask = dsTest.backFace.stencilReadMask;
		stencilWriteMask = dsTest.backFace.stencilWriteMask;

		stencilRef = dsTest.backFace.stencilRef;
		compareRef = stencilRef & stencilReadMask;

		stencilFailFunc = dsTest.backFace.stencilFail;
		depthFailFunc = dsTest.backFace.depthFail;
		depthPassFunc = dsTest.backFace.depthPass;
	}

	UINT32 stencilResult,depthResult;
	UINT32 quadX,quadY,pixel;

	UINT32 quadMask;
	UINT32 quadIndex=0;
	UINT32 shiftValue=0;

	UINT32 tempResult;

	*resultMask = 0;

	for(quadY=0;quadY<8;quadY+=2)
	{
		for(quadX=0;quadX<8;quadX+=2)
		{
			tempResult = 0;
			quadMask = UINT32(coverageMask >> shiftValue) & 15;

			for(pixel=0;pixel<4;pixel++)
			{
				if (quadMask & 1)
				{
					// Stencil test

					if (dsLocal.stencilEnable)
					{
						stencilResult = 0;

						// TODO: Use callback
						switch(stencilTest)
						{
						case Ceng::TEST_TYPE::LESS:
							if ( compareRef < (stencilValue[quadIndex+pixel] & stencilReadMask) )
							{
								stencilResult = 1;
							}
							break;
						case Ceng::TEST_TYPE::LESS_EQ:
							if ( compareRef <= (stencilValue[quadIndex+pixel] & stencilReadMask) )
							{
								stencilResult = 1;
							}
							break;
						case Ceng::TEST_TYPE::EQUAL:
							if ( compareRef == (stencilValue[quadIndex+pixel] & stencilReadMask) )
							{
								stencilResult = 1;
							}
							break;
						case Ceng::TEST_TYPE::NOT_EQUAL:
							if ( compareRef != (stencilValue[quadIndex+pixel] & stencilReadMask) )
							{
								stencilResult = 1;
							}
							break;
						case Ceng::TEST_TYPE::ABOVE:
							if ( compareRef > (stencilValue[quadIndex+pixel] & stencilReadMask) )
							{
								stencilResult = 1;
							}
							break;
						case Ceng::TEST_TYPE::ABOVE_EQ:
							if ( compareRef >= (stencilValue[quadIndex+pixel] & stencilReadMask) )
							{
								stencilResult = 1;
							}
							break;
						case Ceng::TEST_TYPE::ALWAYS_PASS:
							stencilResult = 1;
							break;			
						case Ceng::TEST_TYPE::NEVER_PASS:
							stencilResult = 0;
							break;						
						}
					}
					else
					{
						stencilResult = 1;
					}

					// Depth test
					
					if (dsLocal.depthEnable)
					{
						depthResult = 0;

						switch(dsTest.depthTest)
						{
						case Ceng::TEST_TYPE::LESS_EQ:
							if (packedZ[pixel] <= depth[quadIndex+pixel])
							{
								if (dsTest.depthWrite)
								{
									depth[quadIndex+pixel] = packedZ[pixel];
								}
								depthResult = 1;
							}
							break;
						case Ceng::TEST_TYPE::LESS:
							if (packedZ[pixel] < depth[quadIndex+pixel])
							{
								if (dsTest.depthWrite)
								{
									depth[quadIndex+pixel] = packedZ[pixel];
								}
								depthResult = 1;
							}
							break;
						case Ceng::TEST_TYPE::EQUAL:
							if (packedZ[pixel] == depth[quadIndex+pixel])
							{
								if (dsTest.depthWrite)
								{
									depth[quadIndex+pixel] = packedZ[pixel];
								}
								depthResult = 1;
							}
							break;
						case Ceng::TEST_TYPE::NOT_EQUAL:
							if (packedZ[pixel] != depth[quadIndex+pixel])
							{
								if (dsTest.depthWrite)
								{
									depth[quadIndex+pixel] = packedZ[pixel];
								}
								depthResult = 1;
							}
							break;
						case Ceng::TEST_TYPE::ABOVE_EQ:
							if (packedZ[pixel] >= depth[quadIndex+pixel])
							{
								if (dsTest.depthWrite)
								{
									depth[quadIndex+pixel] = packedZ[pixel];
								}
								depthResult = 1;
							}
							break;
						case Ceng::TEST_TYPE::ABOVE:
							if (packedZ[pixel] > depth[quadIndex+pixel])
							{
								if (dsTest.depthWrite)
								{
									depth[quadIndex+pixel] = packedZ[pixel];
								}
								depthResult = 1;
							}
							break;
						case Ceng::TEST_TYPE::ALWAYS_PASS:
							depthResult = 1;
							break;
						case Ceng::TEST_TYPE::NEVER_PASS:
							depthResult = 0;
							break;
						}
					}
					else
					{
						depthResult = 1;
					}

					tempResult |= ((stencilResult & depthResult) << pixel);

					// Stencil action

					if (dsLocal.stencilEnable)
					{
						if (stencilResult == 0)
						{
							action = stencilFailFunc;
						}
						else
						{
							action = depthPassFunc;
							
							if (depthResult == 0)
							{
								action = depthFailFunc;
							}
						}
						
						UINT8 oldStencil = stencilValue[quadIndex+pixel];
						UINT8 newStencil = 0;
						INT32 saturateStencil;
						
						switch(action)
						{
						case Ceng::STENCIL_ACTION::KEEP:
							break;
						case Ceng::STENCIL_ACTION::ZERO:
							newStencil = 0;
							stencilValue[quadIndex+pixel] = (oldStencil & ~stencilWriteMask) | newStencil;
							break;
						case Ceng::STENCIL_ACTION::SET_REF:
							newStencil = stencilRef & stencilWriteMask;
							stencilValue[quadIndex+pixel] = (oldStencil & ~stencilWriteMask) | newStencil;
							break;
						case Ceng::STENCIL_ACTION::BIT_INVERT:
							newStencil = (oldStencil & stencilWriteMask) ^ -1;
							stencilValue[quadIndex+pixel] = (oldStencil & ~stencilWriteMask) | newStencil;
							break;
						case Ceng::STENCIL_ACTION::INCREMENT:
							newStencil = (oldStencil & stencilWriteMask) + 1;
							stencilValue[quadIndex+pixel] = (oldStencil & ~stencilWriteMask) | newStencil;
							break;
						case Ceng::STENCIL_ACTION::DECREMENT:
							newStencil = (oldStencil & stencilWriteMask) - 1;
							stencilValue[quadIndex+pixel] = (oldStencil & ~stencilWriteMask) | newStencil;
							break;
						case Ceng::STENCIL_ACTION::INCREMENT_SAT:
							saturateStencil = INT32(oldStencil & stencilWriteMask) + 1;
							
							if (saturateStencil > 255)
							{
								newStencil = 255;
							}
							else
							{
								newStencil = UINT8(saturateStencil);
							}
							
							stencilValue[quadIndex+pixel] = (oldStencil & ~stencilWriteMask) | newStencil;
							break;
						case Ceng::STENCIL_ACTION::DECREMENT_SAT:
							saturateStencil = INT32(oldStencil & stencilWriteMask) - 1;
							
							if (saturateStencil < 0)
							{
								newStencil = 0;
							}
							else
							{
								newStencil = UINT8(saturateStencil);
							}
							
							stencilValue[quadIndex+pixel] = (oldStencil & ~stencilWriteMask) | newStencil;
							break;
						}
					}

				} // quadMask

				quadMask = quadMask >> 1;

			} // pixel

			*resultMask |= (UINT64(tempResult) << shiftValue);
			
			quadIndex += 4;
			shiftValue += 4;

			if (quadX < 6)
			{
				packedZ[0] += dStepX->x;
				packedZ[1] += dStepX->y;
				packedZ[2] += dStepX->z;
				packedZ[3] += dStepX->w;
			}

		} // quadX

		if (quadY < 6)
		{
			packedZ[0] += dStepY->x;
			packedZ[1] += dStepY->y;
			packedZ[2] += dStepY->z;
			packedZ[3] += dStepY->w;
		}
	
	} // quadY

	*/
}