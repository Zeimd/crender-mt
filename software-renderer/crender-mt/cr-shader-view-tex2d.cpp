/*****************************************************************************
*
* cr-shader-view-tex2d.cpp
*
* Created By Jari Korkala 5/3/2015
*
*****************************************************************************/

#include "cr-sampler-state.h"

#include "cr-shader-view-tex2d.h"

#include "shader-datatype.h"

using namespace Ceng;

CR_ShaderViewTex2D::FilterFunction CR_ShaderViewTex2D::minFilterTable[16] =
{
	&CR_ShaderViewTex2D::Nearest_SSE2,
	//&CR_ShaderViewTex2D::Nearest_Port,
	&CR_ShaderViewTex2D::Linear_SSE2,
	//&CR_ShaderViewTex2D::Linear_Port,
	&CR_ShaderViewTex2D::Nearest_MipNearest_SSE2,
	&CR_ShaderViewTex2D::Nearest_MipLinear,
	&CR_ShaderViewTex2D::Linear_MipNearest_SSE2,
	&CR_ShaderViewTex2D::Linear_MipLinear,
};

CR_ShaderViewTex2D::FilterFunction CR_ShaderViewTex2D::magFilterTable[8] =
{
	&CR_ShaderViewTex2D::Nearest_SSE2,
	//&CR_ShaderViewTex2D::Nearest_Port,
	&CR_ShaderViewTex2D::Linear_SSE2,
	//&CR_ShaderViewTex2D::Linear_Port,
};

CR_ShaderViewTex2D::CR_ShaderViewTex2D()
{
}

CR_ShaderViewTex2D::CR_ShaderViewTex2D(const Texture2dDesc &desc, TextureMipVector &&textures)
	: desc(desc),textures(textures)
{
}

CR_ShaderViewTex2D::~CR_ShaderViewTex2D()
{
}

void CR_ShaderViewTex2D::Release()
{
	delete this;
}

// NOTE: This is a debug value, dividend should be 1.0
const Ceng::FLOAT32 colorScale = 0.6f / 255.0f;

//const Ceng::FLOAT32 colorScale = 1.0f / 255.0f;

const Ceng::INT32 FIXED_HALF = 1 << (15);
const Ceng::INT32 FIXED_ROUNDPOS = FIXED_HALF;
const Ceng::INT32 FIXED_ROUNDNEG = FIXED_HALF - 1;

const Ceng::INT32 floorMask = ~65535;

_declspec(align(16)) const Ceng::FLOAT32 mulToFX16[] = { 65536.0f, 65536.0f, 65536.0f, 65536.0f };

const Ceng::FLOAT32 distVal = 2.0f;

void CR_ShaderViewTex2D::Sample2D(Shader::Float2 &uv, const CR_SamplerState *sampler,
	Ceng::FLOAT32 *out_colorVecs)
{
	Ceng::FLOAT32 *uvData = (Ceng::FLOAT32*)uv.dataAddress;

	_declspec(align(16)) Ceng::INT32 uFX[4], vFX[4];

	__m128 scaleMul = _mm_load_ps(mulToFX16);

	// Convert to 16.16 fixed point
	/*
	for (k = 0; k < 4; ++k)
	{
		uFX[k] = Ceng::INT32(65536.0f * uvData[k]);

		vFX[k] = Ceng::INT32(65536.0f * uvData[4 + k]);
	}
	*/

	__m128 uLoad = _mm_load_ps(uvData);
		
	__m128 uScaled = _mm_mul_ps(uLoad,scaleMul);

	__m128i out = _mm_cvttps_epi32(uScaled);

	_mm_store_si128((__m128i*)uFX, out);

	__m128 vLoad = _mm_load_ps(&uvData[4]);

	__m128 vScaled = _mm_mul_ps(vLoad, scaleMul);

	out = _mm_cvttps_epi32(vScaled);

	_mm_store_si128((__m128i*)vFX, out);

	// Calculate derivatives

	struct Derivatives
	{
		Ceng::FLOAT32 du_dx;
		Ceng::FLOAT32 du_dy;
		Ceng::FLOAT32 dv_dx;
		Ceng::FLOAT32 dv_dy;		
	};

	Ceng::FLOAT32 baseDist;

	/*
	derivatives.du_dx = (uvData[1] - uvData[0])*textures[0]->bufferWidth;
	derivatives.du_dy = (uvData[2] - uvData[0])*textures[0]->bufferWidth;

	derivatives.dv_dx = (uvData[5] - uvData[4])*textures[0]->bufferHeight;
	derivatives.dv_dy = (uvData[6] - uvData[4])*textures[0]->bufferHeight;

	Ceng::FLOAT32 derivDistX = derivatives.du_dx*derivatives.du_dx + derivatives.dv_dx*derivatives.dv_dx;
	Ceng::FLOAT32 derivDistY = derivatives.du_dy*derivatives.du_dy + derivatives.dv_dy*derivatives.dv_dy;

	baseDist = derivDistX;

	if (baseDist < derivDistY)
	{
	baseDist = derivDistY;
	}
	*/
	
	__m128 uTopLeft = _mm_shuffle_ps(uLoad,uLoad, 0);

	__m128 du_Vec = _mm_sub_ps(uLoad,uTopLeft);

	__m128 widthVec = _mm_load1_ps((float*)&textures[0]->bufferWidth);

	widthVec = _mm_cvtepi32_ps(*(__m128i*)&widthVec);

	du_Vec = _mm_mul_ps(du_Vec,widthVec);

	__m128 vTopLeft = _mm_shuffle_ps(vLoad,vLoad, 0);

	__m128 dv_Vec = _mm_sub_ps(vLoad,vTopLeft);

	__m128 heightVec = _mm_load1_ps((float*)&textures[0]->bufferHeight);

	heightVec = _mm_cvtepi32_ps(*(__m128i*)&heightVec);

	dv_Vec = _mm_mul_ps(dv_Vec, heightVec);

	du_Vec = _mm_mul_ps(du_Vec, du_Vec);
	dv_Vec = _mm_mul_ps(dv_Vec, dv_Vec);

	__m128 dotVec = _mm_add_ps(du_Vec, dv_Vec);

	dv_Vec = _mm_shuffle_ps(dotVec, dotVec, _MM_SHUFFLE(0,0,2,0));

	dv_Vec = _mm_max_ps(du_Vec, dv_Vec);

	dv_Vec = _mm_shuffle_ps(dv_Vec, dv_Vec, 1);

	_mm_store_ss(&baseDist, dv_Vec);	

	// TODO: Is rounding neccessary with OpenGL rules?

	/*
	Ceng::INT32 uRounding = 0;
	Ceng::INT32 vRounding = 0;

	if (du_dx > 0)
	{
		uRounding = FIXED_ROUNDPOS;
	}
	else if (du_dx < 0)
	{
		uRounding = FIXED_ROUNDNEG;
	}
	else
	{
		uRounding = FIXED_ROUNDPOS;

		if (du_dy < 0)
		{
			uRounding = FIXED_ROUNDNEG;
		}
	}

	if (dv_dx > 0)
	{
		vRounding = FIXED_ROUNDPOS;
	}
	else if (dv_dx < 0)
	{
		vRounding = FIXED_ROUNDNEG;
	}
	else
	{
		vRounding = FIXED_ROUNDPOS;

		if (dv_dy < 0)
		{
			vRounding = FIXED_ROUNDNEG;
		}
	}
	*/

	__m128 vecBaseDist = _mm_load_ss(&baseDist);
	__m128 distComp = _mm_load_ss(&distVal);

	if (_mm_comigt_ss(vecBaseDist,distComp))
	//if (baseDist > 2.00f) // NOTE: 1.5*1.5
	{
		(this->*minFilterTable[sampler->desc.minFilter])(uFX, vFX, 0,baseDist, out_colorVecs);
		//Linear_MipNearest_SSE2(uFX, vFX, 0, baseDist, out_colorVecs);
	}
	else
	{
		//Linear_SSE2(uFX, vFX, 0, baseDist, out_colorVecs);
		(this->*magFilterTable[sampler->desc.magFilter])(uFX, vFX, 0,baseDist, out_colorVecs);
	}
}

const Ceng::UINT32 GetMipLevelInt(const Ceng::FLOAT32 mipFactor)
{
	// Calculate mipmap level

	Ceng::UINT32 baseLevel = *(Ceng::UINT32*)&mipFactor;

	// NOTE: Assuming IEEE 32-bit floats

	baseLevel >>= 23;
	baseLevel -= 127;

	//mipLevel = 0.5f*floor(log2(mipFactor));
	return Ceng::INT32(baseLevel >> 1);
}

void CR_ShaderViewTex2D::Nearest_MipNearest_Port(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel, 
	const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color)
{
	mipLevel = GetMipLevelInt(mipFactor);

	// TODO: Remove branch
	if (mipLevel > desc.mipLevels - 1)
	{
		mipLevel = desc.mipLevels - 1;
	}

	Nearest_Port(uFX, vFX, mipLevel, mipFactor, out_color);
}

void CR_ShaderViewTex2D::Nearest_MipNearest_SSE2(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel,
	const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color)
{
	mipLevel = GetMipLevelInt(mipFactor);

	// TODO: Remove branch
	if (mipLevel > desc.mipLevels - 1)
	{
		mipLevel = desc.mipLevels - 1;
	}

	Nearest_SSE2(uFX, vFX, mipLevel, mipFactor, out_color);
}

void CR_ShaderViewTex2D::Linear_MipNearest_Port(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel, 
	const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color)
{
	mipLevel = GetMipLevelInt(mipFactor);

	// TODO: Remove branch
	if (mipLevel > desc.mipLevels - 1)
	{
		mipLevel = desc.mipLevels - 1;
	}

	Linear_Port(uFX, vFX, mipLevel, mipFactor, out_color);
}


void CR_ShaderViewTex2D::Linear_MipNearest_SSE2(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel,
	const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color)
{
	mipLevel = GetMipLevelInt(mipFactor);

	// TODO: Remove branch
	if (mipLevel > desc.mipLevels - 1)
	{
		mipLevel = desc.mipLevels - 1;
	}

	Linear_SSE2(uFX, vFX, mipLevel, mipFactor, out_color);
}


