/*****************************************************************************
*
* texture-loader.cpp
*
* Created By Jari Korkala 7/3/2015
*
*****************************************************************************/

#include "../include/engine-v2.h"

//#include <ceng/datatypes/bufferdata-2d.h>

#include <stdio.h>

const Ceng::CRESULT CreateTexture2dFromFile(const Ceng::StringUtf8 &file, Ceng::Texture2dDesc &desc,
	Ceng::RenderDevice *renderDevice,Ceng::Texture2D **out_texture)
{
	Ceng::StringUtf8::CONST_ITERATOR_TYPE iter = file.FindLast('.',file.ConstBeginIterator());

	if (iter == file.ConstEndIterator())
	{
		return Ceng::CE_ERR_INVALID_PARAM;
	}

	Ceng::StringUtf8 extension;

	while (++iter != file.ConstEndIterator())
	{
		extension += *iter;
	}

	if (extension == "bmp")
	{
		return CreateTexture2d_BMP(file, desc, renderDevice,out_texture);
	}

	return Ceng::CE_OK;
}

const Ceng::CRESULT CreateTexture2d_BMP(const Ceng::StringUtf8 &file, Ceng::Texture2dDesc &desc,
	Ceng::RenderDevice *renderDevice, Ceng::Texture2D **out_texture)
{
	FILE *stream;
	Ceng::UINT8 buffer[16];
	Ceng::INT32 k;
	Ceng::BOOL windowsbitmap = false;

	Ceng::UINT32 filesize = 0;
	Ceng::UINT32 bitmapoffset = 0;
	Ceng::UINT32 bitmapsize = 0;
	Ceng::UINT32 padding = 0;

	Ceng::UINT32 headersize = 0;
	Ceng::UINT32 compressionmethod = 0;
	Ceng::UINT32 importantcolors = 0;
	Ceng::UINT16 colorplanes = 0;
	Ceng::UINT16 colordepthBits = 0;
	Ceng::UINT32 paletteSize = 0;

	errno_t result = fopen_s(&stream, file.ToCString(), "rb");
	if (result != 0)
	{
		return Ceng::CE_ERR_INVALID_PARAM;
	}

	fread(&buffer[0], 1, 2, stream); // Read identification data
	if (feof(stream))
	{
		fclose(stream);
		return Ceng::CE_ERR_FILE_READ;
	}

	// Check that the file is a bitmap
	if (buffer[0] != 'B' && buffer[1] != 'M')
	{
		fclose(stream);
		return Ceng::CE_ERR_FILE_READ;
	}

	// Read bitmap header
	fread(&filesize, 4, 1, stream);
	fread(&buffer[0], 4, 1, stream); // Skip unused data
	fread(&bitmapoffset, 4, 1, stream);

	fread(&headersize, 4, 1, stream);

	// Read bitmap info
	switch (headersize)
	{
	case 12: // OS/2 header
		fclose(stream);
		return Ceng::CE_ERR_NOT_SUPPORTED;
		break;
	case 40: // Windows bitmap version 3
		fread(&desc.width, 4, 1, stream);
		fread(&desc.height, 4, 1, stream);
		fread(&colorplanes, 2, 1, stream); // Color plane counter not used so skip it			
		fread(&colordepthBits, 2, 1, stream); // Read color depth

		fread(&compressionmethod, 4, 1, stream);
		fread(&bitmapsize, 4, 1, stream);
		fread(&buffer[0], 4, 2, stream); // Skip physical resolution data
		fread(&paletteSize, 4, 1, stream);
		fread(&importantcolors, 4, 1, stream); // Skip important colors

		if (compressionmethod != 0)
		{
			fclose(stream);
			return Ceng::CE_ERR_NOT_SUPPORTED;
		}

		windowsbitmap = true;
		break;
	case 64:	// OS/2 v2
		fclose(stream);
		return Ceng::CE_ERR_NOT_SUPPORTED;
		break;
	case 108: // Windows version 4
		fclose(stream);
		windowsbitmap = true;
		return Ceng::CE_ERR_NOT_SUPPORTED;
		break;
	case 124: // Windows version 5
		fclose(stream);
		windowsbitmap = true;
		return Ceng::CE_ERR_NOT_SUPPORTED;
		break;
	default: // Unknown header type
		fclose(stream);
		return Ceng::CE_ERR_NOT_SUPPORTED;
	}

	Ceng::UINT32 bytesPerPixel = colordepthBits / 8;

	switch (bytesPerPixel)
	{
	case 1:
		fclose(stream);
		return Ceng::CE_ERR_NOT_SUPPORTED;
	case 2:
		desc.format = Ceng::IMAGE_FORMAT::C16_A1;
		//colorFormat = COLOR_A1R5G5B5;
		break;
	case 3:
		desc.format = Ceng::IMAGE_FORMAT::C24_RGB;
		break;
	case 4:
		desc.format = Ceng::IMAGE_FORMAT::C32_ARGB;
		break;
	default:
		fclose(stream);
		return Ceng::CE_ERR_NOT_SUPPORTED;
		break;
	}

	// Calculate correct image size if not specified in the header
	//if (bitmapsize == 0) 
	//{
	bitmapsize = desc.width*desc.height*bytesPerPixel;
	//}

	// Allocate space for the bitmap
	Ceng::UINT8 *bitmap = (Ceng::UINT8*)malloc(bitmapsize);
	if (bitmap == nullptr)
	{
		fclose(stream);
		return Ceng::CE_ERR_OUT_OF_MEMORY;
	}

	/*
	usekey = 0;
	colorkey[0] = 0;
	colorkey[1] = 0;
	colorkey[2] = 0;
	colorkey[3] = 0;

	// Read palette if it exists
	if (colorFormat == COLOR_PAL256 && palettesize > 0)
	{
		palette = (PALETTEENTRY*)malloc(palettesize*sizeof(PALETTEENTRY));
		if (palette == NULL)
		{
			fclose(stream);
			free(bitmap);
			return 0;
		}
		for (k = 0; k<palettesize; k++)
		{
			fread(&palette[k].peBlue, 1, 1, stream);
			fread(&palette[k].peGreen, 1, 1, stream);
			fread(&palette[k].peRed, 1, 1, stream);
			if (windowsbitmap == true) fread(&buffer[0], 1, 1, stream); // Skip the last byte
			palette[k].peFlags = 0;
		}
	}
	else
	{
		palettesize = 0;
	}
	*/

	padding = (desc.width*bytesPerPixel) % 4;
	if (padding > 0)
	{
		padding = 4 - padding;
	}

	// Debug dump

	Ceng::UINT32 offset;
	
	// Read the bitmap
	fseek(stream, bitmapoffset, SEEK_SET);
	
	Ceng::UINT32 rowPitch = desc.width*bytesPerPixel;

	offset = bitmapsize - rowPitch;
	for (k = desc.height; k>0; k--)
	{
		fread(&bitmap[offset], 1, rowPitch, stream);
		fread(&buffer[0], 1, padding, stream);
		offset -= rowPitch;
	}

	fclose(stream);

	// Configure descriptor

	Ceng::SubResourceData image;
	
	image.sourcePtr = bitmap;
	image.rowPitch = rowPitch;
	image.depthPitch = bitmapsize;

	Ceng::CRESULT cresult = Ceng::CE_OK;

	cresult = renderDevice->CreateTexture2D(desc, &image, out_texture);

	free(bitmap);

	return cresult;
}
