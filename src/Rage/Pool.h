#pragma once

namespace rage
{
	class fwBasePool;

	enum ePoolFlags
	{
		POOL_ACTIVE = 0x2,
	};

	class __declspec(align(8)) fwBasePool
	{
	public:
		intptr_t _entryList;
		intptr_t _flagsList;
		int _poolSize;
		int32_t _sizeOfEntry;
		int8_t gap18[12];
		int flags;

	public:
		uint8_t GetSlotFlags(int index) const
		{
			return *reinterpret_cast<uint8_t*>(_flagsList + index);
		}

		int GetSize() const
		{
			return _poolSize;
		}

		bool IsSlotActive(int index) const
		{
			return GetSlotFlags(index) & POOL_ACTIVE;
		}

		int GetNumActiveSlots() const
		{
			int count = 0;
			for (int i = 0; i < GetSize(); i++)
			{
				if (!IsSlotActive(i))
					continue;
				count++;
			}
			return count;
		}

		intptr_t GetSlotPtr(int index) const
		{
			if (!IsSlotActive(index))
				return 0;

			return _entryList + _sizeOfEntry * index;
		}

		template<typename T>
		T* GetSlot(int index) const
		{
			if (!IsSlotActive(index))
				return nullptr;

			return reinterpret_cast<T*>(GetSlotPtr(index));
		}
	};
}

typedef rage::fwBasePool CPool;