void CR_ShaderViewTex2D::Nearest_MipLinear(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel, 
	const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color)
{
	/*
	// Calculate mipmap level

	Ceng::FLOAT32 baseLevel = 0.5f*log2(mipFactor);

	Ceng::FLOAT32 intPart;

	Ceng::FLOAT32 fracPart = modf(baseLevel, &intPart);

	Ceng::INT32 mipLevel = Ceng::INT32(baseLevel);

	if (mipLevel > desc.mipLevels - 1)
	{
		mipLevel = desc.mipLevels - 1;
	}

	Ceng::UINT32 colorA[16], colorB[16];

	NearestFetch(uFX, vFX, mipLevel, colorA);

	Ceng::UINT32 nextMip = mipLevel + 1;

	if (nextMip == desc.mipLevels)
	{
		--nextMip;
	}

	NearestFetch(uFX, vFX,  nextMip, colorA);

	Ceng::UINT32 factor = Ceng::UINT32(65536.0f*fracPart);

	for (Ceng::UINT32 k = 0; k < 4; ++k)
	{
		Ceng::UINT32 reverse = 65535 - factor;

		out_color[4 * k + 0] = (reverse*colorA[4 * k + 0] + factor*colorB[4 * k + 0]) >> 16;
		out_color[4 * k + 1] = (reverse*colorA[4 * k + 1] + factor*colorB[4 * k + 1]) >> 16;
		out_color[4 * k + 2] = (reverse*colorA[4 * k + 2] + factor*colorB[4 * k + 2]) >> 16;
		out_color[4 * k + 3] = (reverse*colorA[4 * k + 3] + factor*colorB[4 * k + 3]) >> 16;
	}
	*/
}

void CR_ShaderViewTex2D::Linear_MipLinear(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX,
	Ceng::UINT32 mipLevel, const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color)
{
	/*
	// Calculate mipmap level

	Ceng::FLOAT32 baseLevel = 0.5f*log2(mipFactor);

	Ceng::FLOAT32 intPart;

	Ceng::FLOAT32 fracPart = modf(baseLevel, &intPart);

	Ceng::INT32 mipLevel = Ceng::INT32(baseLevel);

	if (mipLevel > desc.mipLevels - 1)
	{
		mipLevel = desc.mipLevels - 1;
	}


	Ceng::UINT32 colorA[16], colorB[16];

	LinearFetch(uFX, vFX, mipLevel,colorA);
	
	Ceng::UINT32 nextMip = mipLevel + 1;

	if (nextMip == desc.mipLevels)
	{
		--nextMip;
	}	
	
	LinearFetch(uFX, vFX, nextMip, colorB);

	Ceng::UINT32 factor = Ceng::UINT32(65536.0f*fracPart);

	for (Ceng::UINT32 k = 0; k < 4; ++k)
	{
		Ceng::UINT32 reverse = 65535 - factor;

		out_color[4 * k + 0] = (reverse*colorA[4 * k + 0] + factor*colorB[4 * k + 0]) >> 16;
		out_color[4 * k + 1] = (reverse*colorA[4 * k + 1] + factor*colorB[4 * k + 1]) >> 16;
		out_color[4 * k + 2] = (reverse*colorA[4 * k + 2] + factor*colorB[4 * k + 2]) >> 16;
		out_color[4 * k + 3] = (reverse*colorA[4 * k + 3] + factor*colorB[4 * k + 3]) >> 16;
	}
	*/
}

_declspec(align(16)) const Ceng::INT16 fx16half[] = { -1, -1 };

struct AddressScale
{
	Ceng::UINT16 bytesPerPixel;
	Ceng::UINT16 rowBytes;
};

// NOTE: Only works in 32-bit mode.
inline __m128i GenerateAddress(const Ceng::POINTER baseAddress, const AddressScale &scale, const __m128i &uvVec)
{
	// baseAddress = dword {base,base,base,base}
	__m128 baseVec = _mm_load1_ps((float*)&baseAddress);

	// stepVec = word {rowBytes,bytesPerpixel,rowBytes,bytesPerPixel,...}
	__m128 stepVec = _mm_load1_ps((float*)&scale);

	// addressVec = dword{frac(v3)*height*rowBytes + frac(u3)*width*bytesPerPixel,...}
	__m128i addressVec = _mm_madd_epi16(uvVec, *(__m128i*)&stepVec);

	// addressVec = dword{base + frac(v3)*height*rowBytes + frac(u3)*width*bytesPerPixel,...}
	return _mm_add_epi32(addressVec, *(__m128i*)&baseVec);
}


/*
// Scanlined version
void CR_ShaderViewTex2D::Nearest_SSE2(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel, 
	const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color)
{
	// Only works for textures with rowBytes < 65536. 
	// This means a maximum width of 4096 for a float32 argb texture.

	Ceng::UINT16 dimScale[2] = { textures[mipLevel]->bufferWidth, textures[mipLevel]->bufferHeight };

	// Scale uv by texture dimension and convert to integer

	// uVecFX = dword {u3,u2,u1,u0}
	__m128i uVecFX = _mm_load_si128((__m128i*)uFX);

	// vVecFX = dword {v3,v2,v1,0}
	__m128i vVecFX = _mm_load_si128((__m128i*)vFX);

	// Extract fractional part of input uv

	__m128i uFloor = _mm_srli_epi32(uVecFX, 16);

	// uWrapVec = word { 0,frac(u3),0,frac(u2),0,frac(u1),0,frac(u0) }
	__m128i uWrapVec = _mm_sub_epi32(uVecFX, uFloor);
	//uWrapVec = _mm_srli_epi32(uWrapVec, 16);

	// vWrapVec = word { frac(v3),0,frac(v2),0,frac(v1),0,frac(v0),0 }

	__m128i vFloor = _mm_srli_epi32(vVecFX, 16);
	__m128i vWrapVec = _mm_sub_epi32(vVecFX, vFloor);
	vWrapVec = _mm_slli_epi32(vWrapVec, 16);

	// uvVec = word { frac(v3),frac(u3),frac(v2),frac(u2),frac(v1),frac(u1),frac(v0),frac(u0) }
	__m128i uvVec = _mm_or_si128(uWrapVec, vWrapVec);

	// Scale by texture dimensions

	// dimVec = word { height,width,height,width,height,width,height,width }
	__m128 dimVec = _mm_load1_ps((float*)&dimScale);

	// uvVec = word {frac(v3)*height,frac(u3)*width,...}
	uvVec = _mm_mulhi_epu16(uvVec, *(__m128i*)&dimVec);
	//////////////////////////////
	// Calculate texel load address vector

	_declspec(align(16)) AddressScale addressScale;

	addressScale.bytesPerPixel = textures[mipLevel]->bytesPerPixel;
	addressScale.rowBytes = textures[mipLevel]->tileYstep;

	__m128i addressVec = GenerateAddress(textures[mipLevel]->baseAddress, addressScale, uvVec);

	_declspec(align(16)) Ceng::UINT32 *address[4];

	_mm_store_si128((__m128i*)address,addressVec);

	// First texel

	__m128i colorVec = _mm_cvtsi32_si128(*address[0]);

	// Second texel

	__m128i loadVec = _mm_cvtsi32_si128(*address[1]);

	loadVec = _mm_slli_epi64(loadVec, 32);

	colorVec = _mm_or_si128(colorVec, loadVec);

	// Third texel

	loadVec = _mm_cvtsi32_si128(*address[2]);

	loadVec = _mm_slli_si128(loadVec, 8);

	colorVec = _mm_or_si128(colorVec, loadVec);

	// Fourth texel
	loadVec = _mm_cvtsi32_si128(*address[3]);

	loadVec = _mm_slli_si128(loadVec, 12);
	
	// colorVec = { {?,r3,g3,b3} , {?,r2,g2,b2} , {?,r1,g1,b1} , {?,r0,g0,b0} }
	colorVec = _mm_or_si128(colorVec, loadVec);

	// Dest is ubyte4

	// tempColor = { {0,0,0,0} , {0,0,0,0} , {a3,r3,g3,b3} , {a2,r2,g2,b2} }
	__m128i tempColor = _mm_srli_si128(colorVec, 8);

	// tempColor = { {a3,a1,r3,r1} , {g3,g1,b3,b1} , {a2,a0,r2,r0} , {g2,g0,b2,b0} }
	tempColor = _mm_unpacklo_epi8(colorVec, tempColor);

	// writeColor = { {0,0,0,0} , {0,0,0,0} , {a3,a1,r3,r1} , {g3,g1,b3,b1} }
	__m128i writeColor = _mm_srli_si128(tempColor, 8);

	// writeColor = byte {a3,a2,a1,a0,r3,r2,r1,r0,g3,g2,g1,g0,b3,b2,b1,b0}
	writeColor = _mm_unpacklo_epi8(tempColor, writeColor);

	_mm_store_ps(&out_color[0], *(__m128*)&writeColor);

	return;

	//////////////////////////////////////////////////////////////////////////////
	// Convert to vertical layout and then to floating point

	// blue = dword {b3,b2,b1,b0}
	__m128i blue = _mm_slli_epi32(colorVec, 24);
	blue = _mm_srli_epi32(blue, 24);

	// green = dword {g3,g2,g1,g0}
	__m128i green = _mm_slli_epi32(colorVec, 16);
	green = _mm_srli_epi32(green, 24);

	// green = dword {r3,r2,r1,r0}
	__m128i red = _mm_slli_epi32(colorVec, 8);
	red = _mm_srli_epi32(red, 24);	 

	__m128 blueF = _mm_cvtepi32_ps(blue);
	__m128 greenF = _mm_cvtepi32_ps(green);
	__m128 redF = _mm_cvtepi32_ps(red);

	__m128 colorScaleVec = _mm_load1_ps(&colorScale);

	blueF = _mm_mul_ps(blueF,colorScaleVec);
	greenF = _mm_mul_ps(greenF, colorScaleVec);
	redF = _mm_mul_ps(redF, colorScaleVec);

	// Write default alpha

	const Ceng::FLOAT32 alpha = 1.0f;

	__m128 alphaVec = _mm_load1_ps(&alpha);

	_mm_store_ps(&out_color[0], blueF);
	_mm_store_ps(&out_color[4], greenF);
	_mm_store_ps(&out_color[8], redF);
	_mm_store_ps(&out_color[12], alphaVec);
}
*/

struct TiledAddress
{
	Ceng::INT16 tileStepX;
	Ceng::INT16 tileStepY;

	Ceng::INT16 posStepX;
	Ceng::INT16 posStepY;
};


// NOTE: 
// - Only works in 32-bit mode.
// - Assumes 4x4 tiling    

inline __m128i GenerateTiledAddress(const Ceng::POINTER baseAddress, const TiledAddress &scale, const __m128i &uvVec)
{
	// stepVec = word {rowBytes,bytesPerpixel,rowBytes,bytesPerPixel,...}
	__m128 tileStepVec = _mm_load1_ps((float*)&scale.tileStepX);

	__m128i tileIndex = _mm_srli_epi16(uvVec, 2);

	__m128i tileOffset = _mm_madd_epi16(tileIndex, *(__m128i*)&tileStepVec);

	// baseAddress = dword {base,base,base,base}
	__m128 baseVec = _mm_load1_ps((float*)&baseAddress);

	__m128i posIndex = _mm_slli_epi16(uvVec, 14);

	__m128i posTemp = _mm_srli_epi16(posIndex, 14);

	posIndex = _mm_or_si128(posIndex, posTemp);

	posIndex = _mm_slli_epi32(posIndex, 2);
	posIndex = _mm_srli_epi32(posIndex, 14);

	tileOffset = _mm_add_epi32(tileOffset, posIndex);

	// addressVec = dword{base + frac(v3)*height*rowBytes + frac(u3)*width*bytesPerPixel,...}
	return _mm_add_epi32(tileOffset, *(__m128i*)&baseVec);
}

