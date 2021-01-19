/*****************************************************************************
*
* cr-texture-2d.cpp
*
* By Jari Korkala 5/2013
*
*****************************************************************************/

#include <ceng/datatypes/bufferdata-2d.h>
#include <ceng/interfaces/render-target.h>

#include "cr-texture-2d.h"

#include <ceng/datatypes/tex2d-desc.h>

#include <ceng/datatypes/shader-resource-view-desc.h>

#include "cr-shader-view-tex2d.h"

using namespace Ceng;

CR_Texture2D::CR_Texture2D()
{
}

CR_Texture2D::CR_Texture2D(const Texture2dDesc &desc, TextureArrayVector &&textures)
	: desc(desc),textures(textures)
{
}

CR_Texture2D::~CR_Texture2D()
{
}

void CR_Texture2D::Release()
{
	delete this;
}

const BufferType::value CR_Texture2D::GetResourceType()
{
	return BufferType::texture_2d;
}

const CRESULT CR_Texture2D::GetShaderViewTex2D(const ShaderResourceViewDesc &viewDesc, ShaderResourceView **out_view)
{
	TextureMipVector tempTex;

	size_t start = size_t(viewDesc.tex2d.baseMipLevel);
	size_t end = size_t(viewDesc.tex2d.maxMipLevel);

	if (start > textures[0].size())
	{
		start = textures[0].size();
	}

	if (end > textures[0].size())
	{
		end = textures[0].size();
	}

	for (size_t k = start; k < end; ++k)
	{
		if (k < textures[0].size())
		{
			tempTex.push_back(textures[0][k]);
		}
	}

	Texture2dDesc texDesc(desc);

	texDesc.width = desc.width >> viewDesc.tex2d.baseMipLevel;
	texDesc.height = desc.height >> viewDesc.tex2d.baseMipLevel;

	texDesc.arraySize = 1;
	texDesc.mipLevels = end-start;

	*out_view = new CR_ShaderViewTex2D(texDesc, std::move(tempTex));

	return CE_OK;
}

const CRESULT CR_Texture2D::GetBufferData2D(Ceng::BufferData2D *dataPtr)
{
	return CE_OK;
}
		
const CRESULT CR_Texture2D::GetRenderTarget(Ceng::RenderTarget **targetPtr)
{
	return CE_OK;
}

const CRESULT CR_Texture2D::GenerateMipMaps(const Ceng::UINT32 baseLevel)
{
	CRESULT cresult;
	
	for (Ceng::UINT32 k = 0; k < textures.size(); ++k)
	{
		for (Ceng::UINT32 j = baseLevel; j < desc.mipLevels-1; ++j)
		{
			cresult = SuperSampleDown(textures[k][j], textures[k][j + 1]);
		}
	}
	

	return CE_OK;
}

const CRESULT CR_Texture2D::SuperSampleHorizontal1D(std::shared_ptr<CR_NewTargetData> &source,
	std::shared_ptr<CR_NewTargetData> &out_texture)
{
	return CE_OK;
}

const CRESULT CR_Texture2D::SuperSampleVertical1D(std::shared_ptr<CR_NewTargetData> &source,
	std::shared_ptr<CR_NewTargetData> &out_texture)
{
	return CE_OK;
}

