#include "datPacker.h"

#include <algorithm>

#include "helpers/format.h"
#include "paging/datHelper.h"
#include "Logger.h"
#include "helpers/bits.h"

// Conflicting Windows.h macro
#undef min
#undef max

// Conflicting winnt.h macro
#undef BitScanReverse64

void rage::datPackInfo::SetMaxPageSize(u64 pageSize)
{
	MaxPageBit = BitScanReverse64(pageSize);
	PageShift = GetPageShift(pageSize);
}

void rage::datPackInfo::AddPage(u64 pageSize)
{
	u8 index = MaxPageBit - BitScanReverse64(pageSize);
	PageCounts[index]++;
}

u32 rage::datPackInfo::GetData() const
{
	u32 data = 0;

	// Size shift
	data |= PageShift - 4 & 0xF;

	// Page counts
	data |= (PageCounts[0] & 0x1) << 4;
	data |= (PageCounts[1] & 0x3) << 5;
	data |= (PageCounts[2] & 0xF) << 7;
	data |= (PageCounts[3] & 0x3F) << 11;
	data |= (PageCounts[4] & 0x7F) << 17;
	data |= (PageCounts[5] & 0x1) << 24;
	data |= (PageCounts[6] & 0x1) << 25;
	data |= (PageCounts[7] & 0x1) << 26;
	data |= (PageCounts[8] & 0x1) << 27;

	return data;
}

u64 rage::datPacker::GetChunkSizeFromIndices(const Indices& indices) const
{
	u64 size = 0;
	for (u16 index : indices)
		size += m_Chunks[index].Size;
	return size;
}

void rage::datPacker::FindBestChunkCombination(Indices& indices, u16 index)
{
	// NOTE: THIS IS 2^N ALGORITHM! NOT FOR BIG ARRAYS.
	// Say we have chunks: (name - size)
	// A - 35
	// B - 25
	// C - 25
	// D - 20
	// We have limit of maximum size occupied by chunks - 100, which is constrained by primary chunk A.
	//  Constrained meaning: we have to choose page size as multiple of 2 starting from 0x2000,
	//  so for sake of example 100 is minimum page size for primary chunk size 35.
	// Let's enumerate all possible combinations: (total chunks size - their indices)
	// 8 possible combinations:
	// Size: 105       Indices: 0,1,2,3
	// Size: 85        Indices: 0,1,2
	// Size: 80        Indices: 0,1,3
	// Size: 70        Indices: 0,2,3
	// Size: 60        Indices: 0,1
	// Size: 50        Indices: 0,2
	// Size: 45        Indices: 0,3
	// Size: 25        Indices: 0
	// Given the limit of 100, best option is: 0,1,2 - 85

	indices.push_back(index); // Track previous indices of tree

	// IsValid if chunks meet the limits
	u64 chunksSize = GetChunkSizeFromIndices(indices);
	if (chunksSize <= m_SizeLimit && chunksSize > m_BestIndicesSize)
	{
		m_BestIndicesSize = chunksSize;
		m_BestIndices = indices;
	}

	index++; // Each iteration we move to the right in chunk array
	for (size_t i = index; i < m_Chunks.size(); i++)
	{
		// Note: indices array is copied every invocation because every tree path is unique
		Indices indicesCopy = indices;
		FindBestChunkCombination(indicesCopy, static_cast<u16>(i));
	}
}

rage::datPacker::Indices rage::datPacker::FindBestChunkCombination()
{
	m_BestIndicesSize = 0;

	Indices indices;
	FindBestChunkCombination(indices);

	if (m_BestIndicesSize != 0)
		return m_BestIndices;

	// Only possible if every chunk was larger than the limit.
	return {};
}

rage::datPacker::Indices rage::datPacker::GetAllIndices() const
{
	Indices indices;
	for (size_t i = 0; i < m_Chunks.size(); i++)
		indices.push_back(static_cast<u16>(i));
	return indices;
}

rage::datPacker::Indices rage::datPacker::FillChunks() const
{
	Indices indices;

	// Start with primary chunk
	u64 size = m_Chunks[0].Size;
	indices.push_back(0);

	for (size_t i = 1; i < m_Chunks.size(); i++)
	{
		const datAllocatorSortedChunk& chunk = m_Chunks[i];
		if (size + chunk.Size > m_SizeLimit)
			continue;

		indices.push_back(static_cast<u16>(i));
		size += chunk.Size;
	}
	AM_TRACEF("datPacker::FillChunks() -> Filled {} out of {}", FormatBytes(size), FormatBytes(m_SizeLimit));

	return indices;
}

rage::datPacker::Indices rage::datPacker::DoPack()
{
	// Case 1: Total size occupied by chunks satisfies size limit.
	if (m_TotalChunkSize <= m_SizeLimit)
	{
		AM_TRACE("datPacker::PackNext() -> Case 1");

		return GetAllIndices();
	}

	// Case 2: Few amount of chunks.
	// This algorithm is insanely slow (2^n complexity).
	// This is the most accurate way to pack chunks.
	// In 100% cases result gives least possible unused memory.
	if (m_Chunks.size() <= PACK_MAX_CHUNK_COUNT_FOR_COMBINATION)
	{
		AM_TRACE("datPacker::PackNext() -> Case 2");

		return FindBestChunkCombination();
	}

	// Case 3: Naive.
	// Checking all combinations gets really expensive very quick.
	// This leaves us last option: simply add indices until we ran out of free space.
	// It will never as good as combination approach, but much better than
	// leaving one chunk per page.
	AM_TRACE("datPacker::PackNext() -> Case 3");

	return FillChunks();
}

