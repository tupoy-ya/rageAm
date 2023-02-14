#pragma once
#include <vector>

#include "datAllocator.h"
#include "fwTypes.h"

namespace rage
{
	struct datAllocatorSortedChunk
	{
		struct PredicateDescending
		{
			bool operator()(datAllocatorSortedChunk const& a, datAllocatorSortedChunk const& b) const
			{
				return a.Size > b.Size;
			}
		};

		u32 Size;
		u16 Index;

		datAllocatorSortedChunk(u32 size, u16 index)
		{
			Size = size;
			Index = index;
		}
	};

	/**
	 * \brief Utility for encoding page data into 32 bits.
	 * \n Page data is stored for virtual / physical segments in resource header
	 * and parsed by datResourceInfo.
	 */
	class datPackInfo
	{
		u32 PageShift = 0;
		u8 MaxPageBit = 0;
		u8 PageCounts[9]{};
	public:
		void SetMaxPageSize(u64 pageSize);
		void AddPage(u64 pageSize);

		/**
		 * \brief Encodes virtual / physical data (also known as flags).
		 */
		u32 GetData() const;
	};

	/**
	 * \brief Contains single page with packed allocator chunk indices.
	 */
	struct datPackedPage
	{
		u64 Size;
		u32 Data;
		std::vector<u16> Indices;
	};

	// TODO: Packer doesn't take into account that we have only 9 page sizes.

	/**
	 * \brief Contains functions for optimally packing allocated chunks (datAllocatorChunk) in pages.
	 */
	class datPacker
	{
		typedef std::vector<datAllocatorSortedChunk> Chunks;
		typedef std::vector<u16> Indices;

		// 16 - 65 536 iterations. Takes about 16ms in worst scenario;
		// Raising to 20 for testing. On 25 it gets really slow.
		static constexpr u16 PACK_MAX_CHUNK_COUNT_FOR_COMBINATION = 20;

		// For recursive combination.

		Indices m_BestIndices;
		u64 m_BestIndicesSize = 0;

		// Sorted chunks (Descending by size).
		// Exception - main virtual chunk is always the first.
		Chunks m_Chunks;
		u64 m_TotalChunkSize = 0;
		u64 m_MaxChunkSize = 0;

		// Page size.
		u64 m_SizeLimit = 0;

		datAllocator* m_Allocator;

		u64 GetChunkSizeFromIndices(const Indices& indices) const;

		// Finds best chunks combination (with primary chunk) that satisfies size limit.
		void FindBestChunkCombination(Indices& indices, u16 index = 0);

		// Finds best chunks combination (with zero index / biggest chunk) that satisfies size limits.
		// Returns vector with indices of chunks that satisfy the limits; If none of limits were satisfied, empty vector.
		Indices FindBestChunkCombination();

		// Simply returns array with index of every chunk.
		Indices GetAllIndices() const;

		// The most 'naive' approach - add chunks until limit is reached.
		// Fast, but not so memory efficient.
		Indices FillChunks() const;

		// Selects best algorithm for packing based on input chunks and performs packing.
		Indices DoPack();

		// Fixup references on chunks (or translate them from RAM to file space).
		// NOTE: Indices must be in allocator space! Not sorted space.
		void FixupRefs(const Indices& indices, u64 fileOffset) const;

		// Transforms chunk indices from sorted to unsorted.
		void TransformIndices(const Indices& indices, Indices& dest) const;

		// Removes chunks after packing.
		void RemoveChunks(const Indices& indices);

		// Recalculates max and total chunk sizes.
		void UpdateChunkStats();

		// Copies chunks from allocator and sorts them.
		// (Except for main virtual chunk, it's always the first one)
		// We have to keep chunks sorted to pack biggest pages first,
		// because page sizes are computed from largest to smallest.
		// (see datResourceInfo::GenerateChunks);
		void CopyAndSortChunks();

		/**
		 * \brief Tries to pack chunks with primary (biggest chunk) to satisfy page size limit.
		 * \param outIndices Indices of grouped chunks; Empty array if there's no more chunks.
		 * \param pageSize Size of the page chunks were fit into.
		 * \return Whether any chunk was packed or not.
		 */
		bool PackNext(std::vector<u16>& outIndices, u64& pageSize);
	public:
		datPacker(datAllocator* allocator);

		/**
		 * \brief Performs batch packing of every chunk and fixing up refs.
		 * (Converting pointers from RAM to file space)
		 * \param outPage Indices and order of packed chunks, page size and encoded page data.
		 */
		void Pack(datPackedPage& outPage);
	};
}
