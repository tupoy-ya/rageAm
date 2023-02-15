#include "datResourceInfo.h"

u8 rage::datResourceInfo::GetSizeShift(u32 data)
{
	return (data & 0xF) + 4;
}

u8 rage::datResourceInfo::GetNumChunks(u32 data)
{
	return (u8)(
		(data >> 4 & 0x1) +
		(data >> 5 & 0x3) +
		(data >> 7 & 0xF) +
		(data >> 11 & 0x3F) +
		(data >> 17 & 0x7F) +
		(data >> 24 & 0x1) +
		(data >> 25 & 0x1) +
		(data >> 26 & 0x1) +
		(data >> 27 & 0x1));
}

u8 rage::datResourceInfo::GenerateChunks(u32 data, u64 baseSize, datResourceMap& map, u8 startChunk, u64 baseAddr)
{
	u8 sizeShift = GetSizeShift(data);
	u64 size = baseSize << sizeShift;
	u8 chunkIndex = startChunk;

	/*
	 * Page sizes if size shift (exponent) is 4 (0x2000 * 2^4 = 0x20000):
	 * 0x20000
	 * 0x10000
	 * 0x8000
	 * 0x4000
	 * 0x2000
	 * 0x1000
	 * 0x800
	 * 0x400
	 * 0x200
	 *
	 * Total amount of chunks in all pages has to be less or equal 128
	 * (hardcoded limit, see dat.h)
	 */
	u32 pageChunks[9]
	{
		data >> 4 & 0x1, // 0x10 & 0x1
		data >> 5 & 0x3,
		data >> 7 & 0xF,
		data >> 11 & 0x3F,
		data >> 17 & 0x7F,
		data >> 24 & 0x1,	// 0x100'0000 & 0x1
		data >> 25 & 0x1,	// 0x200'0000 & 0x1
		data >> 26 & 0x1,	// 0x400'0000 & 0x1
		data >> 27 & 0x1,	// 0x800'0000 & 0x1
	};

	for (u32 numChunks : pageChunks)
	{
		for (u32 j = 0; j < numChunks; j++)
		{
			map.Chunks[chunkIndex].DestAddr = 0;
			map.Chunks[chunkIndex].SrcAddr = baseAddr;
			map.Chunks[chunkIndex].Size = size;
			baseAddr += size;
			chunkIndex++;
		}
		size >>= 1; // Pages are ordered descending by size
	}

	return chunkIndex;
}

u8 rage::datResourceInfo::GetNumVirtualChunks() const
{
	return GetNumChunks(VirtualData);
}

u8 rage::datResourceInfo::GetNumPhysicalChunks() const
{
	return GetNumChunks(PhysicalData);
}

void rage::datResourceInfo::GenerateMap(datResourceMap& map) const
{
	map.VirtualChunkCount = GetNumVirtualChunks();
	map.PhysicalChunkCount = GetNumPhysicalChunks();

	map.MainChunkIndex = 0;
	map.MainPage = nullptr;
	map.qwordC10 = 0;

	u8 vChunks = GenerateChunks(VirtualData, DAT_MIN_CHUNK_SIZE, map, 0, DAT_VIRTUAL_BASE);
	GenerateChunks(PhysicalData, DAT_MIN_CHUNK_SIZE, map, vChunks, DAT_PHYSICAL_BASE);
}