void CR_ShaderViewTex2D::Nearest_SSE2(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel,
	const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color)
{
	// Only works for textures with rowBytes < 65536. 
	// This means a maximum width of 4096 for a float32 argb texture.

	Ceng::UINT16 dimScale[2] = { textures[mipLevel]->bufferWidth, textures[mipLevel]->bufferHeight };

	// Scale uv by texture dimension and convert to integer

	// uVecFX = dword {u3,u2,u1,u0}
	__m128i uVecFX = _mm_load_si128((__m128i*)uFX);

	// vVecFX = dword {v3,v2,v1,0}
	__m128i vVecFX = _mm_load_si128((__m128i*)vFX);

	// Extract fractional part of input uv

	__m128i uFloor = _mm_srli_epi32(uVecFX, 16);

	// uWrapVec = word { 0,frac(u3),0,frac(u2),0,frac(u1),0,frac(u0) }
	__m128i uWrapVec = _mm_sub_epi32(uVecFX, uFloor);
	//uWrapVec = _mm_srli_epi32(uWrapVec, 16);

	// vWrapVec = word { frac(v3),0,frac(v2),0,frac(v1),0,frac(v0),0 }

	__m128i vFloor = _mm_srli_epi32(vVecFX, 16);
	__m128i vWrapVec = _mm_sub_epi32(vVecFX, vFloor);
	vWrapVec = _mm_slli_epi32(vWrapVec, 16);

	// uvVec = word { frac(v3),frac(u3),frac(v2),frac(u2),frac(v1),frac(u1),frac(v0),frac(u0) }
	__m128i uvVec = _mm_or_si128(uWrapVec, vWrapVec);

	// Scale by texture dimensions

	// dimVec = word { height,width,height,width,height,width,height,width }
	__m128 dimVec = _mm_load1_ps((float*)&dimScale);

	// uvVec = word {frac(v3)*height,frac(u3)*width,...}
	uvVec = _mm_mulhi_epu16(uvVec, *(__m128i*)&dimVec);
	//////////////////////////////
	// Calculate texel load address vector

	_declspec(align(16)) TiledAddress addressScale;

	addressScale.tileStepX = textures[mipLevel]->tileXstep;
	addressScale.tileStepY = textures[mipLevel]->tileYstep;
	addressScale.posStepX = textures[mipLevel]->channels[0].unitXstep;
	addressScale.posStepY = textures[mipLevel]->channels[0].unitYstep;

	__m128i addressVec = GenerateTiledAddress(textures[mipLevel]->baseAddress, addressScale, uvVec);

	_declspec(align(16)) Ceng::UINT32 *address[4];

	_mm_store_si128((__m128i*)address, addressVec);

	// First texel

	__m128i colorVec = _mm_cvtsi32_si128(*address[0]);

	// Second texel

	__m128i loadVec = _mm_cvtsi32_si128(*address[1]);

	loadVec = _mm_slli_epi64(loadVec, 32);

	colorVec = _mm_or_si128(colorVec, loadVec);

	// Third texel

	loadVec = _mm_cvtsi32_si128(*address[2]);

	loadVec = _mm_slli_si128(loadVec, 8);

	colorVec = _mm_or_si128(colorVec, loadVec);

	// Fourth texel
	loadVec = _mm_cvtsi32_si128(*address[3]);

	loadVec = _mm_slli_si128(loadVec, 12);

	// colorVec = { {?,r3,g3,b3} , {?,r2,g2,b2} , {?,r1,g1,b1} , {?,r0,g0,b0} }
	colorVec = _mm_or_si128(colorVec, loadVec);

	// Dest is ubyte4

	// tempColor = { {0,0,0,0} , {0,0,0,0} , {a3,r3,g3,b3} , {a2,r2,g2,b2} }
	__m128i tempColor = _mm_srli_si128(colorVec, 8);

	// tempColor = { {a3,a1,r3,r1} , {g3,g1,b3,b1} , {a2,a0,r2,r0} , {g2,g0,b2,b0} }
	tempColor = _mm_unpacklo_epi8(colorVec, tempColor);

	// writeColor = { {0,0,0,0} , {0,0,0,0} , {a3,a1,r3,r1} , {g3,g1,b3,b1} }
	__m128i writeColor = _mm_srli_si128(tempColor, 8);

	// writeColor = byte {a3,a2,a1,a0,r3,r2,r1,r0,g3,g2,g1,g0,b3,b2,b1,b0}
	writeColor = _mm_unpacklo_epi8(tempColor, writeColor);

	_mm_store_ps(&out_color[0], *(__m128*)&writeColor);

	return;

	//////////////////////////////////////////////////////////////////////////////
	// Convert to vertical layout and then to floating point

	// blue = dword {b3,b2,b1,b0}
	__m128i blue = _mm_slli_epi32(colorVec, 24);
	blue = _mm_srli_epi32(blue, 24);

	// green = dword {g3,g2,g1,g0}
	__m128i green = _mm_slli_epi32(colorVec, 16);
	green = _mm_srli_epi32(green, 24);

	// green = dword {r3,r2,r1,r0}
	__m128i red = _mm_slli_epi32(colorVec, 8);
	red = _mm_srli_epi32(red, 24);

	__m128 blueF = _mm_cvtepi32_ps(blue);
	__m128 greenF = _mm_cvtepi32_ps(green);
	__m128 redF = _mm_cvtepi32_ps(red);

	__m128 colorScaleVec = _mm_load1_ps(&colorScale);

	blueF = _mm_mul_ps(blueF, colorScaleVec);
	greenF = _mm_mul_ps(greenF, colorScaleVec);
	redF = _mm_mul_ps(redF, colorScaleVec);

	// Write default alpha

	const Ceng::FLOAT32 alpha = 1.0f;

	__m128 alphaVec = _mm_load1_ps(&alpha);

	_mm_store_ps(&out_color[0], blueF);
	_mm_store_ps(&out_color[4], greenF);
	_mm_store_ps(&out_color[8], redF);
	_mm_store_ps(&out_color[12], alphaVec);
}

inline __m128i MinWrapCoordinate(const __m128i &coordinate, const Ceng::UINT32 wrapValue)
{
	__m128i zeroComp = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi16(zeroComp, coordinate);

	__m128 wrapVec = _mm_load1_ps((float*)&wrapValue);

	mask = _mm_and_si128(mask, *(__m128i*)&wrapVec);

	return _mm_add_epi16(coordinate, mask);
}

// Converts x == wrapValue to zero
inline __m128i MaxWrapCoordinate(const __m128i &coordinate, const Ceng::UINT32 wrapValue)
{
	__m128 wrapVec = _mm_load1_ps((float*)&wrapValue);

	__m128i mask = _mm_cmpgt_epi16(*(__m128i*)&wrapVec,coordinate);
	
	// Subtract if x >= wrapValue
	__m128i delta = _mm_andnot_si128(mask, *(__m128i*)&wrapVec);

	return _mm_sub_epi16(coordinate, delta);
}

inline __m128i TexelAddressStep(const __m128i &coordinate, const __m128i &coordinateMax,
	const  __m128i &normalStep, const __m128i &wrappingStep)
{
	// Need a dword granularity mask here
	__m128i selector = _mm_cmpeq_epi32(coordinate, coordinateMax);

	/*
	__m128i greater = _mm_cmpgt_epi32(coordinate, coordinateMax);
	selector = _mm_or_si128(selector, greater);
	*/

	__m128i stepA = _mm_andnot_si128(selector, normalStep);
	__m128i stepB = _mm_and_si128(selector, wrappingStep);

	return _mm_add_epi32(stepA, stepB);
}

/*
__forceinline __m128i LinearFetch(Ceng::UINT32 *fetchPtr[4],  __m128i &allZeroes,
	 __m128i &weightVec)
{
	__m128i colorSum = _mm_setzero_si128();

	// color = byte { {0,0,0,0} , {0,0,0,0} ,{0,0,0,0} , {a0,r0,g0,b0} }
	__m128i color = _mm_cvtsi32_si128(*fetchPtr[0]);

	// reader = byte { {0,0,0,0} , {0,0,0,0} ,{0,0,0,0} , {a1,r1,g1,b1} }
	__m128i reader = _mm_cvtsi32_si128(*fetchPtr[1]);

	// reader = byte { {0,0,0,0} , {0,0,0,0} , {a1,r1,g1,b1} ,{0,0,0,0} }
	reader = _mm_slli_si128(reader, 4);

	// color = byte { {0,0,0,0} , {0,0,0,0} ,{a1,r1,g1,b1} , {a0,r0,g0,b0} }
	color = _mm_or_si128(color, reader);

	// color = word { a1,r1,g1,b1,a0,r0,g0,b0 }
	color = _mm_unpacklo_epi8(color, allZeroes);

	// Prescale for weight multiplication
	color = _mm_slli_epi16(color, 7);

	// weight = word { w1,w1,w1,w1,w0,w0,w0,w0}
	__m128i weight = _mm_shuffle_epi32(weightVec, _MM_SHUFFLE(1, 1, 0, 0));

	// Multiply by factor
	color = _mm_mulhi_epu16(color, *(__m128i*)&weight);

	// Accumulate color in 15.1 format
	colorSum = _mm_adds_epu16(colorSum, color);


	// color = byte { {0,0,0,0} , {0,0,0,0} ,{0,0,0,0} , {a2,r2,g2,b2} }
	color = _mm_cvtsi32_si128(*fetchPtr[2]);

	// reader = byte { {0,0,0,0} , {0,0,0,0} ,{0,0,0,0} , {a3,r3,g3,b3} }
	reader = _mm_cvtsi32_si128(*fetchPtr[3]);

	// reader = byte { {0,0,0,0} , {0,0,0,0} , {a3,r3,g3,b3} ,{0,0,0,0} }
	reader = _mm_slli_si128(reader, 4);

	// color = byte { {0,0,0,0} , {0,0,0,0} ,{a3,r3,g3,b3} , {a2,r2,g2,b2} }
	color = _mm_or_si128(color, reader);

	// color = word { a3,r3,g3,b3,a2,r2,g2,b2 }
	color = _mm_unpacklo_epi8(color, allZeroes);

	// Prescale for weight multiplication
	color = _mm_slli_epi16(color, 7);

	// weight = word { w3,w3,w3,w3,w2,w2,w2,w2}
	weight = _mm_shuffle_epi32(weightVec, _MM_SHUFFLE(3, 3, 2, 2));

	// Multiply by factor
	color = _mm_mulhi_epu16(color, *(__m128i*)&weight);

	// Accumulate color in 15.1 format
	// colorSum = { {a3+a1},{r3+r1},{g3+g1},{b3+b1},{a2+a0},{r2+r0},{g2+g0},{b2+b0} }
	colorSum = _mm_adds_epu16(colorSum, color);

	// sumHigh = { 0,0,0,0, {a3+a1},{r3+r1},{g3+g1},{b3+b1} }
	__m128i sumHigh = _mm_srli_si128(colorSum, 8);

	// colorSum = {x,x,x,x,{a3+a2+a1+a0},{r3+r2+r1+r0},{g3+g2+g1+g0},{b3+b2+b1+b0} }

	colorSum = _mm_adds_epu16(colorSum, sumHigh);

	return _mm_move_epi64(colorSum);
}
*/


