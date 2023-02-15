#pragma once

#include "fwTypes.h"
#include "iterator.h"
#include "paging/place.h"
#include "paging/datResource.h"

#include "algorithm"
#include <stdexcept>

// Undef conflicting Windows.h macro
#undef min
#undef max

namespace rage
{
	/**
	 * \brief Simple array with dynamic memory allocation.
	 * \tparam T Type to store.
	 */
	template<typename T>
	class atArray
	{
		static constexpr u16 AT_ARRAY_GROW_SIZE = 10;
		static constexpr u16 AT_ARRAY_DEFAULT_CAPACITY = 10;

		void DoAlloc()
		{
			datAllocator* pAllocator = TlsManager::GetVirtualAllocator();
			if (!pAllocator)
			{
				m_Items = new T[m_Capacity];
				return;
			}

			// Allocate on virtual stream if we're compiling resource.
			// We have to do that manually because new operator overriden only for resources. (place macro)
			// (because compiler have to allocate memory and it would allocate on virtual stream otherwise)
			pAllocator->AllocateRefArray(m_Items, m_Capacity);
		}
	protected:
		T* m_Items;
		u16 m_Size;
		u16 m_Capacity;

	public:
		/**
		 * \brief Constructs new array and allocates memory.
		 * \param capacity Default capacity (pre-allocated array size).
		 */
		atArray(u16 capacity)
		{
			m_Capacity = capacity;
			m_Size = 0;
			m_Items = nullptr;
			Resize(capacity);
		}

		/**
		 * \brief Constructs new array and allocates memory with default capacity.
		 */
		atArray() : atArray(AT_ARRAY_DEFAULT_CAPACITY) {}

		/**
		 * \brief Constructs array from std vector.
		 * \param vec Vector to copy elements from.
		 */
		atArray(const std::vector<T>& vec) : atArray(vec.capacity())
		{
			for (const T& item : vec)
				Add(item);
		}

		atArray(const datResource& rsc)
		{
			if (m_Items) rsc.Fixup(m_Items);
		}

		atArray(const atArray& other)
			: atArray(other.GetSize())
		{
			m_Size = other.m_Size;
			m_Capacity = other.m_Capacity;
			memcpy(m_Items, other.m_Items, m_Size * sizeof(T));
		}

		~atArray()
		{
			Free();
		}

		u16 GetSize() const { return m_Size; }
		u16 GetCapacity() const { return m_Capacity; }

		/**
		 * \brief Adds new item in the end of array.
		 * \n If there's not enough size, array will be resized.
		 */
		void Add(const T& item)
		{
			if (m_Size == m_Capacity)
				Resize(m_Capacity + AT_ARRAY_GROW_SIZE);
			m_Items[m_Size++] = item;
		}

		/**
		 * \brief Removes item at given index. Other elements will be shifted to fill the gap.
		 */
		void Remove(u16 index)
		{
			if (index < 0 || index >= m_Size)
				throw std::out_of_range("atArray::Remove() -> Index was out of range.");

			memmove(m_Items + index, m_Items + index + 1,
				(size_t)(m_Size - index - 1) * sizeof(T));
			m_Size--;
		}

		/**
		 * \brief Reallocates array to given capacity.
		 * \n Elements that don't fit new array range will be truncated.
		 * \n Resizing array to 0 will deallocate it.
		 */
		void Resize(u16 capacity)
		{
			m_Capacity = capacity;
			m_Size = std::min(m_Capacity, m_Size);

			if (m_Capacity == 0)
			{
				Free();
				return;
			}


			T* oldItems = m_Items;
			DoAlloc();
			if (!oldItems)
				return;

			memcpy(m_Items, oldItems, m_Size * sizeof(T));
			delete[] oldItems;
		}

		/**
		 * \brief Deallocate reserved array memory.
		 */
		void Free()
		{
			if (!m_Items)
				return;

			delete[] m_Items;
			m_Items = nullptr;
		}

		/**
		 * \brief Sorts array. Has to be done before performing search.
		 */
		void Sort()
		{
			std::sort((T*)begin(), (T*)end());
		}

		/**
		 * \brief Gets item reference at given index.
		 * \throws std::out_of_range If index was negative or greater than array size.
		 */
		T& Get(u16 index) const
		{
			if (index < 0 || index >= m_Size)
				throw std::out_of_range("atArray::Get() -> Index was out of range.");

			return m_Items[index];
		}

		/**
		 * \brief Sets value at given index.
		 * \throws std::out_of_range If index was negative or greater than array size.
		 */
		T& Set(u16 index, const T& value) const
		{
			if (index < 0 || index >= m_Size)
				throw std::out_of_range("atArray::Set() -> Index was out of range.");

			return m_Items[index] = value;
		}

		/**
		 * \brief Gets pointer for item at given index.
		 * \throws std::out_of_range If index was negative or greater than array size.
		 */
		T* GetPtr(u16 index) const
		{
			if (index < 0 || index >= m_Size)
				throw std::out_of_range("atArray::GetPtr() -> Index was out of range.");

			return &m_Items[index];
		}

		/**
		 * \brief Returns index of iterator. If iterator was nullptr, returns -1;
		 */
		u16 IndexOf(const atIterator<T>& it)
		{
			if (it == nullptr)
				return -1;
			return it - m_Items;
		}

		/**
		 * \brief Performs binary search to find value.
		 * \return Iterator on value (which is just pointer).
		 * \n Use IndexOf to get actual index in array.
		 */
		atIterator<T> Find(const T& value)
		{
			atIterator<T> l = begin();
			atIterator<T> r = end() - 1;

			while (l <= r)
			{
				atIterator<T> m = l.Mid(r);
				if (m > value)
					r = m - 1;
				else if (m < value)
					l = m + 1;
				else return m;
			}
			return nullptr;
		}

		atIterator<T> begin() const { return m_Items; }
		atIterator<T> end() const { return m_Items + m_Size; }

		T& operator [](u16 index) { return Get(index); }
	};

	// Size is 12 bytes + 4 byte pad
	static_assert(sizeof(atArray<void*>) == 12 + 4);
}
