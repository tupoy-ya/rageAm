#pragma once

#include <vector>

#include "fwTypes.h"

namespace rage
{
	struct datAllocatorChunk
	{
		u32 Size;

		datAllocatorChunk(u32 size)
		{
			Size = size;
		}
	};

	/**
	 * \brief Linear allocator for re-allocating (via copy constructor) resource on 'file' space.
	 */
	class datAllocator
	{
		class Header
		{
			u32 m_Guard;
			u32 m_Size;
			std::vector<void**> m_Refs;

			u32 GetGuard() const;
		public:
			Header(u32 size);

			/**
			 * \brief Checks if this header contains valid information.
			 */
			bool IsValid() const;

			/**
			 * \brief Adds reference on this chunk.
			 * \param pRef Pointer of structure field.
			 */
			void AddRef(void** pRef);

			/**
			 * \brief Sets new address to all references on this chunk.
			 */
			void FixupRefs(u64 newAddress) const;

			/**
			 * \brief Gets address on memory chunk following to this header.
			 */
			void* GetBlock() const;

			/**
			 * \brief Gets pointer on next (if exists) header.
			 */
			Header* Next() const;

			/**
			 * \brief Gets size of memory chunk following to this header.
			 */
			u32 GetSize() const;
		};

		bool m_IsVirtual;
		void* m_Heap;
		u32	m_Offset;

		// Pointers to headers of allocated chunks.
		std::vector<Header*> m_Headers;

		void* GetHeapAt(u32 offset) const;

		// Casts heap address to 64 bit integer.
		u64 GetHeap64() const;

		// Gets header offset relative to heap address.
		u32 GetHeaderOffset(const Header* pHeader) const;

		// Gets pointer on header structure on given address.
		// If pointer is not valid (header not found), nullptr is returned.
		Header* GetHeaderFromAddress(void* addr) const;

		void* DoAllocate(u32 size, u32 align);
	public:
		datAllocator(u32 size, bool isVirtual);
		~datAllocator();

		/**
		 * \brief Allocates new chunk.
		 * \param size Size to allocate in bytes.
		 * \param align Required align of the pointer.
		 * \return Pointer to allocated chunk.
		 */
		void* Allocate(size_t size, size_t align = 16);

		/**
		 * \brief Makes allocator snapshot for packing chunks up.
		 * \param outChunks Vector in which all chunks will be written to.
		 */
		void GetChunks(std::vector<datAllocatorChunk>& outChunks) const;

		/**
		 * \brief Sets new address for all references on given chunk (see AddRef);
		 * \param chunkIndex Index of referenced chunk.
		 * \param newAddress New address to set.
		 */
		void FixupRefs(u16 chunkIndex, u64 newAddress) const;

		/**
		 * \brief Gets base pointer address for file offset (0x5... or 0x6...);
		 */
		u64 GetBaseAddress() const;

		/**
		 * \brief Gets pointer to allocated chunk data at given index.
		 */
		char* GetChunkData(u16 index) const;

		/**
		 * \brief Gets allocated chunk size at given index.
		 */
		u32 GetChunkSize(u16 index) const;

		/**
		 * \brief Gets whether this allocator is for virtual or physical segments of data.
		 */
		bool IsVirtual() const;

		/**
		 * \brief Marks field pointer allocated by this allocator to be later changed to file offset.
		 * \param pRef Pointer to structure field.
		 */
		template<typename T>
		void AddRef(T* pRef)
		{
			void* address = reinterpret_cast<void*>(*pRef);

			Header* pHeader = GetHeaderFromAddress(address);
			if (!pHeader)
				throw;

			pHeader->AddRef(reinterpret_cast<void**>(pRef));
		}
	};
}
