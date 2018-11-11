/*****************************************************************************
*
* cr-pool-block.h
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#ifndef _CENG_CR_POOL_BLOCK_H
#define _CENG_CR_POOL_BLOCK_H

#include "crender-base.h"

#include "cr-memory-pool.h"

namespace Ceng
{
	class CR_PoolBlock
	{
	public:
		void *baseAddress;

		UINT32 alignment;
		UINT32 sizeBytes;

		CR_MemoryPool *sourcePool;
		INT32 poolID;

	public:

		CR_PoolBlock();
		~CR_PoolBlock();

		void Release();

	};
};

#endif