__forceinline __m128i LinearFetch(Ceng::UINT32 *fetchPtr[4], __m128i &allZeroes,
	__m128i &weightVec)
{
	__m128i colorSum = _mm_setzero_si128();

	// color = byte { {0,0,0,0} , {0,0,0,0} ,{0,0,0,0} , {a0,r0,g0,b0} }
	__m128i color = _mm_insert_epi32(colorSum, *fetchPtr[0], 0);

	// reader = byte { {0,0,0,0} , {0,0,0,0} ,{0,0,0,0} , {a1,r1,g1,b1} }
	color = _mm_insert_epi32(color, *fetchPtr[1], 1);

	// color = word { a1,r1,g1,b1,a0,r0,g0,b0 }
	color = _mm_unpacklo_epi8(color, allZeroes);

	// Prescale for weight multiplication
	color = _mm_slli_epi16(color, 7);

	// weight = word { w1,w1,w1,w1,w0,w0,w0,w0}
	__m128i weight = _mm_shuffle_epi32(weightVec, _MM_SHUFFLE(1, 1, 0, 0));

	// Multiply by factor
	color = _mm_mulhi_epu16(color, *(__m128i*)&weight);

	// Accumulate color in 15.1 format
	colorSum = _mm_adds_epu16(colorSum, color);

	// color = byte { {0,0,0,0} , {0,0,0,0} ,{0,0,0,0} , {a2,r2,g2,b2} }
	color = _mm_insert_epi32(color, *fetchPtr[2], 0);

	// reader = byte { {0,0,0,0} , {0,0,0,0} ,{0,0,0,0} , {a3,r3,g3,b3} }
	color = _mm_insert_epi32(color, *fetchPtr[3], 1);

	// color = word { a3,r3,g3,b3,a2,r2,g2,b2 }
	color = _mm_unpacklo_epi8(color, allZeroes);

	// Prescale for weight multiplication
	color = _mm_slli_epi16(color, 7);

	// weight = word { w3,w3,w3,w3,w2,w2,w2,w2}
	weight = _mm_shuffle_epi32(weightVec, _MM_SHUFFLE(3, 3, 2, 2));

	// Multiply by factor
	color = _mm_mulhi_epu16(color, *(__m128i*)&weight);

	// Accumulate color in 15.1 format
	// colorSum = { {a3+a1},{r3+r1},{g3+g1},{b3+b1},{a2+a0},{r2+r0},{g2+g0},{b2+b0} }
	colorSum = _mm_adds_epu16(colorSum, color);

	// sumHigh = { 0,0,0,0, {a3+a1},{r3+r1},{g3+g1},{b3+b1} }
	__m128i sumHigh = _mm_srli_si128(colorSum, 8);

	// colorSum = {x,x,x,x,{a3+a2+a1+a0},{r3+r2+r1+r0},{g3+g2+g1+g0},{b3+b2+b1+b0} }

	colorSum = _mm_adds_epu16(colorSum, sumHigh);

	return _mm_move_epi64(colorSum);
}

__forceinline void LinearFetchLow(__m128i &outColor,Ceng::UINT32 *fetchPtr[4],__m128i &allZeroes,
	 __m128i &weightVec)
{
	outColor = LinearFetch(fetchPtr,allZeroes,weightVec);
}

__forceinline void LinearFetchHigh(__m128i &outColor, Ceng::UINT32 *fetchPtr[4], __m128i &allZeroes,
	 __m128i &weightVec)
{
	__m128i temp = LinearFetch(fetchPtr, allZeroes, weightVec);

	temp = _mm_slli_si128(temp, 8);

	outColor = _mm_or_si128(outColor, temp);
}

// NOTE: Only works in 32-bit mode.
inline __m128i GenerateAddressNew(const __m128i &baseVec, const __m128i &scaleVec, const __m128i &uvVec,const __m128i &wrapVec)
{
	__m128i mask = _mm_cmpeq_epi16(uvVec, wrapVec);

	__m128i delta = _mm_and_si128(mask,wrapVec);

	__m128i uvFinal = _mm_sub_epi16(uvVec, delta);

	// addressVec = dword{frac(v3)*height*rowBytes + frac(u3)*width*bytesPerPixel,...}
	__m128i addressVec = _mm_madd_epi16(uvFinal,scaleVec);

	// addressVec = dword{base + frac(v3)*height*rowBytes + frac(u3)*width*bytesPerPixel,...}
	return _mm_add_epi32(addressVec,baseVec);
}

const Ceng::FLOAT32 defaultAlpha = 1.0f;