void rage::datPacker::FixupRefs(const Indices& indices, u64 fileOffset) const
{
	u64 pageOffset = 0;
	for (u16 index : indices)
	{
		AM_TRACEF("datPacker::FixupRefs() -> {}", index);

		u64 filePointer = fileOffset + pageOffset;
		filePointer |= m_Allocator->GetBaseAddress();

		m_Allocator->FixupRefs(index, filePointer);

		pageOffset += m_Allocator->GetChunkSize(index);
	}
}

void rage::datPacker::TransformIndices(const Indices& indices, Indices& dest) const
{
	dest.clear();
	dest.reserve(indices.size());

	for (u16 index : indices)
		dest.emplace_back(m_Chunks[index].Index);
}

void rage::datPacker::RemoveChunks(const Indices& indices)
{
	// Mark chunks as no longer needed
	for (u16 index : indices)
		m_Chunks[index] = datAllocatorSortedChunk(0, 0xFFFF);

	// Make new array without removed chunks
	Chunks newChunks;
	newChunks.reserve(m_Chunks.size());
	for (datAllocatorSortedChunk& chunk : m_Chunks)
	{
		if (chunk.Index == 0xFFFF)
			continue;
		newChunks.emplace_back(chunk);
	}
	m_Chunks = newChunks;

	UpdateChunkStats();
}

void rage::datPacker::UpdateChunkStats()
{
	m_TotalChunkSize = 0;
	m_MaxChunkSize = 0;

	for (const datAllocatorSortedChunk& chunk : m_Chunks)
	{
		u64 size = chunk.Size;

		m_TotalChunkSize += size;
		m_MaxChunkSize = std::max(m_MaxChunkSize, size);
	}
}

void rage::datPacker::CopyAndSortChunks()
{
	std::vector<datAllocatorChunk> chunks;
	m_Allocator->GetChunks(chunks);

	m_Chunks.reserve(chunks.size());
	for (size_t i = 0; i < chunks.size(); i++)
		m_Chunks.emplace_back(chunks[i].Size, i);

	UpdateChunkStats();

	// For virtual region we have to 'pin' first chunk.
	if (m_Allocator->IsVirtual())
	{
		// This will ruin packing efficiency slightly.
		std::ranges::sort(m_Chunks.begin() + 1, m_Chunks.end(),
			datAllocatorSortedChunk::PredicateDescending());
	}
	else
	{
		std::ranges::sort(m_Chunks,
			datAllocatorSortedChunk::PredicateDescending());
	}
}

bool rage::datPacker::PackNext(std::vector<u16>& outIndices, u64& pageSize)
{
	pageSize = 0;
	if (m_Chunks.empty())
		return false;

	AM_TRACEF("datPacker::PackNext() -> {} chunk(s) to pack.", m_Chunks.size());

	pageSize = GetPageSize(m_MaxChunkSize);
	m_SizeLimit = pageSize;

	AM_TRACEF("datPacker::PackNext() -> Page size {:X}  for primary chunk of size {:X}",
		m_SizeLimit, m_MaxChunkSize);

	Indices indices = DoPack();

	AM_TRACEF("datPacker::PackNext() -> Grouped {} chunk(s).", indices.size());
	AM_TRACEF("datPacker::PackNext() -> Memory use: {:.2f}%",
		static_cast<double>(GetChunkSizeFromIndices(indices)) / m_SizeLimit * 100.0);

	TransformIndices(indices, outIndices);
	RemoveChunks(indices);
	return true;
}

rage::datPacker::datPacker(datAllocator* allocator)
{
	m_Allocator = allocator;

	CopyAndSortChunks();

	AM_TRACEF("datPacker::() -> Init with {} {} chunks.",
		m_Chunks.size(), m_Allocator->IsVirtual() ? " virtual" : " physical");
}

void rage::datPacker::Pack(datPackedPage& outPage)
{
	datPackInfo info{};

	u64 maxPageSize = 0;
	u64 fileOffset = 0;
	u64 pageSize;

	std::vector<u16> indices;
	while (PackNext(indices, pageSize))
	{
		// Always the first one.
		if (maxPageSize == 0)
		{
			maxPageSize = pageSize;
			info.SetMaxPageSize(pageSize);
		}
		info.AddPage(pageSize);

		FixupRefs(indices, fileOffset);
		fileOffset += pageSize;

		outPage.Size = pageSize;
		for (u16 index : indices)
			outPage.Indices.push_back(index);
	}
	outPage.Data = info.GetData();

	AM_TRACEF("datPacker::Pack() -> Packed with max page size of {:X}", maxPageSize);
}
