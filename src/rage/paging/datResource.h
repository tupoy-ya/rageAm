#pragma once

#include <new>

#include "dat.h"
#include "datResourceChunk.h"
#include "datResourceMap.h"
#include "fwTypes.h"
#include "TlsManager.h"

#ifndef RAGE_STANDALONE
#include "gmFunc.h"
#include "Logger.h"
#endif

namespace rage
{
	class pgBase;

	/**
	 * \brief Provides functionality for building resource from allocated chunks.
	 */
	struct datResource
	{
		/**
		 * \brief Offsets map of this resource.
		 */
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

		/**
		 * \brief Physical and Virtual chunks combined.
		 */
		uint8_t ChunkCount;

		datResource(datResourceMap& map, const char* name = "<unknown>");
		~datResource();

		// Comparer for sorting chunks.
		static bool SortByRegionAddress(const datResourceSortedChunk& lhs, const datResourceSortedChunk& rhs);

		/**
		 * \brief Gets corresponding resource chunk from it's sorted version.
		 * \param sortedChunk Sorted chunk to get datResourceChunk for.
		 */
		datResourceChunk* GetChunk(const datResourceSortedChunk* sortedChunk) const;

		/**
		 * \brief Searches for an address in given chunk array.
		 * \param chunks A pointer to chunk array (source or destination).
		 * \param address Address to find.
		 * \return A pointer to datResourceSortedChunk whose address range
		 * contains the given address, if any; otherwise, nullptr.
		 */
		const datResourceSortedChunk* Find(const datResourceSortedChunk* chunks, uintptr_t address) const;

		/**
		 * \brief Checks whether resource contains given game heap address.
		 * \param address Game address to check.
		 * \return True if any game memory chunk address range contains address; otherwise False.
		 */
		bool ContainsThisAddress(uintptr_t address) const;

		/**
		 * \brief Gets offset for resource offset that maps it into allocated chunk in game heap.
		 * \param resourceOffset Offset address of the resource.
		 * \return Offset that maps resource address into game address.
		 */
		uint64_t GetFixup(uintptr_t resourceOffset) const;

		/**
		 * \brief Maps resource offset into allocated chunk in game heap.
		 * \n USAGE: Fixup(&m_Name);
		 * \tparam T Type of resource struct field.
		 * \param pField Pointer to resource struct field.
		 */
		template<typename T>
		void Fixup(T& pField) const
		{
			pField = (T)((u64)pField + GetFixup((u64)pField));
		}

		/**
		 * \brief Fixes address and performs new placement for resource struct.
		 * \n Used for collection elements for e.g. in pgPtrArray.
		 * \tparam T Type of structure to place.
		 * \param paged Pointer to structure.
		 */
		template<typename T>
		void Place(T& paged) const
		{
			Fixup(paged);
			Construct(paged);
		}

		/**
		 * \brief Performs new placement (datResource constructor) for resource struct.
		 * \tparam T Type of structure to construct.
		 * \param pPaged Pointer to structure.
		 */
		template<typename T>
		void Construct(T* pPaged) const
		{
			// Important thing to note here is that with manual new-placing no destructor
			// will be invoked, and that's exactly what we need because it has to be managed
			// by streaming allocator.
			// Otherwise for e.g. destructor of atArray will blow up whole operation.

			const datResource& rsc = *this;
			void* addr = (void*)pPaged;
			new (addr) T(rsc);
		}

		/**
		 * \brief Performs new placement of main resource page.
		 * \n Alternative way is to use overloaded operator new: new (rsc) pgDictionary<grcTexture>(rsc);
		 * \tparam T Resource type.
		 * \return Pointer to resource instance.
		 */
		template<typename T>
		T* Construct() const
		{
			static_assert(std::is_base_of_v<pgBase, T>, "Resource type must be inherited from pgBase.");

			T* t = (T*)Map->MainPage;
			Construct(t);
			return t;
		}
	};
	// Size is 0x101A, + 6 byte pad at the end
	static_assert(sizeof(datResource) == 0x101A + 6);

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
#endif
	}
}

// Constructs resource from main page.
// NOTE: datResource still has to be passed into constructor.
inline void* operator new(size_t, const rage::datResource& rsc)
{
	return rsc.Map->MainPage;
}
inline void operator delete(void*, const rage::datResource&) {}
