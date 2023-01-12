#pragma once
#include "datResourceChunk.h"
#include <cstddef>

namespace rage
{
	typedef u64 pgBase;
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
	};
	static_assert(sizeof(datResourceMap) == 0xC18);
	static_assert(offsetof(datResourceMap, Chunks) == 0x10);
}