void CR_ShaderViewTex2D::Linear_SSE2(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel,
	const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color)
{

	// Only works for textures with rowBytes < 32768. 
	// This means a maximum width of 4096 for a float32 argb texture.

	// Width and height are scaled by 2 to produce correct result during fused multiply-add
	_declspec(align(8)) Ceng::UINT16 dimScale[4] =
	{ 1, textures[mipLevel]->bufferWidth << 1, 1, textures[mipLevel]->bufferHeight << 1 };

	Ceng::INT16 truncate[2] = { -32768, 0 };

	// uVecFX = dword {u3,u2,u1,u0}
	__m128i uVecFX = _mm_load_si128((__m128i*)uFX);

	// vVecFX = dword {v3,v2,v1,0}
	__m128i vVecFX = _mm_load_si128((__m128i*)vFX);

	////////////////////////////////////////////////////////////////////////////////////
	// Get fractional part of input uv

	__m128i uFloor = _mm_srli_epi32(uVecFX, 16);
	uFloor = _mm_slli_epi32(uVecFX, 16);

	// uFrac = u - floor(u)
	__m128i uWrapVec = _mm_sub_epi32(uVecFX, uFloor);

	// Convert to 0.15 fixed point for multiplication
	uWrapVec = _mm_slli_epi32(uWrapVec, 16);
	uWrapVec = _mm_srli_epi16(uWrapVec, 1);

	__m128i vFloor = _mm_srli_epi32(vVecFX, 16);
	vFloor = _mm_slli_epi32(vVecFX, 16);

	__m128i vWrapVec = _mm_sub_epi32(vVecFX, vFloor);

	// Convert to 0.15 fixed point for multiplication
	vWrapVec = _mm_slli_epi32(vWrapVec, 16);
	vWrapVec = _mm_srli_epi16(vWrapVec, 1);

	__m128 negTerm = _mm_load1_ps((float*)&truncate);

	// uWrapVec = word { {uFrac[3],-32768} , {uFrac[2],-32768} ,{uFrac[1],-32768},{uFrac[0],-32768}}
	uWrapVec = _mm_or_si128(uWrapVec, *(__m128i*)&negTerm);

	// vWrapVec = word { {vFrac[3],-32768} , {vFrac[2],-32768} ,{vFrac[1],-32768},{vFrac[0],-32768}}
	vWrapVec = _mm_or_si128(vWrapVec, *(__m128i*)&negTerm);

	__m128i dimLoad = _mm_loadl_epi64((__m128i*)dimScale);

	__m128i widthMulVec = _mm_shuffle_epi32(dimLoad, _MM_SHUFFLE(0, 0, 0, 0));
	__m128i heightMulVec = _mm_shuffle_epi32(dimLoad, _MM_SHUFFLE(1, 1, 1, 1));

	// uVec = {uFrac[3]*width-32768,uFrac[2]*width-32768,...}
	__m128i uVec = _mm_madd_epi16(uWrapVec, widthMulVec);

	// vVec = {vFrac[3]*width-32768,vFrac[2]*width-32768,...}
	__m128i vVec = _mm_madd_epi16(vWrapVec, heightMulVec);

	///////////////////////////////////////////////////////////////////////////////////
	// Final uv

	// uIntVec = { {0,uInt[3]} , {0,uInt[2]} , {0,uInt[1]} , {0,uInt[0]}}
	__m128i uIntVec = _mm_srli_epi32(uVec, 16);

	// uFraction = { {uFrac[3],uFrac[3]} , {uFrac[2],uFrac[2]},...}
	__m128i uFraction = _mm_slli_epi32(uVec, 16);
	uFraction = _mm_or_si128(uFraction, _mm_srli_epi32(uFraction, 16));

	// vIntVec = { {0,vInt[3]} , {0,vInt[2]} , {0,vInt[1]} , {0,vInt[0]}}
	__m128i vIntVec = _mm_srli_epi32(vVec, 16);

	// vFraction = { {vFrac[3],vFrac[3]} , {vFrac[2],vFrac[2]} , ...}
	__m128i vFraction = _mm_slli_epi32(vVec, 16);
	vFraction = _mm_or_si128(vFraction, _mm_srli_epi32(vFraction, 16));

	// factor_bottomRight = word { {uFrac[3]*vFrac[3],uFrac[3]*vFrac[3]},...}
	__m128i factor_bottomRight = _mm_mulhi_epu16(uFraction, vFraction);

	// factor_topLeft = word { 65535+uFrac[3]*vFrac[3]-uFrac[3]-vFrac[3],...}

	// NOTE: Apply terms in this order to avoid overflow from 65535+uFrac*vFrac.
	__m128i factor_topLeft = _mm_setzero_si128(); // Zero is equivalent to 65536 mod 65536

	factor_topLeft = _mm_sub_epi16(factor_topLeft, uFraction);
	factor_topLeft = _mm_sub_epi16(factor_topLeft, vFraction);
	factor_topLeft = _mm_add_epi16(factor_topLeft, factor_bottomRight);

	// factor_topRight = word {uFrac[3]-uFrac[3]*vFrac[3],...}
	__m128i factor_topRight = _mm_sub_epi16(uFraction, factor_bottomRight);

	// factor_bottomLeft = word {vFrac[3]-uFrac[3]*vFrac[3],...}
	__m128i factor_bottomLeft = _mm_sub_epi16(vFraction, factor_bottomRight);

	// Convert factors from SOA to AOS

	__m128i muxA = _mm_unpacklo_epi32(factor_topLeft, factor_topRight);
	__m128i muxB = _mm_unpacklo_epi32(factor_bottomLeft, factor_bottomRight);

	__m128i muxC = _mm_unpackhi_epi32(factor_topLeft, factor_topRight);
	__m128i muxD = _mm_unpackhi_epi32(factor_bottomLeft, factor_bottomRight);

	factor_topLeft = _mm_unpacklo_epi64(muxA, muxB);
	factor_topRight = _mm_unpackhi_epi64(muxA, muxB);

	factor_bottomLeft = _mm_unpacklo_epi64(muxC, muxD);
	factor_bottomRight = _mm_unpackhi_epi64(muxC, muxD);

	/////////////////////////////////////////////////////////////////////////////////
	// Texel address mode = wrap

	uIntVec = MinWrapCoordinate(uIntVec, textures[mipLevel]->bufferWidth);
	vIntVec = MinWrapCoordinate(vIntVec, textures[mipLevel]->bufferHeight);

	//////////////////////////////////////////////////////////////////////////////////
	// Combined uv-vector

	vIntVec = _mm_slli_epi32(vIntVec, 16);

	// uvVec = { {vInt[3],uInt[3]} , {vInt[2],uInt[2]} , ... }
	__m128i uvVec = _mm_or_si128(uIntVec, vIntVec);

	vIntVec = _mm_srli_epi32(vIntVec, 16);

	__m128i allZeroes = _mm_setzero_si128();

	////////////////////////////////////////////////////
	// Generate top-left texel address

	_declspec(align(16)) TiledAddress addressScale;

	addressScale.tileStepX = textures[mipLevel]->tileXstep;
	addressScale.tileStepY = textures[mipLevel]->tileYstep;
	addressScale.posStepX = textures[mipLevel]->channels[0].unitXstep;
	addressScale.posStepY = textures[mipLevel]->channels[0].unitYstep;

	__m128i topLeftAdr = GenerateTiledAddress(textures[mipLevel]->baseAddress, addressScale, uvVec);

	////////////////////////////////////////////////////
	// Generate top-right texel address

	// allOnes = dword {1,1,1,1}
	__m128i allOnes = _mm_srli_epi32(*(__m128i*)&negTerm, 15);

	__m128i steppedU = _mm_add_epi16(uIntVec, allOnes);

	/*
	Ceng::INT32 defStepU = 4;

	// Additional step if tile wraps right
	// NOTE: bytes per pixel * (1+n(n-1))
	Ceng::INT32 tileWrapU = (1 + 4 * (4 - 1)) * 4;

	// Additional step if entire texture wraps
	// NOTE: bytes per pixel * -n*width
	Ceng::INT32 texWrapU = 4 * (-4 * textures[mipLevel]->bufferWidth);

	// Adjust both so that all steps can be added independently
	tileWrapU -= defStepU;

	// Start with default step in u-direction
	__m128i stepRight = *(__m128i*)&_mm_load1_ps((float*)&defStepU);

	// Check if the entire texture wraps
	__m128i texWrapStepU = *(__m128i*)&_mm_load1_ps((float*)&texWrapU);

	__m128i uMax = *(__m128i*)&_mm_load1_ps((float*)&textures[mipLevel]->bufferWidth);

	__m128i texWrapTest = _mm_cmpgt_epi32(uMax, steppedU);

	texWrapTest = _mm_andnot_si128(texWrapTest, texWrapStepU);

	stepRight = _mm_add_epi32(stepRight, texWrapTest);

	// Check if tile wrapped
	__m128i tileWrapTest = _mm_slli_epi16(steppedU, 14);
	tileWrapTest = _mm_slli_epi16(steppedU, 14);

	tileWrapTest = _mm_cmpeq_epi32(tileWrapTest, allZeroes);

	__m128i tileWrapStep = *(__m128i*)&_mm_load1_ps((float*)&tileWrapU);

	tileWrapTest = _mm_and_si128(tileWrapTest, tileWrapStep);

	stepRight = _mm_add_epi32(stepRight, tileWrapTest);

	__m128i topRightAdr = _mm_add_epi32(topLeftAdr, stepRight);
	*/

	steppedU = MaxWrapCoordinate(steppedU, textures[mipLevel]->bufferWidth);
	
	__m128i uvTemp = _mm_slli_epi32(vIntVec, 16);

	uvTemp = _mm_or_si128(uvTemp, steppedU);

	__m128i topRightAdr = GenerateTiledAddress(textures[mipLevel]->baseAddress, addressScale, uvTemp);
	

	////////////////////////////////////////////////////
	// Generate bottom-left texel address

	__m128i steppedV = _mm_add_epi16(vIntVec, allOnes);

	/*
	Ceng::INT32 defStepV = addressScale.posStepY;

	// Additional step if tile wraps right
	// NOTE: bytes per pixel * (1+n(n-1))
	Ceng::INT32 tileWrapV = 4 * (4 * textures[mipLevel]->bufferWidth - 4 * (4 - 1));

	// Additional step if entire texture wraps
	// NOTE: bytes per pixel * -n*width
	Ceng::INT32 texWrapV = -addressScale.tileStepY*(textures[mipLevel]->bufferHeight>>2);

	// Adjust both so that all steps can be added independently
	tileWrapV -= defStepV;

	// Start with default step in u-direction
	__m128i stepDown = *(__m128i*)&_mm_load1_ps((float*)&defStepV);

	// Check if the entire texture wraps
	__m128i texWrapStepV = *(__m128i*)&_mm_load1_ps((float*)&texWrapV);

	__m128i vMax = *(__m128i*)&_mm_load1_ps((float*)&textures[mipLevel]->bufferHeight);

	texWrapTest = _mm_cmpgt_epi32(vMax, steppedV);

	texWrapTest = _mm_andnot_si128(texWrapTest, texWrapStepV);

	stepDown = _mm_add_epi32(stepDown, texWrapTest);

	// Check if tile wrapped
	tileWrapTest = _mm_slli_epi16(steppedV, 14);
	tileWrapTest = _mm_slli_epi16(steppedV, 14);

	tileWrapTest = _mm_cmpeq_epi32(tileWrapTest, allZeroes);

	tileWrapStep = *(__m128i*)&_mm_load1_ps((float*)&tileWrapV);

	tileWrapTest = _mm_and_si128(tileWrapTest, tileWrapStep);

	stepDown = _mm_add_epi32(stepDown, tileWrapTest);

	__m128i bottomLeftAdr = _mm_add_epi32(topLeftAdr, stepDown);
	*/
	

	steppedV = MaxWrapCoordinate(steppedV, textures[mipLevel]->bufferHeight);

	uvTemp = _mm_slli_epi32(steppedV, 16);

	uvTemp = _mm_or_si128(uvTemp,uIntVec);

	__m128i bottomLeftAdr = GenerateTiledAddress(textures[mipLevel]->baseAddress, addressScale, uvTemp);
	

	////////////////////////////////////////////////////
	// Generate bottom-right texel address

	
	__m128i adrStep = _mm_sub_epi32(topRightAdr,topLeftAdr);
	__m128i bottomRightAdr = _mm_add_epi32(bottomLeftAdr, adrStep);
	

	//__m128i bottomRightAdr = _mm_add_epi32(bottomLeftAdr, stepRight);
	
	//////////////////////////////////////////////////////////
	// Shuffle addresses so that all texel addresses for one screen pixel are
	// in a single vector

	__m128i fetchAddress[4];

	// addressA = dword {topRight[1],topLeft[1],topRight[0],topLeft[0]}
	__m128i addressA = _mm_unpacklo_epi32(topLeftAdr, topRightAdr);

	// addressB = dword {bottomRight[1],bottomLeft[1],bottomRight[0],bottomLeft[0]}
	__m128i addressB = _mm_unpacklo_epi32(bottomLeftAdr, bottomRightAdr);

	// fetchAddress[0] = dword {bottomRight[0],bottomLeft[0],topRight[0],topLeft[0] }
	fetchAddress[0] = _mm_unpacklo_epi64(addressA, addressB);

	// fetchAddress[1] = dword {bottomRight[1],bottomLeft[1],topRight[1],topLeft[1] }
	fetchAddress[1] = _mm_unpackhi_epi64(addressA, addressB);

	// addressA = dword {topRight[3],topLeft[3],topRight[2],topLeft[2]}
	addressA = _mm_unpackhi_epi32(topLeftAdr, topRightAdr);

	// addressB = dword {bottomRight[3],bottomLeft[3],bottomRight[2],bottomLeft[2]}
	addressB = _mm_unpackhi_epi32(bottomLeftAdr, bottomRightAdr);

	// fetchAddress[2] = dword {bottomRight[2],bottomLeft[2],topRight[2],topLeft[2] }
	fetchAddress[2] = _mm_unpacklo_epi64(addressA, addressB);

	// fetchAddress[3] = dword {bottomRight[3],bottomLeft[3],topRight[3],topLeft[3] }
	fetchAddress[3] = _mm_unpackhi_epi64(addressA, addressB);

	_declspec(align(16)) Ceng::UINT32 *fetchPtr[4][4];

	_mm_store_si128((__m128i*)&fetchPtr[0][0], fetchAddress[0]);
	_mm_store_si128((__m128i*)&fetchPtr[1][0], fetchAddress[1]);
	_mm_store_si128((__m128i*)&fetchPtr[2][0], fetchAddress[2]);
	_mm_store_si128((__m128i*)&fetchPtr[3][0], fetchAddress[3]);

	////////////////////////////////////////////////////////
	// Setup texel fetch

	//__m128i allZeroes = _mm_setzero_si128();

	__m128i finalColor[2] = { _mm_setzero_si128(), _mm_setzero_si128() };

	__m128i weight;
	__m128i color,reader;

	////////////////////////////////////////////////////////
	// Top-left fetch

	LinearFetchLow(finalColor[0],&fetchPtr[0][0], allZeroes, factor_topLeft);

	//////////////////////////////////////////////////////////////////
	// Top-right fetch

	LinearFetchHigh(finalColor[0], &fetchPtr[1][0], allZeroes, factor_topRight);

	//////////////////////////////////////////////////////////////////
	// Bottom-left fetch

	LinearFetchLow(finalColor[1], &fetchPtr[2][0], allZeroes, factor_bottomLeft);

	//////////////////////////////////////////////////////////////////
	// Bottom-right fetch

	LinearFetchHigh(finalColor[1], &fetchPtr[3][0], allZeroes, factor_bottomRight);

	//////////////////////////////////////////////////////////////////
	// Truncate color to integer

	finalColor[0] = _mm_srli_epi16(finalColor[0], 7);
	finalColor[1] = _mm_srli_epi16(finalColor[1], 7);

	//////////////////////////////////////////////////////////////////
	// Prefetch output address

	_mm_prefetch((char*)out_color, 1);

	//////////////////////////////////////////////////////////////////
	// Convert to vertical format

	// Dest is ubyte4

	// finalColor[0] = byte{0,a1,0,r1,0,g1,0,b1,0,a0,0,r0,0,g0,0,b0}
	// finalColor[1] = byte{0,a3,0,r3,0,g3,0,b3,0,a2,0,r2,0,g2,0,b2}

	__m128i colorTemp = _mm_slli_epi16(finalColor[1], 8);
	colorTemp = _mm_or_si128(colorTemp, finalColor[0]);

	// colorTemp = byte{a3,a1,r3,r1,g3,g1,b3,b1,a2,a0,r2,r0,g2,g0,b2,b0}

	__m128i writeColor = _mm_srli_si128(colorTemp, 8);

	// writeColor = byte {a3,a2,a1,a0,r3,r2,r1,r0,g3,g2,g1,g0,b3,b2,b1,b0}
	writeColor = _mm_unpacklo_epi8(colorTemp, writeColor);

	_mm_store_ps(&out_color[0], *(__m128*)&writeColor);

	return;

	// Dest is float4

	// finalColor[0] = word{a1,r1,g1,b1,a0,r0,g0,b0}
	// finalColor[1] = word{a3,r3,g3,b3,a2,r2,g2,b2}

	// blueGreenLow = dword {g1,g0,b1,b0}
	__m128i soaTempA = _mm_unpacklo_epi16(finalColor[0],finalColor[1]);

	// redAlphaLow = dword {a1,a0,r1,r0}
	__m128i soaTempB = _mm_unpackhi_epi16(finalColor[0],finalColor[1]);

	// blueGreenHigh = dword {g3,g2,b3,b2}
	__m128i blueGreen = _mm_unpacklo_epi16(soaTempA,soaTempB);

	// redAlphaHigh = dword {a3,a2,r3,r2}
	__m128i redAlpha = _mm_unpackhi_epi16(soaTempA,soaTempB);

	__m128 colorScaleVec = _mm_load1_ps(&colorScale);

	__m128i blue = _mm_unpacklo_epi16(blueGreen, allZeroes);
	__m128i green = _mm_unpackhi_epi16(blueGreen, allZeroes);
	__m128i red = _mm_unpacklo_epi16(redAlpha, allZeroes);

	__m128 blueF = _mm_cvtepi32_ps(blue);
	__m128 greenF = _mm_cvtepi32_ps(green);
	__m128 redF = _mm_cvtepi32_ps(red);

	__m128 alphaVec = _mm_load1_ps(&defaultAlpha);

	blueF = _mm_mul_ps(blueF, colorScaleVec);
	greenF = _mm_mul_ps(greenF, colorScaleVec);
	redF = _mm_mul_ps(redF, colorScaleVec);

	// Write default alpha

	_mm_store_ps(&out_color[0], blueF);
	_mm_store_ps(&out_color[4], greenF);
	_mm_store_ps(&out_color[8], redF);
	_mm_store_ps(&out_color[12], alphaVec);
}
	

