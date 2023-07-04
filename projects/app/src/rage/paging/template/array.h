//
// File: array.h
//
// Copyright (C) 2023 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include <new.h>

#include "rage/atl/array.h"
#include "rage/paging/compiler/compiler.h"
#include "rage/paging/place.h"
#include "rage/paging/resource.h"

namespace rage
{
	template<typename T, typename TSize>
	struct pgArrayAllocate
	{
		static void Allocate(T** ppItems, TSize capacity, TSize size)
		{
			pgSnapshotAllocator* pAllocator = pgRscCompiler::GetVirtualAllocator();
			if (!pAllocator) // We're doing regular allocator, resource compiler isn't active
			{
				atArrayAllocate<T, TSize>::Allocate(ppItems, capacity, size);
				return;
			}

			// Resource is being compiled...
			// Note that we used size here instead of capacity,
			// we don't want extra unused memory to be in resource.
			pAllocator->AllocateRefArray(*ppItems, size);
		}

		static void Free(T* pItems)
		{
			pgSnapshotAllocator* pAllocator = pgRscCompiler::GetVirtualAllocator();
			if (!pAllocator)
				atArrayAllocate<T, TSize>::Free(pItems);

			// We don't need to free memory in datAllocator because it will be destroyed after compilation is done
		}
	};

	/**
	 * \brief Array for paged resource.
	 * \remark For pointer, see rage::pgPtrArray;
	 */
	template<typename T>
	class pgArray : public atArray<T, u16, pgArrayAllocate<T, u16>>
	{
		using Array = atArray<T, u16, pgArrayAllocate<T, u16>>;
	public:
		using Array::atArray;

		pgArray(const datResource& rsc) : Array(rsc)
		{
			rsc.Fixup(this->m_Items);
		}

		IMPLEMENT_PLACE_INLINE(pgArray);
	};

	/**
	 * \brief Array of pointers to non pgBase structures.
	 */
	template <typename T>
	class pgPtrArray : public pgArray<T*>
	{
		using TArray = pgArray<T*>;
	public:
		using TArray::atArray;

		pgPtrArray(const datResource& rsc) : pgArray<T*>(rsc)
		{
			for (u16 i = 0; i < this->GetSize(); i++)
				rsc.Fixup(this->Get(i));
		}
	};

	/**
	 * \brief Array of pointers to pgBase structures, such as grcTexture.
	 */
	template <typename T>
	class pgRscArray : public pgArray<T*>
	{
		using TArray = pgArray<T*>;
	public:
		using TArray::pgArray;

		pgRscArray(const pgRscArray& other) : TArray(other)
		{
			pgSnapshotAllocator* pAllocator = pgRscCompiler::GetVirtualAllocator();
			if (!pAllocator)
				return;

			// atArray will copy pgRef but won't snapshot structure, we have to do it for every item manually
			for (int i = 0; i < TArray::GetSize(); i++)
			{
				auto& item = TArray::Get(i);

				// As usual, allocate memory for resource struct on virtual allocator
				// and copy structure memory via placement new and copy constructor
				pVoid where = pAllocator->Allocate(sizeof(T));
				item = new (where) T(*item); // Construct and replace old pointer

				pAllocator->AddRef(item);
			}
		}

		pgRscArray(const datResource& rsc) : TArray(rsc)
		{
			for (u16 i = 0; i < this->GetSize(); i++)
			{
				// In contrast with pgPtrArray we not only fixup address but also execute new-placement constructor
				rsc.Place(this->Get(i));
			}
		}
	};
}
