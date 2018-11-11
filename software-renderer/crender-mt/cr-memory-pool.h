/*****************************************************************************
*
* cr-memory-pool.h
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#ifndef _CENG_CR_MEMORY_POOL_H
#define _CENG_CR_MEMORY_POOL_H

#include "crender-base.h"

namespace Ceng
{
	class CR_PoolBlock;

	class CR_MemoryPool
	{
	public:
		void *buffer;

		UINT32 alignment;
		UINT32 bufferSize;
		UINT32 blockSize;

		UINT32 *allocBitmap;

	public:
		CR_MemoryPool();
		~CR_MemoryPool();

		CR_MemoryPool* CreatePool(const UINT32 blockSize,const UINT32 blocks,const UINT32 alignment);

		CR_PoolBlock GetFreeBlock();
	};
};

#endif