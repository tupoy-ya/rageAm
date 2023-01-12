#pragma once
#include "datResourceMap.h"
#include "gmFunc.h"
#include "unionCast.h"

namespace rage
{
	static inline gm::gmFunc<void, datResourceMap*> gImpl_datResourceInfo_GenerateMap;

	struct datResourceInfo
	{
		u32 VirtualData;
		u32 PhysicalData;

		static u8 GetSizeShift(u32 data);
		static u8 GetNumChunks(u32 data);
		static u32 GenerateChunks(u32 data, u64 baseSize, datResourceMap* map, u32 startChunk, u64 baseAddr);

		u8 GetNumVirtualChunks() const;
		u8 GetNumPhysicalChunks() const;

		void GenerateMap(datResourceMap* map) const;
	};
}