const CRESULT CR_Texture2D::SuperSampleDown(std::shared_ptr<CR_NewTargetData> &source,std::shared_ptr<CR_NewTargetData> &out_texture)
{
	/*
	if (source->bufferWidth == 1)
	{
		return SuperSampleHorizontal1D(source, out_texture);
	}
	else if (source->bufferHeight == 1)
	{
		return SuperSampleHorizontal1D(source, out_texture);
	}
	*/

	Ceng::UINT32 newWidth = out_texture->bufferWidth;
	Ceng::UINT32 newHeight = out_texture->bufferHeight;

	Ceng::UINT32 oddWidth, oddHeight;

	oddWidth = source->bufferWidth & 1;
	oddHeight = source->bufferHeight & 1;

	Ceng::UINT32 k;

	Ceng::UINT32 halfWidth = newWidth >> 1;
	Ceng::UINT32 halfHeight = newHeight >> 1;

	Ceng::UINT8 *sourcePtr = (Ceng::UINT8*)source->baseAddress;

	Ceng::UINT32 bytesPerPixel = source->bytesPerPixel;
	Ceng::UINT32 sourcePitch = source->tileYstep;

	Ceng::UINT8 *destPtr = (Ceng::UINT8*)out_texture->baseAddress;

	Ceng::UINT32 destPitch = out_texture->tileYstep;

	if (halfHeight > 0)
	{
		for (k = 0; k < halfHeight - oddHeight; k++)
		{
			if (halfWidth > 0)
			{
				SuperSampleHorizontal(destPtr, destPitch, sourcePtr, sourcePitch, bytesPerPixel, 0, halfWidth - oddWidth, k);
			}

			if (oddWidth == 1)
			{
				if (newWidth > 1)
				{
					SuperSampleHorizontalCenter(destPtr, destPitch, sourcePtr, sourcePitch,
						bytesPerPixel, halfWidth - 1, k);
				}

				SuperSampleHorizontalCenter(destPtr, destPitch, sourcePtr, sourcePitch, bytesPerPixel, halfWidth, k);
			}

			if (halfWidth > 0)
			{
				SuperSampleHorizontal(destPtr, destPitch, sourcePtr, sourcePitch, bytesPerPixel, halfWidth + oddWidth, newWidth, k);
			}
		}
	}

	
	if (oddHeight == 1)
	{
		if (newHeight > 1)
		{
			if (halfWidth > 0)
			{
				SuperSampleHorizontalOdd(destPtr, destPitch, sourcePtr, sourcePitch, bytesPerPixel, 0, halfWidth - oddWidth, halfHeight - 1);
			}

			if (oddWidth == 1)
			{
				if (newWidth > 1)
				{
					SuperSampleTrueCenter(destPtr, destPitch, sourcePtr, sourcePitch, bytesPerPixel, halfWidth - 1, halfHeight-1);
				}

				SuperSampleTrueCenter(destPtr, destPitch, sourcePtr, sourcePitch, bytesPerPixel, halfWidth, halfHeight-1);
			}

			if (halfWidth > 0)
			{
				SuperSampleHorizontalOdd(destPtr, destPitch, sourcePtr, sourcePitch, bytesPerPixel, halfWidth + oddWidth, newWidth, halfHeight - 1);
			}
		}

		if (halfWidth > 0)
		{
			SuperSampleHorizontalOdd(destPtr, destPitch, sourcePtr, sourcePitch, bytesPerPixel, 0, halfWidth - oddWidth, halfHeight);
		}

		if (oddWidth == 1)
		{
			if (newWidth > 1)
			{
				SuperSampleTrueCenter(destPtr, destPitch, sourcePtr, sourcePitch, bytesPerPixel, halfWidth - 1, halfHeight);
			}

			SuperSampleTrueCenter(destPtr, destPitch, sourcePtr, sourcePitch, bytesPerPixel, halfWidth, halfHeight);
		}

		if (halfWidth > 0)
		{
			SuperSampleHorizontalOdd(destPtr, destPitch, sourcePtr, sourcePitch, bytesPerPixel, halfWidth + oddWidth, newWidth, halfHeight);
		}
	}

	if (halfHeight > 0)
	{
		for (k = halfHeight + oddHeight; k < newHeight; k++)
		{
			if (halfWidth > 0)
			{
				SuperSampleHorizontal(destPtr, destPitch, sourcePtr, sourcePitch, bytesPerPixel, 0, halfWidth - oddWidth, k);
			}

			if (oddWidth == 1)
			{
				if (newWidth > 1)
				{
					SuperSampleHorizontalCenter(destPtr, destPitch, sourcePtr, sourcePitch,
						bytesPerPixel, halfWidth - 1, k);
				}

				SuperSampleHorizontalCenter(destPtr, destPitch, sourcePtr, sourcePitch, bytesPerPixel, halfWidth, k);
			}

			if (halfWidth > 0)
			{
				SuperSampleHorizontal(destPtr, destPitch, sourcePtr, sourcePitch, bytesPerPixel, halfWidth + oddWidth, newWidth, k);
			}
		}
	}

	return CE_OK;
}