/*
// Old scanlined version
void CR_ShaderViewTex2D::Linear_SSE2(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel, 
	const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color)
{
	
	// Only works for textures with rowBytes < 32768. 
	// This means a maximum width of 4096 for a float32 argb texture.

	// Width and height are scaled by 2 to produce correct result during fused multiply-add
	_declspec(align(8)) Ceng::UINT16 dimScale[4] = 
	{ 1, textures[mipLevel]->bufferWidth << 1, 1, textures[mipLevel]->bufferHeight << 1 };
	
	Ceng::INT16 truncate[2] = { -32768, 0 };

	// uVecFX = dword {u3,u2,u1,u0}
	__m128i uVecFX = _mm_load_si128((__m128i*)uFX);

	// vVecFX = dword {v3,v2,v1,0}
	__m128i vVecFX = _mm_load_si128((__m128i*)vFX);

	////////////////////////////////////////////////////////////////////////////////////
	// Get fractional part of input uv

	__m128i uFloor = _mm_srli_epi32(uVecFX, 16);
	uFloor = _mm_slli_epi32(uVecFX, 16);

	// uFrac = u - floor(u)
	__m128i uWrapVec = _mm_sub_epi32(uVecFX, uFloor);

	// Convert to 0.15 fixed point for multiplication
	uWrapVec = _mm_slli_epi32(uWrapVec, 16);
	uWrapVec = _mm_srli_epi16(uWrapVec, 1);

	__m128i vFloor = _mm_srli_epi32(vVecFX, 16);
	vFloor = _mm_slli_epi32(vVecFX, 16);

	__m128i vWrapVec = _mm_sub_epi32(vVecFX, vFloor);

	// Convert to 0.15 fixed point for multiplication
	vWrapVec = _mm_slli_epi32(vWrapVec, 16);
	vWrapVec = _mm_srli_epi16(vWrapVec, 1);

	__m128 negTerm = _mm_load1_ps((float*)&truncate);

	// uWrapVec = word { {uFrac[3],-32768} , {uFrac[2],-32768} ,{uFrac[1],-32768},{uFrac[0],-32768}}
	uWrapVec = _mm_or_si128(uWrapVec, *(__m128i*)&negTerm);

	// vWrapVec = word { {vFrac[3],-32768} , {vFrac[2],-32768} ,{vFrac[1],-32768},{vFrac[0],-32768}}
	vWrapVec = _mm_or_si128(vWrapVec, *(__m128i*)&negTerm);

	__m128i dimLoad = _mm_loadl_epi64((__m128i*)dimScale);
	
	__m128i widthMulVec = _mm_shuffle_epi32(dimLoad, _MM_SHUFFLE(0,0,0,0));
	__m128i heightMulVec = _mm_shuffle_epi32(dimLoad, _MM_SHUFFLE(1, 1, 1, 1));

	// uVec = {uFrac[3]*width-32768,uFrac[2]*width-32768,...}
	__m128i uVec = _mm_madd_epi16(uWrapVec,widthMulVec);

	// vVec = {vFrac[3]*width-32768,vFrac[2]*width-32768,...}
	__m128i vVec = _mm_madd_epi16(vWrapVec,heightMulVec);

	///////////////////////////////////////////////////////////////////////////////////
	// Final uv

	// uIntVec = { {0,uInt[3]} , {0,uInt[2]} , {0,uInt[1]} , {0,uInt[0]}}
	__m128i uIntVec = _mm_srli_epi32(uVec, 16);

	// uFraction = { {uFrac[3],uFrac[3]} , {uFrac[2],uFrac[2]},...}
	__m128i uFraction = _mm_slli_epi32(uVec, 16);
	uFraction = _mm_or_si128(uFraction, _mm_srli_epi32(uFraction, 16));

	// vIntVec = { {0,vInt[3]} , {0,vInt[2]} , {0,vInt[1]} , {0,vInt[0]}}
	__m128i vIntVec = _mm_srli_epi32(vVec, 16);

	// vFraction = { {vFrac[3],vFrac[3]} , {vFrac[2],vFrac[2]} , ...}
	__m128i vFraction = _mm_slli_epi32(vVec, 16);
	vFraction = _mm_or_si128(vFraction, _mm_srli_epi32(vFraction, 16));

	// factor_bottomRight = word { {uFrac[3]*vFrac[3],uFrac[3]*vFrac[3]},...}
	__m128i factor_bottomRight = _mm_mulhi_epu16(uFraction, vFraction);

	// factor_topLeft = word { 65535+uFrac[3]*vFrac[3]-uFrac[3]-vFrac[3],...}
	__m128i factor_topLeft = _mm_cmpeq_epi16(uFraction, vFraction);

	// NOTE: Apply terms in this order to avoid overflow from 65535+uFrac*vFrac.
	factor_topLeft = _mm_sub_epi16(factor_topLeft, uFraction);
	factor_topLeft = _mm_sub_epi16(factor_topLeft, vFraction);
	factor_topLeft = _mm_add_epi16(factor_topLeft,factor_bottomRight);
	
	// factor_topRight = word {uFrac[3]-uFrac[3]*vFrac[3],...}
	__m128i factor_topRight = _mm_sub_epi16(uFraction, factor_bottomRight);

	// factor_bottomLeft = word {vFrac[3]-uFrac[3]*vFrac[3],...}
	__m128i factor_bottomLeft = _mm_sub_epi16(vFraction, factor_bottomRight);

	// Convert factors from SOA to AOS

	__m128i muxA = _mm_unpacklo_epi32(factor_topLeft, factor_topRight);
	__m128i muxB = _mm_unpacklo_epi32(factor_bottomLeft, factor_bottomRight);

	__m128i muxC = _mm_unpackhi_epi32(factor_topLeft, factor_topRight);
	__m128i muxD = _mm_unpackhi_epi32(factor_bottomLeft, factor_bottomRight);

	factor_topLeft = _mm_unpacklo_epi64(muxA, muxB);
	factor_topRight = _mm_unpackhi_epi64(muxA, muxB);

	factor_bottomLeft = _mm_unpacklo_epi64(muxC, muxD);
	factor_bottomRight = _mm_unpackhi_epi64(muxC, muxD);

	/////////////////////////////////////////////////////////////////////////////////
	// Texel address mode = wrap

	uIntVec = MinWrapCoordinate(uIntVec, textures[mipLevel]->bufferWidth);
	vIntVec = MinWrapCoordinate(vIntVec, textures[mipLevel]->bufferHeight);

	//////////////////////////////////////////////////////////////////////////////////
	// Combined uv-vector

	vIntVec = _mm_slli_epi32(vIntVec, 16);

	// uvVec = { {vInt[3],uInt[3]} , {vInt[2],uInt[2]} , ... }
	__m128i uvVec = _mm_or_si128(uIntVec, vIntVec);

	vIntVec = _mm_srli_epi32(vIntVec, 16);
	
	/*
	// New way 
	////////////////////////////////////////////////////
	// Generate top-left texel address

	const Ceng::UINT16 wrapVal[2] = { textures[mipLevel]->bufferWidth, textures[mipLevel]->bufferHeight };

	__m128i wrapValVec;
	__m128 *wrapValVecF = (__m128*)&wrapValVec;

	*wrapValVecF = _mm_load1_ps((float*)wrapVal);

	_declspec(align(16)) AddressScale addressScale;

	addressScale.bytesPerPixel = textures[mipLevel]->bytesPerPixel;
	addressScale.rowBytes = textures[mipLevel]->tileYstep;

	__m128i baseVec;
	__m128 *baseVecF = (__m128*)&baseVec;
	
	*baseVecF = _mm_load1_ps((float*)&textures[mipLevel]->baseAddress);

	__m128i scaleVec;
	__m128 *scaleVecF = (__m128*)&scaleVec;

	*scaleVecF = _mm_load1_ps((float*)&addressScale);

	__m128i topLeftAdr = GenerateAddressNew(baseVec, scaleVec, uvVec,wrapValVec);

	const Ceng::UINT16 stepTerm[2] = { 1, 0 };

	__m128i uvStepVec;
	__m128 *uvStepVecF = (__m128*)&uvStepVec;

	*uvStepVecF = _mm_load1_ps((float*)&stepTerm);

	////////////////////////////////////////////////////
	// Generate top-right texel address

	uvVec = _mm_add_epi16(uvVec, uvStepVec);

	__m128i topRightAdr = GenerateAddressNew(baseVec, scaleVec, uvVec, wrapValVec);
	////////////////////////////////////////////////////
	// Generate bottom-left texel address

	uvVec = _mm_sub_epi16(uvVec, uvStepVec);
	uvStepVec = _mm_slli_epi32(uvStepVec, 16);
	uvVec = _mm_add_epi16(uvVec, uvStepVec);

	__m128i bottomLeftAdr = GenerateAddressNew(baseVec, scaleVec, uvVec, wrapValVec);
	////////////////////////////////////////////////////
	// Generate bottom-right texel address

	uvStepVec = _mm_srli_epi32(uvStepVec, 16);
	uvVec = _mm_add_epi16(uvVec, uvStepVec);

	__m128i bottomRightAdr = GenerateAddressNew(baseVec, scaleVec, uvVec, wrapValVec);
	*/

	/*
	// old way
	////////////////////////////////////////////////////
	// Generate top-left texel address

	_declspec(align(16)) AddressScale addressScale;

	addressScale.bytesPerPixel = textures[mipLevel]->bytesPerPixel;
	addressScale.rowBytes = textures[mipLevel]->tileYstep;

	__m128i topLeftAdr = GenerateAddress(textures[mipLevel]->baseAddress,addressScale, uvVec);

	////////////////////////////////////////////////////
	// Generate top-right texel address
	
	// allOnes = dword {1,1,1,1}
	__m128i allOnes = _mm_srli_epi32(*(__m128i*)&negTerm, 15);

	__m128i steppedU = _mm_add_epi16(uIntVec, allOnes);

	__m128 widthMax = _mm_load1_ps((float*)&textures[mipLevel]->bufferWidth);

	__m128 widthStepA = _mm_load1_ps((float*)&textures[mipLevel]->bytesPerPixel);

	__m128 widthStepB = _mm_load1_ps((float*)&textures[mipLevel]->uWrapStep);

	__m128i uStep = TexelAddressStep(steppedU, *(__m128i*)&widthMax, *(__m128i*)&widthStepA,
		*(__m128i*)&widthStepB);	

	__m128i topRightAdr = _mm_add_epi32(topLeftAdr, uStep);

	////////////////////////////////////////////////////
	// Generate bottom-left texel address

	__m128i steppedV = _mm_add_epi16(vIntVec, allOnes);

	__m128 heightMax = _mm_load1_ps((float*)&textures[mipLevel]->bufferHeight);

	__m128 heightStepA = _mm_load1_ps((float*)&textures[mipLevel]->tileYstep);

	__m128 heightStepB = _mm_load1_ps((float*)&textures[mipLevel]->vWrapStep);

	__m128i vStep = TexelAddressStep(steppedV, *(__m128i*)&heightMax, *(__m128i*)&heightStepA, *(__m128i*)&heightStepB);

	__m128i bottomLeftAdr = _mm_add_epi32(topLeftAdr, vStep);

	////////////////////////////////////////////////////
	// Generate bottom-right texel address

	__m128i bottomRightAdr = _mm_add_epi32(bottomLeftAdr, uStep);
	
	//////////////////////////////////////////////////////////
	// Shuffle addresses so that all texel addresses for one screen pixel are 
	// in a single vector

	__m128i fetchAddress[4];

	// addressA = dword {topRight[1],topLeft[1],topRight[0],topLeft[0]}
	__m128i addressA = _mm_unpacklo_epi32(topLeftAdr, topRightAdr);

	// addressB = dword {bottomRight[1],bottomLeft[1],bottomRight[0],bottomLeft[0]}
	__m128i addressB = _mm_unpacklo_epi32(bottomLeftAdr, bottomRightAdr);

	// fetchAddress[0] = dword {bottomRight[0],bottomLeft[0],topRight[0],topLeft[0] }
	fetchAddress[0] = _mm_unpacklo_epi64(addressA, addressB);

	// fetchAddress[1] = dword {bottomRight[1],bottomLeft[1],topRight[1],topLeft[1] }
	fetchAddress[1] = _mm_unpackhi_epi64(addressA, addressB);

	// addressA = dword {topRight[3],topLeft[3],topRight[2],topLeft[2]}
	addressA = _mm_unpackhi_epi32(topLeftAdr, topRightAdr);

	// addressB = dword {bottomRight[3],bottomLeft[3],bottomRight[2],bottomLeft[2]}
	addressB = _mm_unpackhi_epi32(bottomLeftAdr, bottomRightAdr);

	// fetchAddress[2] = dword {bottomRight[2],bottomLeft[2],topRight[2],topLeft[2] }
	fetchAddress[2] = _mm_unpacklo_epi64(addressA, addressB);

	// fetchAddress[3] = dword {bottomRight[3],bottomLeft[3],topRight[3],topLeft[3] }
	fetchAddress[3] = _mm_unpackhi_epi64(addressA, addressB);

	_declspec(align(16)) Ceng::UINT32 *fetchPtr[4][4];

	_mm_store_si128((__m128i*)&fetchPtr[0][0], fetchAddress[0]);
	_mm_store_si128((__m128i*)&fetchPtr[1][0], fetchAddress[1]);
	_mm_store_si128((__m128i*)&fetchPtr[2][0], fetchAddress[2]);
	_mm_store_si128((__m128i*)&fetchPtr[3][0], fetchAddress[3]);

	////////////////////////////////////////////////////////
	// Setup texel fetch

	__m128i allZeroes = _mm_setzero_si128();

	__m128i finalColor[2] = { _mm_setzero_si128(), _mm_setzero_si128() };

	__m128i weight;
	__m128i color,reader;

	////////////////////////////////////////////////////////
	// Top-left fetch

	LinearFetchLow(finalColor[0],&fetchPtr[0][0], allZeroes, factor_topLeft);
	
	//////////////////////////////////////////////////////////////////
	// Top-right fetch

	LinearFetchHigh(finalColor[0], &fetchPtr[1][0], allZeroes, factor_topRight);

	//////////////////////////////////////////////////////////////////
	// Bottom-left fetch

	LinearFetchLow(finalColor[1], &fetchPtr[2][0], allZeroes, factor_bottomLeft);

	//////////////////////////////////////////////////////////////////
	// Bottom-right fetch

	LinearFetchHigh(finalColor[1], &fetchPtr[3][0], allZeroes, factor_bottomRight);

	//////////////////////////////////////////////////////////////////
	// Truncate color to integer

	finalColor[0] = _mm_srli_epi16(finalColor[0], 7);
	finalColor[1] = _mm_srli_epi16(finalColor[1], 7);

	//////////////////////////////////////////////////////////////////
	// Prefetch output address

	_mm_prefetch((char*)out_color, 1);

	//////////////////////////////////////////////////////////////////
	// Convert to vertical format

	// Dest is ubyte4

	// finalColor[0] = byte{0,a1,0,r1,0,g1,0,b1,0,a0,0,r0,0,g0,0,b0}
	// finalColor[1] = byte{0,a3,0,r3,0,g3,0,b3,0,a2,0,r2,0,g2,0,b2}

	__m128i colorTemp = _mm_slli_epi16(finalColor[1], 8);
	colorTemp = _mm_or_si128(colorTemp, finalColor[0]);

	// colorTemp = byte{a3,a1,r3,r1,g3,g1,b3,b1,a2,a0,r2,r0,g2,g0,b2,b0}
	
	__m128i writeColor = _mm_srli_si128(colorTemp, 8);

	// writeColor = byte {a3,a2,a1,a0,r3,r2,r1,r0,g3,g2,g1,g0,b3,b2,b1,b0}
	writeColor = _mm_unpacklo_epi8(colorTemp, writeColor);

	_mm_store_ps(&out_color[0], *(__m128*)&writeColor);

	return;

	// Dest is float4

	// finalColor[0] = word{a1,r1,g1,b1,a0,r0,g0,b0}
	// finalColor[1] = word{a3,r3,g3,b3,a2,r2,g2,b2}

	// blueGreenLow = dword {g1,g0,b1,b0}
	__m128i soaTempA = _mm_unpacklo_epi16(finalColor[0],finalColor[1]);

	// redAlphaLow = dword {a1,a0,r1,r0}
	__m128i soaTempB = _mm_unpackhi_epi16(finalColor[0],finalColor[1]);

	// blueGreenHigh = dword {g3,g2,b3,b2}
	__m128i blueGreen = _mm_unpacklo_epi16(soaTempA,soaTempB);

	// redAlphaHigh = dword {a3,a2,r3,r2}
	__m128i redAlpha = _mm_unpackhi_epi16(soaTempA,soaTempB);

	

	__m128 colorScaleVec = _mm_load1_ps(&colorScale);

	__m128i blue = _mm_unpacklo_epi16(blueGreen, allZeroes);
	__m128i green = _mm_unpackhi_epi16(blueGreen, allZeroes);
	__m128i red = _mm_unpacklo_epi16(redAlpha, allZeroes);

	__m128 blueF = _mm_cvtepi32_ps(blue);
	__m128 greenF = _mm_cvtepi32_ps(green);
	__m128 redF = _mm_cvtepi32_ps(red);

	__m128 alphaVec = _mm_load1_ps(&defaultAlpha);
	
	blueF = _mm_mul_ps(blueF, colorScaleVec);
	greenF = _mm_mul_ps(greenF, colorScaleVec);
	redF = _mm_mul_ps(redF, colorScaleVec);

	// Write default alpha

	_mm_store_ps(&out_color[0], blueF);
	_mm_store_ps(&out_color[4], greenF);
	_mm_store_ps(&out_color[8], redF);
	_mm_store_ps(&out_color[12], alphaVec);
}
*/

