#pragma once
#include "memory/unionCast.h"

namespace rage
{
	/*
	 * Before exploring code below, make sure to understand virtual & physical memory management in operating systems.
	 * https://stackoverflow.com/questions/14347206/what-are-the-differences-between-virtual-memory-and-physical-memory
	 *
	 * Resource: #ft, #dr, #td, #cd etc.
	 * Short-term: datResource provides functionality to map resource addresses (or file offsets) to allocated chunks in game heap.
	 * All resource addresses are split in two types: virtual and physical (in CodeWalker they're called system and graphical),
	 * The only difference is that physical addresses point to data that will be allocated GPU (for i.e. textures)
	 * and will be removed from game memory as soon D3D object was created.
	 */

	static constexpr u8 DAT_NUM_CHUNKS = 128;

	static constexpr u64 DAT_CHUNK_SIZE_MASK = 0x00FF'FFFF'FFFF'FFFF;
	static constexpr u64 DAT_CHUNK_INDEX_SHIFT = 56; // 8 highest bits
	static constexpr u64 DAT_CHUNK_INDEX_MASK = 0xFF;

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
			IndexAndSize = index << DAT_CHUNK_INDEX_SHIFT & size;
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

	// one of them - VirtualCount
	struct datResourceMap
	{
		int8_t byte0;
		int8_t byte1;
		int8_t byte2;
		int8_t byte3;
		int32_t dword4;
		int64_t ResourceData;
		datResourceChunk Chunks[DAT_NUM_CHUNKS];
		int64_t qwordC10;
	};
	static_assert(sizeof(datResourceMap) == 0xC18);
	static_assert(offsetof(datResourceMap, Chunks) == 0x10);

	static bool SortByRegionAddress(const datResourceSortedChunk& lhs, const datResourceSortedChunk& rhs)
	{
		return lhs.Address < rhs.Address;
	}

	struct datResource
	{
		datResourceMap* Map;

		/**
		 * \brief Pointer to previous datResource instance that was stored in thread local storage.
		 */
		datResource* PreviousResource;

		/**
		 * \brief Purpose of this name is unclear, most of the time its '<unknown>',
		 * but once appears as 'Viseme animation data'.
		 */
		const char* Name;

		/**
		 * \brief Chunks of resource file.
		 */
		datResourceSortedChunk SrcChunks[DAT_NUM_CHUNKS];

		/**
		 * \brief Chunks allocated on heap.
		 */
		datResourceSortedChunk DestChunks[DAT_NUM_CHUNKS];

		uint8_t NumChunks;
		uint8_t byte1019;

		datResource(datResourceMap* map, const char* name = "<unknown>")
		{
			Map = map;
			Name = name;
			byte1019 = 0;
			NumChunks = map->byte0 + map->byte1;

			bool v10 = map->byte0 + map->byte1 == 0;
			if (!v10)
			{
				for (u8 i = 0; i < NumChunks; i++)
				{
					const datResourceChunk& chunk = map->Chunks[i];

					SrcChunks[i] = datResourceSortedChunk(chunk.SrcAddr, chunk.Size, i);
					DestChunks[i] = datResourceSortedChunk(chunk.DestAddr, chunk.Size, i);
				}
			}
			std::sort(SrcChunks, &SrcChunks[NumChunks], SortByRegionAddress);
			std::sort(DestChunks, &DestChunks[NumChunks], SortByRegionAddress);

			PreviousResource = TlsManager::GetResource();
			TlsManager::SetResource(this);
			*TlsManager::Get<u32*>(TLS_INDEX_NUM_DATRESOURCES) += 1;
		}

		~datResource()
		{
			TlsManager::SetResource(PreviousResource);
			*TlsManager::Get<u32*>(TLS_INDEX_NUM_DATRESOURCES) -= 1;
		}

		/**
		 * \brief Gets corresponding resource chunk from it's sorted version.
		 * \param sortedChunk Sorted chunk to get datResourceChunk for.
		 */
		datResourceChunk* GetChunk(const datResourceSortedChunk* sortedChunk) const
		{
			return &Map->Chunks[sortedChunk->GetChunkIndex()];
		}

		/**
		 * \brief Searches for an address in given chunk array.
		 * \param chunks A pointer to chunk array (source or destination).
		 * \param address Address to find.
		 * \return A pointer to datResourceSortedChunk whose address range
		 * contains the given address, if any; otherwise, nullptr.
		 */
		datResourceSortedChunk* Find(datResourceSortedChunk* chunks, uintptr_t address) const
		{
			datResourceSortedChunk* pageIt = chunks;
			u8 index = NumChunks;

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

			datResourceSortedChunk* lastPage = &chunks[NumChunks];
			if (pageIt != lastPage & pageIt->ContainsThisAddress(address))
				return pageIt;
			return nullptr;
		}

		/**
		 * \brief Checks whether resource contains given game heap address.
		 * \param address Game address to check.
		 * \return True if any game memory chunk address range contains address; otherwise False.
		 */
		bool ContainsThisAddress(uintptr_t address)
		{
			return Find(DestChunks, address) != nullptr;
		}

		/**
		 * \brief Gets offset for resource offset that maps it into allocated chunk in game heap.
		 * \param resourceOffset Offset address of the resource.
		 * \return Offset that maps resource address into game address.
		 */
		uint64_t GetFixup(uintptr_t resourceOffset)
		{
			if (datResourceSortedChunk* chunk = Find(SrcChunks, resourceOffset))
				return GetChunk(chunk)->GetFixup();

			AM_ERRF("rage::datResource::GetFixup() -> ERR_SYS_INVALIDRESOURCE_5"
				": Invalid fixup, address {:X} is neither virtual nor physical.", resourceOffset);
			return 0;
		}

		template<typename TPaged>
		static void Place(TPaged* ppPaged)
		{
			datResource* rsc = TlsManager::GetResource();

			if (!rsc || rsc->ContainsThisAddress((uintptr_t)ppPaged))
			{
				*ppPaged = nullptr;
				return;
			}

			ppPaged += rsc->GetFixup((uintptr_t)ppPaged);
			*ppPaged = new (sizeof(TPaged), *ppPaged)TPaged(rsc);
		}
	}; // Size is 0x101A, + 6 byte align at the end
	static_assert(sizeof(datResource) == 0x101A + 0x6);

	/*
	class pgBase
	{
		uint64_t vftable;
		uint64_t qword8;
	protected:
		static datResource* GetResource() { return TlsManager::GetResource(); }
	public:
		pgBase()
		{
			if (qword8 == 0)
				return;

			datResource* rsc = GetResource();
			if (!rsc)
				return;

			qword8 += rsc->GetFixup(qword8);
		}
	};
	static_assert(sizeof(pgBase) == 0x10);
	*/

	namespace hooks
	{
#ifdef RAGE_HOOK_SWAP_DATRESOURCE
		static inline gm::gmFuncSwap gSwap_datResource_ContainsThisAddress(
			"rage::datResource::ContainsThisAddress",
			"48 89 5C 24 08 48 89 7C 24 10 0F B6 81",
			gm::CastLPVOID(&datResource::ContainsThisAddress));

		static inline gm::gmFuncSwap gSwap_datResource_GetFixup(
			"rage::datResource::GetFixup",
			"48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 0F B6 81",
			gm::CastLPVOID(&datResource::GetFixup));
#endif RAGE_HOOK_SWAP_DATRESOURCE
	}
}