void CR_Texture2D::SuperSampleTrueCenter(Ceng::UINT8 *destScanline, const Ceng::UINT32 destPitch,
	Ceng::UINT8 *sourceScanline, const Ceng::UINT32 sourcePitch,
	const Ceng::UINT32 bytesPerPixel, const Ceng::UINT32 x, const Ceng::UINT32 y)
{
	Ceng::UINT32 red, green, blue,alpha;

	Ceng::UINT32 sourceStepX = 2 * bytesPerPixel*x;
	Ceng::UINT32 sourceStepY = 2 * sourcePitch*y;

	blue = sourceScanline[sourceStepX + sourceStepY];
	green = sourceScanline[sourceStepX + sourceStepY + 1];
	red = sourceScanline[sourceStepX + sourceStepY + 2];
	alpha = sourceScanline[sourceStepX + sourceStepY + 3];

	blue += 2*sourceScanline[sourceStepX + sourceStepY + 4];
	green += 2*sourceScanline[sourceStepX + sourceStepY + 5];
	red += 2*sourceScanline[sourceStepX + sourceStepY + 6];
	alpha += 2*sourceScanline[sourceStepX + sourceStepY + 7];

	blue += sourceScanline[sourceStepX + sourceStepY + 8];
	green += sourceScanline[sourceStepX + sourceStepY + 9];
	red += sourceScanline[sourceStepX + sourceStepY + 10];
	alpha += sourceScanline[sourceStepX + sourceStepY + 11];

	sourceStepY += sourcePitch;

	blue += 2*sourceScanline[sourceStepX + sourceStepY];
	green += 2*sourceScanline[sourceStepX + sourceStepY + 1];
	red += 2*sourceScanline[sourceStepX + sourceStepY + 2];
	alpha += 2*sourceScanline[sourceStepX + sourceStepY + 3];

	blue += 4*sourceScanline[sourceStepX + sourceStepY + 4];
	green += 4*sourceScanline[sourceStepX + sourceStepY + 5];
	red += 4*sourceScanline[sourceStepX + sourceStepY + 6];
	alpha += 4*sourceScanline[sourceStepX + sourceStepY + 7];

	blue += 2*sourceScanline[sourceStepX + sourceStepY + 8];
	green += 2*sourceScanline[sourceStepX + sourceStepY + 9];
	red += 2*sourceScanline[sourceStepX + sourceStepY + 10];
	alpha += 2*sourceScanline[sourceStepX + sourceStepY + 11];

	sourceStepY += sourcePitch;

	blue += sourceScanline[sourceStepX + sourceStepY];
	green += sourceScanline[sourceStepX + sourceStepY + 1];
	red += sourceScanline[sourceStepX + sourceStepY + 2];
	alpha += sourceScanline[sourceStepX + sourceStepY + 3];

	blue += 2*sourceScanline[sourceStepX + sourceStepY + 4];
	green += 2*sourceScanline[sourceStepX + sourceStepY + 5];
	red += 2*sourceScanline[sourceStepX + sourceStepY + 6];
	alpha += 2*sourceScanline[sourceStepX + sourceStepY + 7];

	blue += sourceScanline[sourceStepX + sourceStepY + 8];
	green += sourceScanline[sourceStepX + sourceStepY + 9];
	red += sourceScanline[sourceStepX + sourceStepY + 10];
	alpha += sourceScanline[sourceStepX + sourceStepY + 11];

	red = red / 16;
	green = green / 16;
	blue = blue / 16;
	alpha = alpha / 16;

	destScanline[destPitch*y + bytesPerPixel*x] = Ceng::UINT8(blue);
	destScanline[destPitch*y + bytesPerPixel*x + 1] = Ceng::UINT8(green);
	destScanline[destPitch*y + bytesPerPixel*x + 2] = Ceng::UINT8(red);
	destScanline[destPitch*y + bytesPerPixel*x + 3] = Ceng::UINT8(alpha);
}

