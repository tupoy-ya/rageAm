#pragma once

#include "datResource.h"
#include "pgBase.h"
#include "pgArray.h"
#include "pgRscArray.h"
#include "place.h"

namespace rage
{
	template<typename T>
	struct pgKeyPair
	{
		u32 Key;
		T* Value;
	};

	/**
	 * \brief Dictionary with key and pointer value. Used in resources.
	 * \tparam T Pointer type to store. For e.g. grcTexture;
	 */
	template<typename T>
	class pgDictionary : public pgBase
	{
		typedef T* PT;

		uint64_t qword10;
		uint32_t dword18;
		uint32_t gap1C;
		pgArray<u32> m_Keys;
		pgRscArray<T> m_Items;

	public:
		pgDictionary() = default;

		pgDictionary(u16 capacity) : m_Keys(capacity), m_Items(capacity) {}

		pgDictionary(const datResource& rsc) : m_Keys(rsc), m_Items(rsc)
		{

		}

		pgDictionary(const pgDictionary& other)
		{
			m_Keys = other.m_Keys;
			m_Items = other.m_Items;
		}

		const pgArray<u32>& GetKeys() const { return m_Keys; }
		const pgArray<u32>& GetItems() const { return m_Items; }

		/**
		 * \brief Gets number of used slots.
		 */
		int GetCount() const { return m_Items.GetSize(); }

		/**
		 * \brief Checks whether hash key present in dictionary or not.
		 */
		bool Contains(u32 key) { return m_Keys.Find(key); }

		/**
		 * \brief Gets pair of key and value at given index.
		 * \throws std::invalid_argument If given index was negative or greater than size of the dictionary.
		 */
		pgKeyPair<T> GetSlot(u16 index)
		{
			if (index < 0 || index == m_Keys.GetSize())
				throw std::invalid_argument("pgDictionary::GetSlot() -> Index was out of range.");

			return { m_Keys[index], m_Items[index] };
		}

		/**
		 * \brief Searches for entry by hash key.
		 * \return Pointer reference to found item.
		 * \throws std::invalid_argument If key is not present in dictionary.
		 */
		PT& Find(u32 key)
		{
			atIterator<u32> slot = m_Keys.Find(key);
			if (!slot)
				throw std::invalid_argument("pgDictionary::Find() -> Key was not found.");

			u16 index = m_Keys.IndexOf(slot);
			return m_Items[index];
		}

		/**
		 * \brief Alternative for Find(), but instead of throwing exception if key is not present, returns -1;
		 * \return Item index if found, otherwise -1.
		 */
		u16 FindIndex(u32 key)
		{
			return m_Keys.IndexOf(m_Keys.Find(key));
		}

		/**
		 * \brief Gets item at given index.
		 * \return Pointer reference to item.
		 * \throws std::invalid_argument If given index was negative or greater than size of the dictionary.
		 */
		PT& GetAt(u16 index)
		{
			if (index < 0 || index == m_Keys.GetSize())
				throw std::invalid_argument("pgDictionary::GetAt() -> Index was out of range.");

			return m_Items[index];
		}

		/**
		 * \brief If value is not present in dictionary, adds new entry;
		 * Otherwise replaces existing value.
		 * \return Reference on value.
		 */
		PT& Add(u32 key, PT item)
		{
			u16 index = FindIndex(key);
			if (index != -1)
				return m_Items[index] = item;

			m_Keys.Add(key);
			return m_Items.Add(item);
		}

		/**
		 * \brief Sorts keys and items.
		 * \n Use after adding new items in dictionary if search is required.
		 */
		void Sort()
		{
			// Sort both arrays by keys

			u16 size = m_Keys.GetSize();

			atArray oldItems = m_Items;
			atArray oldKeys = m_Keys;

			m_Keys.Sort();
			for (u16 i = 0; i < size; i++)
			{
				u16 newIndex = m_Keys.IndexOf(m_Keys.Find(oldKeys[i]));
				m_Items[newIndex] = oldItems[i];
			}
		}

		/**
		 * \brief Gets existing item if exists, otherwise creates new.
		 * \returns Pointer reference to item.
		 * \remaks If key was not present and returned value was not assigned,
		 * entry will remain as null pointer.
		 */
		[[nodiscard]] PT& operator [](u32 key)
		{
			u16 index = FindIndex(key);
			if (index != -1)
				return m_Items[index];
			return Add(key, nullptr);
		}

		IMPLEMENT_PLACE_INLINE(pgDictionary);
	};
	static_assert(sizeof(pgDictionary<LPVOID>) == 0x40);
}