void CR_ShaderViewTex2D::Nearest_Port(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, Ceng::UINT32 mipLevel,
	const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color)
{
	Ceng::UINT32 k;

	Ceng::UINT32 uInt[4], vInt[4];

	for (k = 0; k < 4; ++k)
	{
		// Wrap coordinates to [0,1] range

		uInt[k] = uFX[k] - (uFX[k] & floorMask);
		vInt[k] = vFX[k] - (vFX[k] & floorMask);

		uInt[k] = uInt[k] * (textures[mipLevel]->bufferWidth);

		uInt[k] = uInt[k] >> 16;

		vInt[k] = vInt[k] * (textures[mipLevel]->bufferHeight);

		vInt[k] = vInt[k] >> 16;
	}

	Ceng::UINT8 *texelPtr;

	// Dest is ubyte4

	Ceng::UINT8 *byteOut = (Ceng::UINT8*)out_color;

	for (Ceng::UINT32 k = 0; k < 4; ++k)
	{

		Ceng::UINT32 tileX = uInt[k] >> 2;
		Ceng::UINT32 posX = uInt[k] & 3;

		Ceng::UINT32 tileY = vInt[k] >> 2;
		Ceng::UINT32 posY = vInt[k] & 3;

		/*
		Ceng::UINT32 tileX = uInt[k] >> 3;
		Ceng::UINT32 posX = uInt[k] & 7;

		Ceng::UINT32 tileY = vInt[k] >> 3;
		Ceng::UINT32 posY = vInt[k] & 7;
		*/

		/*
		Ceng::UINT32 tileX = uInt[k] >> 4;
		Ceng::UINT32 posX = uInt[k] & 15;

		Ceng::UINT32 tileY = vInt[k] >> 4;
		Ceng::UINT32 posY = vInt[k] & 15;
		*/

		texelPtr = (Ceng::UINT8*)textures[mipLevel]->baseAddress;

		texelPtr += textures[mipLevel]->tileXstep*tileX;
		texelPtr += textures[mipLevel]->tileYstep*tileY;

		texelPtr += textures[mipLevel]->channels[0].unitYstep*posY;
		texelPtr += textures[mipLevel]->channels[0].unitXstep*posX;

		byteOut[k] = texelPtr[0];
		byteOut[4 + k] = texelPtr[1];
		byteOut[8 + k] = texelPtr[2];
		byteOut[12 + k] = texelPtr[3];
	}
	/*
	// Scanline layout version

	// Dest is ubyte4

	Ceng::UINT8 *byteOut = (Ceng::UINT8*)out_color;

	for (Ceng::UINT32 k = 0; k < 4; ++k)
	{
	texelPtr = (Ceng::UINT8*)
	(textures[mipLevel]->baseAddress +
	textures[mipLevel]->bytesPerPixel*uInt[k] +
	textures[mipLevel]->tileYstep*vInt[k]);

	byteOut[k] = texelPtr[0];
	byteOut[4+k] = texelPtr[1];
	byteOut[8+k] = texelPtr[2];
	byteOut[12+k] = texelPtr[3];
	}
	*/

	return;

	// Dest is float4

	for (Ceng::UINT32 k = 0; k < 4; ++k)
	{
		texelPtr = (Ceng::UINT8*)
			(textures[mipLevel]->baseAddress +
			textures[mipLevel]->bytesPerPixel*uInt[k] +
			textures[mipLevel]->tileYstep*vInt[k]);

		out_color[k] = colorScale*Ceng::FLOAT32(texelPtr[0]);
		out_color[4 + k] = colorScale*Ceng::FLOAT32(texelPtr[1]);
		out_color[8 + k] = colorScale*Ceng::FLOAT32(texelPtr[2]);
		out_color[12 + k] = 1.0f;
	}
}

