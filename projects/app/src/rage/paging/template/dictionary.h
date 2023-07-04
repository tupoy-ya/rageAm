//
// File: dictionary.h
//
// Copyright (C) 2023 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include "rage/crypto/joaat.h"
#include "rage/paging/template/array.h"
#include "rage/paging/resource.h"
#include "rage/paging/base.h"
#include "rage/paging/place.h"
#include "rage/paging/ref.h"

namespace rage
{
	template<typename T>
	struct pgKeyPair
	{
		u32	Key;
		T* Value;
	};

	/**
	 * \brief Dictionary with hash keys.
	 */
	template<typename T>
	class pgDictionary : public pgBase
	{
		using TPtr = T*;

		// For pgTextureDictionary - parent from CTxdRelationship
		pgRef<pgDictionary> m_Parent = nullptr;

		u32 m_RefCount;
		u32 m_Unused1C = 0;

		pgArray<u32>	m_Keys;
		pgRscArray<T>	m_Items;

		void Sort()
		{
			u16 size = m_Keys.GetSize();

			atArray oldItems = m_Items;
			atArray oldKeys = m_Keys;

			m_Keys.Sort();
			for (u16 i = 0; i < size; i++)
			{
				u16 newIndex = m_Keys.Find(oldKeys[i]);
				m_Items[newIndex] = std::move(oldItems[i]);
			}
		}
	public:
		pgDictionary(u16 size) : m_Keys(size), m_Items(size)
		{
			m_RefCount = 1;
		}
		pgDictionary()
		{
			m_RefCount = 1;
		}
		pgDictionary(const pgDictionary& other)
			: pgBase(other), m_Keys(other.m_Keys), m_Items(other.m_Items)
		{
			m_RefCount = 1;
		}
		pgDictionary(pgDictionary&& other) noexcept : pgDictionary(0)
		{
			std::swap(m_RefCount, other.m_RefCount);
			std::swap(m_Keys, other.m_Keys);
			std::swap(m_Items, other.m_Items);
		}

		// ReSharper disable once CppPossiblyUninitializedMember
		pgDictionary(const datResource& rsc) : m_Keys(rsc), m_Items(rsc)
		{
			// Nothing to do here, everything is resolved by pgArray
		}

		~pgDictionary() override
		{
			// For compiled resources everything is handled by pgBase::Destroy, it rebuilds map
			// and de-allocates all chunks that belong to resource, including chunk that contains pgBase.
			// But in game there's no code to handle dynamic / stack allocations
			// because all resources are compiled and code for that present only in debug builds.

			sysMemAllocator* multi = GetMultiAllocator();
			for (int i = 0; i < m_Items.GetSize(); i++)
			{
				auto& item = m_Items[i];

				item->~T();

				sysMemAllocator* owner = multi->GetPointerOwner(item);
				if (owner)
					owner->Free(item);
			}
		}

		pgRef<pgDictionary>& GetParent() { return m_Parent; }
		void SetParent(const pgRef<pgDictionary>& newParent) { m_Parent = newParent; }

		const pgArray<u32>& GetKeys() const { return m_Keys; }
		const pgArray<u32>& GetItems() const { return m_Items; }

		int  GetSize() const { return m_Items.GetSize(); }
		void Clear() { m_Items.Clear(); m_Keys.Clear(); }
		bool ContainsKey(u32 key) const { return m_Keys.Find(key); }
		bool Find(u32 key, TPtr& outItem) const
		{
			s32 slot = m_Keys.Find(key);
			if (slot == -1) return false;

			outItem = m_Items[slot];
			return true;
		}

		pgKeyPair<T> GetAt(u16 index) { return { m_Keys[index], m_Items[index] }; }
		TPtr& GetValueAt(u16 index) { return m_Items[index]; }

		/**
		 * \brief Gets index of item by hash key.
		 * \return Index if item present in dictionary; Otherwise -1;
		 */
		s32 GetIndexOf(u32 key) const
		{
			return m_Keys.Find(key);
		}

		s32 GetIndexOf(ConstString key) const { return GetIndexOf(joaat(key)); }

		template<typename ...Args>
		TPtr& Construct(ConstString key, Args&&... args)
		{
			return Construct(joaat(key), std::forward<Args>(args)...);
		}

		template<typename ...Args>
		TPtr& Construct(u32 key, Args&&... args)
		{
			TPtr newItem;

			s32 index = GetIndexOf(key);
			if (index != -1)
			{
				m_Items[index]->~T();
				newItem = m_Items[index] = new T(std::forward<Args>(args)...);
			}
			else
			{
				m_Keys.Add(key);
				newItem = m_Items.Construct(new T(args...));
			}

			// Binary search requires array to be sorted
			// TODO: This can be improved by 'inserting' new item without messing with order
			Sort();

			return newItem;
		}

		/**
		 * \brief If item with given key is not present in dictionary, item is inserted;
		 * Otherwise existing item is destructed and replaced.
		 */
		TPtr& Add(u32 key, TPtr item)
		{
			TPtr* newItem;

			s32 index = GetIndexOf(key);
			if (index != -1)
			{
				m_Items[index]->~T();
				newItem = &m_Items[index] = item;
			}
			else
			{
				m_Keys.Add(key);
				newItem = &m_Items.Add(item);
			}

			// We have to sort it now, otherwise Find will be broken.
			// TODO: This can be improved by 'inserting' new item without messing with order
			Sort();

			return *newItem;
		}

		TPtr& Add(ConstString key, TPtr item)
		{
			return Add(joaat(key), item);
		}

		/**
		 * \brief Gets existing item if exists, otherwise creates new.
		 * \returns Reference to item pointer.
		 * \remarks If key was not present and returned value was not assigned, entry will remain as null pointer.
		 */
		[[nodiscard]] TPtr& operator [](u32 key)
		{
			s32 index = GetIndexOf(key);
			if (index != -1) return m_Items[index];
			return Add(key, nullptr);
		}

		[[nodiscard]] TPtr& operator [](ConstString key)
		{
			return operator[](joaat(key));
		}

		IMPLEMENT_PLACE_INLINE(pgDictionary);
		IMPLEMENT_REF_COUNTER(pgDictionary);
	};
	static_assert(sizeof(pgDictionary<u32>) == 0x40);
}
