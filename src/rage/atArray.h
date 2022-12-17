#pragma once
#include "fwTypes.h"

namespace rage
{
	template<typename T>
	class atArray
	{
		//intptr_t items;
		T* items;
		int16_t size;
		int16_t unk10;
		int32_t unk18;

	public:
		int GetSize() const
		{
			return size;
		}

		T* GetAt(int index) const
		{
			if (index < 0 || index > size)
				return nullptr;

			// Not my brightest moment but how to get pointer
			// otherwise? It's not pointer array so indexing
			// is not an option

			return &items[index];

			// TODO: &items[index]; should work
			//return reinterpret_cast<T*>(items + sizeof(T) * index);
		}

		template<typename T2>
		T* GetAt(int index) const
		{
			uintptr_t addr = (uintptr_t)items + sizeof(T2) * index;
			return (T*)addr;
		}

		T* operator[](int index) const
		{
			return GetAt(index);
		}
	};
}
