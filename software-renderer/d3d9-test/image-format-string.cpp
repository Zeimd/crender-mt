/*****************************************************************************
*
* image-format-string.cpp
*
* Created By Jari Korkala 2/3/2015
*
*****************************************************************************/

#include <ceng.h>

const Ceng::String ImageFormatToString(const Ceng::IMAGE_FORMAT::value format)
{
	switch (format)
	{
	case Ceng::IMAGE_FORMAT::C32_ABGR:
		return "C32_ABGR";
	case Ceng::IMAGE_FORMAT::C32_XBGR:
		return "C32_XBGR";

	case Ceng::IMAGE_FORMAT::C32_ARGB:
		return "C32_ARGB";
	case Ceng::IMAGE_FORMAT::C32_XRGB:
		return "C32_XRGB";
	}

	return "UNKNOWN";
}