void CR_Texture2D::SuperSampleHorizontalCenter(Ceng::UINT8 *destScanline, const Ceng::UINT32 destPitch,
	Ceng::UINT8 *sourceScanline, const Ceng::UINT32 sourcePitch,
	const Ceng::UINT32 bytesPerPixel, const Ceng::UINT32 x, const Ceng::UINT32 y)
{
	Ceng::UINT32 red, green, blue,alpha;

	Ceng::UINT32 sourceStepX = 2 * bytesPerPixel*x;
	Ceng::UINT32 sourceStepY = 2 * sourcePitch*y;


	blue = sourceScanline[sourceStepX + sourceStepY];
	green = sourceScanline[sourceStepX + sourceStepY + 1];
	red = sourceScanline[sourceStepX + sourceStepY + 2];
	alpha = sourceScanline[sourceStepX + sourceStepY + 3];

	blue += 2*sourceScanline[sourceStepX + sourceStepY + 4];
	green += 2*sourceScanline[sourceStepX + sourceStepY + 5];
	red += 2*sourceScanline[sourceStepX + sourceStepY + 6];
	alpha += 2*sourceScanline[sourceStepX + sourceStepY + 7];

	blue += sourceScanline[sourceStepX + sourceStepY + 8];
	green += sourceScanline[sourceStepX + sourceStepY + 9];
	red += sourceScanline[sourceStepX + sourceStepY + 10];
	alpha += sourceScanline[sourceStepX + sourceStepY + 11];

	sourceStepY += sourcePitch;

	blue += sourceScanline[sourceStepX + sourceStepY];
	green += sourceScanline[sourceStepX + sourceStepY + 1];
	red += sourceScanline[sourceStepX + sourceStepY + 2];
	alpha += sourceScanline[sourceStepX + sourceStepY + 3];

	blue += 2*sourceScanline[sourceStepX + sourceStepY + 4];
	green += 2*sourceScanline[sourceStepX + sourceStepY + 5];
	red += 2*sourceScanline[sourceStepX + sourceStepY + 6];
	alpha += 2*sourceScanline[sourceStepX + sourceStepY + 7];

	blue += sourceScanline[sourceStepX + sourceStepY + 8];
	green += sourceScanline[sourceStepX + sourceStepY + 9];
	red += sourceScanline[sourceStepX + sourceStepY + 10];
	alpha += sourceScanline[sourceStepX + sourceStepY + 11];

	red = red / 8;
	green = green / 8;
	blue = blue / 8;
	alpha = alpha / 8;	

	destScanline[destPitch*y + bytesPerPixel*x] = Ceng::UINT8(blue);
	destScanline[destPitch*y + bytesPerPixel*x + 1] = Ceng::UINT8(green);
	destScanline[destPitch*y + bytesPerPixel*x + 2] = Ceng::UINT8(red);
	destScanline[destPitch*y + bytesPerPixel*x + 3] = Ceng::UINT8(alpha);
}

void CR_Texture2D::SuperSampleHorizontal(Ceng::UINT8 *destScanline,const Ceng::UINT32 destPitch,
	Ceng::UINT8 *sourceScanline,const Ceng::UINT32 sourcePitch, 
	const Ceng::UINT32 bytesPerPixel,const Ceng::UINT32 xStart, const Ceng::UINT32 xEnd, const Ceng::UINT32 y)
{
	Ceng::UINT32 red,green,blue,alpha;

	Ceng::UINT32 doubleBytes = 2 * bytesPerPixel;

	for (Ceng::UINT32 j = xStart; j<xEnd; j++)
	{
		blue = sourceScanline[2 * sourcePitch*y + doubleBytes * j];
		green = sourceScanline[2 * sourcePitch*y + doubleBytes * j+1];
		red = sourceScanline[2 * sourcePitch*y + doubleBytes * j+2];
		alpha = sourceScanline[2 * sourcePitch*y + doubleBytes * j + 3];

		blue += sourceScanline[2 * sourcePitch*y + doubleBytes * j + 4];
		green += sourceScanline[2 * sourcePitch*y + doubleBytes * j + 5];
		red += sourceScanline[2 * sourcePitch*y + doubleBytes * j + 6];
		alpha += sourceScanline[2 * sourcePitch*y + doubleBytes * j + 7];

		blue += sourceScanline[sourcePitch*(2*y+1) + doubleBytes * j];
		green += sourceScanline[sourcePitch*(2*y+1) + doubleBytes * j + 1];
		red += sourceScanline[sourcePitch*(2*y+1) + doubleBytes * j + 2];
		alpha += sourceScanline[sourcePitch*(2 * y + 1) + doubleBytes * j + 3];
		
		blue += sourceScanline[sourcePitch*(2*y+1) + doubleBytes * j + 4];
		green += sourceScanline[sourcePitch*(2*y+1) + doubleBytes * j + 5];
		red += sourceScanline[sourcePitch*(2*y+1) + doubleBytes * j + 6];
		alpha += sourceScanline[sourcePitch*(2 * y + 1) + doubleBytes * j + 7];
		
		red = red >> 2;
		green = green >> 2;
		blue = blue >> 2;
		alpha = alpha >> 2;

		destScanline[destPitch*y + bytesPerPixel*j] = Ceng::UINT8(blue);
		destScanline[destPitch*y + bytesPerPixel*j+1] = Ceng::UINT8(green);
		destScanline[destPitch*y + bytesPerPixel*j+2] = Ceng::UINT8(red);
		destScanline[destPitch*y + bytesPerPixel*j + 3] = Ceng::UINT8(alpha);
	}
}

