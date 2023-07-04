#pragma once

#include "paging.h"
#include "resourcemap.h"
#include "common/types.h"
#include "common/logger.h"

namespace rage
{
	class pgBase;
	struct datResource;

	/**
	 * \brief Holds data for deserializing resource.
	 */
	struct datResource
	{
		datResourceMap* Map;								// Offset map
		datResource* PreviousResource;						// Previous resource from TLS
		const char* Name;									// Debug name, <unknown> in most of the cases
		datResourceSortedChunk SrcChunks[PG_MAX_CHUNKS];	// Chunks from file
		datResourceSortedChunk DestChunks[PG_MAX_CHUNKS];	// Chunks allocated on game heap
		u8 ChunkCount;										// Virtual + Physical

	private:
		static inline thread_local datResource* tl_CurrentResource = nullptr;
	public:
		datResource(datResourceMap& map, ConstString name = "<unknown>");
		~datResource();

		// Comparer for sorting chunks.
		static bool SortByRegionAddress(const datResourceSortedChunk& lhs, const datResourceSortedChunk& rhs);

		static datResource* GetCurrent() { return tl_CurrentResource; }

		// Gets corresponding resource chunk from it's sorted version.
		datResourceChunk* GetChunk(const datResourceSortedChunk* sortedChunk) const;

		/**
		 * \brief Searches for an address in given chunk array.
		 * \param chunks A pointer to chunk array (source or destination).
		 * \param address Address to find.
		 * \return A pointer to datResourceSortedChunk whose address range
		 * contains the given address, if any; otherwise, nullptr.
		 */
		const datResourceSortedChunk* Find(const datResourceSortedChunk* chunks, u64 address) const;

		/**
		 * \brief Checks whether resource contains given game heap address.
		 * \param address Game address to check.
		 * \return True if any game memory chunk address range contains address; otherwise False.
		 */
		bool ContainsThisAddress(u64 address) const;

		/**
		 * \brief Gets offset for resource offset that maps it into allocated chunk in game heap.
		 * \param resourceOffset Offset address of the resource.
		 * \return Offset that maps resource address into game address.
		 */
		u64 GetFixup(u64 resourceOffset) const;

		/**
		 * \brief Maps resource offset into allocated chunk in game heap.
		 * \n USAGE: Fixup(m_Name);
		 * \tparam T Type of resource struct field.
		 * \param pField Pointer to resource struct field.
		 */
		template<typename T>
		void Fixup(T& pField) const
		{
			if (!(u64)pField)
				return;

			/* Spam machine
			auto chunk = Find(SrcChunks, (u64)pField);
			AM_DEBUGF("datResource::Fixup(%llx) -> %llx in chunk (%u, base: %llx)",
				(u64)pField,
				(u64)pField + GetFixup((u64)pField),
				chunk->GetChunkIndex(),
				chunk->Address);
			*/

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
		void Construct() const
		{
			static_assert(std::is_base_of_v<pgBase, T>, "Resource type must be derived from pgBase.");

			T* t = (T*)Map->MainChunk;
			Construct(t);
		}
	};
}

/**
 * \brief Constructs resource from main page.
 * \n NOTE: datResource still has to be passed into resource constructor.
 */
inline void* operator new(size_t, const rage::datResource& rsc)
{
	return rsc.Map->MainChunk;
}
inline void operator delete(void*, const rage::datResource&) {}
