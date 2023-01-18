#pragma once
#include "datResourceChunk.h"
#include "pgBase.h"
#include <stddef.h>

namespace rage
{
	class pgBase;

	struct datResourceMap
	{
		u8 VirtualChunkCount;
		u8 PhysicalChunkCount;
		u8 MainChunkIndex;
		u8 byte3;
		u32 dword4;
		pgBase* MainPage;
		datResourceChunk Chunks[DAT_NUM_CHUNKS];
		u64 qwordC10;

		u8 GetChunkCount() const
		{
			return VirtualChunkCount + PhysicalChunkCount;
		}
	};
	static_assert(sizeof(datResourceMap) == 0xC18);
	static_assert(offsetof(datResourceMap, Chunks) == 0x10);
}
