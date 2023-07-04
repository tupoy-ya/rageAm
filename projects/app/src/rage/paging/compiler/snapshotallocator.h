//
// File: snapshotallocator.h
//
// Copyright (C) 2023 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include "rage/atl/array.h"
#include "am/system/asserts.h"

#include <typeinfo>

namespace rage
{
	/**
	 * \brief Linear allocator for performing snapshot (via copy constructor) of paged resource.
	 */
	class pgSnapshotAllocator
	{
		struct Node
		{
			u32 Guard;
			u32 Size;
			atArray<void**> Refs;

			Node(u32 size);

			u32	CreateGuard() const;
			bool VerifyGuard() const;
			void AssertGuard() const;

			void FixupRefs(u64 newAddress) const;

			char* GetBlock() const;
			Node* GetNext() const;
		};

		bool m_IsVirtual;
		pVoid m_Heap;
		u32	m_Offset = 0;
		u32 m_HeapSize = 0;
		u16 m_NodeCount = 0;

		Node* GetNodeFromBlock(pVoid block) const;
		Node* GetNodeFromBlockIndex(u32 index) const;
		Node* GetRootNode() const;
	public:
		pgSnapshotAllocator(u32 size, bool isVirtual);
		~pgSnapshotAllocator();

		pVoid Allocate(u32 size);

		/**
		 * \brief After memory blocks are packed, we have to 'fixup' every reference on them.
		 * For example - m_Items field in pgArray will be set to new address (file offset).
		 *
		 * \param blockIndex	Index of memory block (relative to ::GetBlockSizes array indices)
		 * \param newAddress	New address that will be set to every reference on given memory block.
		 * \remarks See ::AddRef.
		 */
		void FixupBlockReferences(u16 blockIndex, u64 newAddress) const;

		/**
		 * \brief Gets base address for file offset (0x5... for virtual and 0x6... for physical allocator's)
		 */
		u64 GetBaseAddress() const;

		/**
		 * \brief Gets sizes of allocated memory blocks.
		 * \param outSizes Array where memory blocks sizes will be written to.
		 */
		void GetBlockSizes(atArray<u32>& outSizes) const;

		/**
		 * \brief Gets memory block address from block index (relative to ::GetBlockSizes array indices)
		 */
		pVoid GetBlock(u16 index) const;

		/**
		 * \brief Gets allocated memory block size from block index (relative to ::GetBlockSizes array indices)
		 */
		u32 GetBlockSize(u16 index) const;

		/**
		 * \brief Gets total number of allocated blocks.
		 */
		u16 GetBlockCount() const { return m_NodeCount; }

		/**
		 * \brief Gets whether this allocator contains virtual or physical data.
		 */
		bool IsVirtual() const;

		ConstString GetDebugName() const { return IsVirtual() ? "Virtual" : "Physical"; }

		/**
		 * \brief Adds reference to given block.
		 * \param block	Structure field.
		 * \remarks See ::FixupBlockReferences for more info.
		 */
		template<typename T>
		void AddRef(T*& block)
		{
			Node* node = GetNodeFromBlock((pVoid)block);

			AM_ASSERT(node, "SnapshotAllocator::AddRef() -> Block is not valid.");

			node->Refs.Add((void**)&block);

			AM_DEBUGF("SnapshotAllocator::AddRef<%s>() -> %#llx", typeid(T).name(), (u64)block);
		}

		/**
		 * \brief Wrapper for allocating field and adding reference automatically.
		 * \param block Field name to allocate.
		 * \param size Size to allocate, by default - sizeof. If array is needed, use AllocateRefArray.
		 */
		template<typename T>
		void AllocateRef(T*& block, u32 size = sizeof(T))
		{
			block = static_cast<T*>(Allocate(size));
			AddRef(block);
		}

		void AllocateRefString(ConstString& str)
		{
			// grcTexture holds static name (not atString) that we have to snapshot
			// This is quite retarded hack but it's rockstar that decided not to use atString for some reason...
			// TODO: Can we add new type instead? pgConstString

			u32 size = strlen(str) + 1; // Including null-terminator
			ConstString orig = str;
			AllocateRef(str, size);
			// We have to cast it back to char* in order to perform copying
			String::Copy((char*)str, size, orig); // NOLINT(clang-diagnostic-cast-qual)
		}

		/**
		 * \brief Wrapper for allocating array and adding reference automatically.
		 * \param tRef Field name to allocate.
		 * \param count Size of array.
		 * \param size Size of array element, by default - sizeof.
		 */
		template<typename T>
		void AllocateRefArray(T*& tRef, u32 count, u32 size = sizeof(T))
		{
			AllocateRef(tRef, size * count);
		}
	};
}
