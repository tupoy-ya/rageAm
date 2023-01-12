#pragma once
#include "../fwTypes.h"
#include "../TlsManager.h"
#include "Logger.h"
#include "unionCast.h"

#include <algorithm>

#include "gmFunc.h"
#include "dat.h"
#include "datResourceInfo.h"
#include "datResourceMap.h"
#include "datResourceChunk.h"

namespace rage
{
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
			NumChunks = map->VirtualChunkCount + map->PhysicalChunkCount;

			for (u8 i = 0; i < NumChunks; i++)
			{
				const datResourceChunk& chunk = map->Chunks[i];

				SrcChunks[i] = datResourceSortedChunk(chunk.SrcAddr, chunk.Size, i);
				DestChunks[i] = datResourceSortedChunk(chunk.DestAddr, chunk.Size, i);
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
		//static inline gm::gmFuncSwap gSwap_datResource_ContainsThisAddress(
		//	"rage::datResource::ContainsThisAddress",
		//	"48 89 5C 24 08 48 89 7C 24 10 0F B6 81",
		//	gm::CastAny(&datResource::ContainsThisAddress));

		//static inline gm::gmFuncSwap gSwap_datResource_GetFixup(
		//	"rage::datResource::GetFixup",
		//	"48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 0F B6 81",
		//	gm::CastAny(&datResource::GetFixup));

		//static inline gm::gmFuncHook gSwap_datResourceInfo_GenerateMap(
		//	"rage::datResourceInfo::GenerateMap",
		//	"48 89 5C 24 08 57 48 83 EC 30 44 8B 09",
		//	gm::CastAny(&datResourceInfo::GenerateMap),
		//	gImpl_datResourceInfo_GenerateMap.GetAddressPtr());
#endif
	}
}