void CR_ShaderViewTex2D::Linear_Port(const Ceng::INT32 *uFX, const Ceng::INT32 *vFX, 
	Ceng::UINT32 mipLevel, const Ceng::FLOAT32 mipFactor, Ceng::FLOAT32 *out_color)
{
	Ceng::UINT32 k;

	// Scale by texture dimension and convert to integer

	_declspec(align(16)) Ceng::INT32 uInt[4], vInt[4];
	_declspec(align(16)) Ceng::INT32 uFrac[4], vFrac[4];

	_declspec(align(16)) Ceng::UINT32 factor[16];

	Ceng::UINT32 totalColor[16];

	for (k = 0; k < 4; ++k)
	{
		// Wrap coordinates to [0,1] range

		uInt[k] = uFX[k] - (uFX[k] & floorMask);
		vInt[k] = vFX[k] - (vFX[k] & floorMask);

		uInt[k] = uInt[k] * (textures[mipLevel]->bufferWidth) - 32768;

		//uInt[k] += uRounding;

		uFrac[k] = uInt[k] & 65535;
		uInt[k] = uInt[k] >> 16;

		if (uInt[k] < 0)
		{
			uInt[k] += textures[mipLevel]->bufferWidth;
		}

		vInt[k] = vInt[k] * (textures[mipLevel]->bufferHeight) - 32768;

		//vInt[k] += vRounding;

		vFrac[k] = vInt[k] & 65535;
		vInt[k] = vInt[k] >> 16;

		if (vInt[k] < 0)
		{
			vInt[k] += textures[mipLevel]->bufferHeight;
		}

		Ceng::UINT32 fracMul = (uFrac[k] >> 8)*(vFrac[k] >> 8);

		factor[4 * k + 0] = 65535 + fracMul - uFrac[k] - vFrac[k];
		factor[4 * k + 1] = uFrac[k] - fracMul;
		factor[4 * k + 2] = vFrac[k] - fracMul;
		factor[4 * k + 3] = fracMul;
	}

	Ceng::UINT32 tileX;
	Ceng::UINT32 posX;

	Ceng::UINT32 tileY;
	Ceng::UINT32 posY;

	Ceng::UINT8 *texelPtr;

	for (Ceng::UINT32 k = 0; k < 4; ++k)
	{
		// Top-left texel

		tileX = uInt[k] >> 2;
		posX = uInt[k] & 3;

		tileY = vInt[k] >> 2;
		posY = vInt[k] & 3;

		texelPtr = (Ceng::UINT8*)textures[mipLevel]->baseAddress;

		texelPtr += textures[mipLevel]->tileXstep*tileX;
		texelPtr += textures[mipLevel]->tileYstep*tileY;

		texelPtr += textures[mipLevel]->channels[0].unitYstep*posY;
		texelPtr += textures[mipLevel]->channels[0].unitXstep*posX;

		totalColor[4 * k + 0] = texelPtr[0] * factor[4 * k + 0];
		totalColor[4 * k + 1] = texelPtr[1] * factor[4 * k + 0];
		totalColor[4 * k + 2] = texelPtr[2] * factor[4 * k + 0];
		totalColor[4 * k + 3] = texelPtr[3] * factor[4 * k + 0];

		// Top-right texel

		Ceng::INT32 u = uInt[k] + 1;

		// Wrap mode
		if (u == textures[mipLevel]->bufferWidth)
		{
			u = 0;
		}

		tileX = u >> 2;
		posX = u & 3;

		tileY = vInt[k] >> 2;
		posY = vInt[k] & 3;

		texelPtr = (Ceng::UINT8*)textures[mipLevel]->baseAddress;

		texelPtr += textures[mipLevel]->tileXstep*tileX;
		texelPtr += textures[mipLevel]->tileYstep*tileY;

		texelPtr += textures[mipLevel]->channels[0].unitYstep*posY;
		texelPtr += textures[mipLevel]->channels[0].unitXstep*posX;

		totalColor[4 * k + 0] += texelPtr[0] * factor[4 * k + 1];
		totalColor[4 * k + 1] += texelPtr[1] * factor[4 * k + 1];
		totalColor[4 * k + 2] += texelPtr[2] * factor[4 * k + 1];
		totalColor[4 * k + 3] += texelPtr[3] * factor[4 * k + 1];

		// Bottom-left texel

		Ceng::INT32 v = vInt[k] + 1;

		// Wrap mode
		if (v == textures[mipLevel]->bufferHeight)
		{
			v = 0;
		}

		tileX = uInt[k] >> 2;
		posX = uInt[k] & 3;

		tileY = v >> 2;
		posY = v & 3;

		texelPtr = (Ceng::UINT8*)textures[mipLevel]->baseAddress;

		texelPtr += textures[mipLevel]->tileXstep*tileX;
		texelPtr += textures[mipLevel]->tileYstep*tileY;

		texelPtr += textures[mipLevel]->channels[0].unitYstep*posY;
		texelPtr += textures[mipLevel]->channels[0].unitXstep*posX;

		totalColor[4 * k + 0] += texelPtr[0] * factor[4 * k + 2];
		totalColor[4 * k + 1] += texelPtr[1] * factor[4 * k + 2];
		totalColor[4 * k + 2] += texelPtr[2] * factor[4 * k + 2];
		totalColor[4 * k + 3] += texelPtr[3] * factor[4 * k + 2];


		// Bottom-right texel

		tileX = u >> 2;
		posX = u & 3;

		tileY = v >> 2;
		posY = v & 3;

		texelPtr = (Ceng::UINT8*)textures[mipLevel]->baseAddress;

		texelPtr += textures[mipLevel]->tileXstep*tileX;
		texelPtr += textures[mipLevel]->tileYstep*tileY;

		texelPtr += textures[mipLevel]->channels[0].unitYstep*posY;
		texelPtr += textures[mipLevel]->channels[0].unitXstep*posX;

		totalColor[4 * k + 0] += texelPtr[0] * factor[4 * k + 3];
		totalColor[4 * k + 1] += texelPtr[1] * factor[4 * k + 3];
		totalColor[4 * k + 2] += texelPtr[2] * factor[4 * k + 3];
		totalColor[4 * k + 3] += texelPtr[3] * factor[4 * k + 3];

		totalColor[4 * k + 0] >>= 16;
		totalColor[4 * k + 1] >>= 16;
		totalColor[4 * k + 2] >>= 16;
		totalColor[4 * k + 3] >>= 16;
	}

	Ceng::UINT8 *byteOut = (Ceng::UINT8*)out_color;

	byteOut[0] = totalColor[0];
	byteOut[1] = totalColor[4];
	byteOut[2] = totalColor[8];
	byteOut[3] = totalColor[12];

	byteOut[4] = totalColor[1];
	byteOut[5] = totalColor[5];
	byteOut[6] = totalColor[9];
	byteOut[7] = totalColor[13];

	byteOut[8] = totalColor[2];
	byteOut[9] = totalColor[6];
	byteOut[10] = totalColor[10];
	byteOut[11] = totalColor[14];

	byteOut[12] = totalColor[3];
	byteOut[13] = totalColor[7];
	byteOut[14] = totalColor[11];
	byteOut[15] = totalColor[15];

	/*
	out_color[0] = colorScale*Ceng::FLOAT32(totalColor[0]);
	out_color[1] = colorScale*Ceng::FLOAT32(totalColor[4]);
	out_color[2] = colorScale*Ceng::FLOAT32(totalColor[8]);
	out_color[3] = colorScale*Ceng::FLOAT32(totalColor[12]);

	out_color[4] = colorScale*Ceng::FLOAT32(totalColor[1]);
	out_color[5] = colorScale*Ceng::FLOAT32(totalColor[5]);
	out_color[6] = colorScale*Ceng::FLOAT32(totalColor[9]);
	out_color[7] = colorScale*Ceng::FLOAT32(totalColor[13]);

	out_color[8] = colorScale*Ceng::FLOAT32(totalColor[2]);
	out_color[9] = colorScale*Ceng::FLOAT32(totalColor[6]);
	out_color[10] = colorScale*Ceng::FLOAT32(totalColor[10]);
	out_color[11] = colorScale*Ceng::FLOAT32(totalColor[14]);

	out_color[12] = colorScale*Ceng::FLOAT32(totalColor[3]);
	out_color[13] = colorScale*Ceng::FLOAT32(totalColor[7]);
	out_color[14] = colorScale*Ceng::FLOAT32(totalColor[11]);
	out_color[15] = colorScale*Ceng::FLOAT32(totalColor[15]);
	*/
}