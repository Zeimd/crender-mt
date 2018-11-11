/*****************************************************************************
*
* cache-tag.h
*
* Created By Jari Korkala 27/2/2015.
*
* class FragmentCacheTag moved here.
*
*****************************************************************************/

#ifndef CENG_CACHE_TAG_H
#define CENG_CACHE_TAG_H

#include <ceng/datatypes/basic-types.h>

namespace Ceng
{
	class FragmentCacheTag
	{
	public:
		/**
		 * Index to vertex buffer.
		 */
		Ceng::INT32 tag;

		/**
		 * Corresponding fragment.
		 */
		Ceng::INT32 cacheIndex;

		/**
		 * Absolute memory address of the fragment.
		 */
		Ceng::UINT8* address;

	public:

		FragmentCacheTag()
		{
			tag = -1;
			cacheIndex = -1;
			address = nullptr;
		}

		~FragmentCacheTag()
		{
		}
	};
}

#endif