void CR_Texture2D::SuperSampleHorizontalOdd(Ceng::UINT8 *destScanline, const Ceng::UINT32 destPitch,
	Ceng::UINT8 *sourceScanline, const Ceng::UINT32 sourcePitch,
	const Ceng::UINT32 bytesPerPixel, const Ceng::UINT32 xStart, const Ceng::UINT32 xEnd, const Ceng::UINT32 y)
{
	Ceng::UINT32 red, green, blue,alpha;

	Ceng::UINT32 doubleBytes = 2 * bytesPerPixel;

	for (Ceng::UINT32 j = xStart; j<xEnd; j++)
	{
		blue = sourceScanline[2 * sourcePitch*y + doubleBytes * j];
		green = sourceScanline[2 * sourcePitch*y + doubleBytes * j + 1];
		red = sourceScanline[2 * sourcePitch*y + doubleBytes * j + 2];
		alpha = sourceScanline[2 * sourcePitch*y + doubleBytes * j + 3];		

		blue += sourceScanline[2 * sourcePitch*y + doubleBytes * j + 4];
		green += sourceScanline[2 * sourcePitch*y + doubleBytes * j + 5];
		red += sourceScanline[2 * sourcePitch*y + doubleBytes * j + 6];
		alpha += sourceScanline[2 * sourcePitch*y + doubleBytes * j + 7];

		blue += 2*sourceScanline[sourcePitch*(2*y + 1) + doubleBytes * j];
		green += 2*sourceScanline[sourcePitch*(2*y + 1) + doubleBytes * j + 1];
		red += 2*sourceScanline[sourcePitch*(2*y + 1) + doubleBytes * j + 2];
		alpha += 2 * sourceScanline[sourcePitch*(2 * y + 1) + doubleBytes * j + 3];

		blue += 2*sourceScanline[sourcePitch*(2*y + 1) + doubleBytes * j + 4];
		green += 2*sourceScanline[sourcePitch*(2*y + 1) + doubleBytes * j + 5];
		red += 2*sourceScanline[sourcePitch*(2*y + 1) + doubleBytes * j + 6];
		alpha += 2 * sourceScanline[sourcePitch*(2 * y + 1) + doubleBytes * j + 7];
		
		blue += sourceScanline[sourcePitch*(2*y + 2) + doubleBytes * j + 0];
		green += sourceScanline[sourcePitch*(2*y + 2) + doubleBytes * j + 1];
		red += sourceScanline[sourcePitch*(2*y + 2) + doubleBytes * j + 2];
		alpha += sourceScanline[sourcePitch*(2 * y + 2) + doubleBytes * j + 3];

		blue += sourceScanline[sourcePitch*(2*y + 2) + doubleBytes * j + 4];
		green += sourceScanline[sourcePitch*(2*y + 2) + doubleBytes * j + 5];
		red += sourceScanline[sourcePitch*(2*y + 2) + doubleBytes * j + 6];
		alpha += sourceScanline[sourcePitch*(2 * y + 2) + doubleBytes * j + 7];
		

		red = red / 8;
		green = green / 8;
		blue = blue / 8;		
		alpha = alpha / 8;

		destScanline[destPitch*y + bytesPerPixel*j] = Ceng::UINT8(blue);
		destScanline[destPitch*y + bytesPerPixel*j + 1] = Ceng::UINT8(green);
		destScanline[destPitch*y + bytesPerPixel*j + 2] = Ceng::UINT8(red);
		destScanline[destPitch*y + bytesPerPixel*j + 3] = Ceng::UINT8(alpha);
	}
}