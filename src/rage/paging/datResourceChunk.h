#pragma once
#include "dat.h"

namespace rage
{
	/**
	 * \brief Represents mapping between resource (file) offset and game memory address.
	 */
	struct datResourceChunk
	{
		/**
		 * \brief Resource chunk address (file offset).
		 */
		uint64_t SrcAddr;

		/**
		 * \brief Address of corresponding chunk allocated in heap.
		 */
		uint64_t DestAddr;

		/**
		 * \brief Size of chunk in bytes.
		 */
		uint64_t Size;

		/**
		 * \brief Gets offset that maps resource address to corresponding address allocated in game heap.
		 * \return Offset.
		 */
		uint64_t GetFixup() const
		{
			return DestAddr - SrcAddr;
		}
	};
	static_assert(sizeof(datResourceChunk) == 0x18);

	/**
	 * \brief Same as datResourceChunk but used in sorted list for quick binary search.
	 */
	struct datResourceSortedChunk
	{
		/**
		 * \brief Start address of this chunk.
		 */
		u64 Address;

		/**
		 * \brief The 8 most high bits store index of this chunk in datResourceMap.Chunks,
		 * the rest 56 bits represent chunk size.
		 */
		u64 IndexAndSize;

		datResourceSortedChunk()
		{
			Address = 0;
			IndexAndSize = 0;
		}

		datResourceSortedChunk(u64 address, u64 size, u8 index)
		{
			Address = address;
			IndexAndSize = (u64)index << DAT_CHUNK_INDEX_SHIFT | size;
		}

		/**
		 * \brief Gets index of corresponding chunk in datResourceMap.Chunks;
		 */
		u8 GetChunkIndex() const
		{
			// Map chunk index is stored in the highest byte
			return (u8)(IndexAndSize >> DAT_CHUNK_INDEX_SHIFT & DAT_CHUNK_INDEX_MASK);
		}

		/**
		 * \brief Gets size of this chunk in bytes.
		 */
		u64 GetSize() const
		{
			return IndexAndSize & DAT_CHUNK_SIZE_MASK;
		}

		/**
		 * \brief Gets the last address of this chunk.
		 * \remark End address is not considered as valid.
		 */
		u64 GetEndAddress() const
		{
			return Address + GetSize();
		}

		/**
		 * \brief Gets whether given address is within address range of this chunk.
		 * \param addr Address to check.
		 * \return True if address is within this chunk address range; otherwise False.
		 */
		u64 ContainsThisAddress(u64 addr) const
		{
			return addr >= Address && addr < GetEndAddress();
		}
	};
	static_assert(sizeof(datResourceSortedChunk) == 0x10);
}
