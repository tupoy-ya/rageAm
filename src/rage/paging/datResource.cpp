#include "datResource.h"

#include <algorithm>

#include "Logger.h"

rage::datResource::datResource(datResourceMap& map, const char* name)
{
	Map = &map;
	Name = name;
	ChunkCount = map.GetChunkCount();

	for (u8 i = 0; i < ChunkCount; i++)
	{
		const datResourceChunk& chunk = map.Chunks[i];

		SrcChunks[i] = datResourceSortedChunk(chunk.SrcAddr, chunk.Size, i);
		DestChunks[i] = datResourceSortedChunk(chunk.DestAddr, chunk.Size, i);
	}

	std::sort(SrcChunks, &SrcChunks[ChunkCount], SortByRegionAddress);
	std::sort(DestChunks, &DestChunks[ChunkCount], SortByRegionAddress);

	PreviousResource = TlsManager::GetResource();
	TlsManager::SetResource(this);
	// *TlsManager::Get<u32*>(TLS_INDEX_NUM_DATRESOURCES) += 1; -- Doesn't work
}

rage::datResource::~datResource()
{
	TlsManager::SetResource(PreviousResource);
	// *TlsManager::Get<u32*>(TLS_INDEX_NUM_DATRESOURCES) -= 1; -- Doesn't work
}

bool rage::datResource::SortByRegionAddress(const datResourceSortedChunk& lhs, const datResourceSortedChunk& rhs)
{
	return lhs.Address < rhs.Address;
}

rage::datResourceChunk* rage::datResource::GetChunk(const datResourceSortedChunk* sortedChunk) const
{
	return &Map->Chunks[sortedChunk->GetChunkIndex()];
}

const rage::datResourceSortedChunk* rage::datResource::Find(const datResourceSortedChunk* chunks, uintptr_t address) const
{
	const datResourceSortedChunk* pageIt = chunks;
	u8 index = ChunkCount;

	// Binary search to find page which range contains address
	while (index > 0)
	{
		u8 mid = index / 2;
		const datResourceSortedChunk& page = pageIt[mid];

		if (page.GetEndAddress() > address)
		{
			index = mid;
		}
		else
		{
			pageIt += mid + 1;
			index -= mid + 1;
		}
	}

	const datResourceSortedChunk* end = &chunks[ChunkCount];
	if (pageIt != end && pageIt->ContainsThisAddress(address))
		return pageIt;
	return nullptr;
}

bool rage::datResource::ContainsThisAddress(uintptr_t address) const
{
	return Find(DestChunks, address) != nullptr;
}

uint64_t rage::datResource::GetFixup(uintptr_t resourceOffset) const
{
	if (const datResourceSortedChunk* chunk = Find(SrcChunks, resourceOffset))
		return GetChunk(chunk)->GetFixup();

	// ERR_SYS_INVALIDRESOURCE_5
	AM_ERRF("rage::datResource::GetFixup() -> "
	        "Invalid fixup, address {:X} is neither virtual nor physical.", resourceOffset);
	return 0;
}
