#pragma once

namespace rage
{
	template<typename T>
	class pgDictionary
	{
		int64_t vftable;
		int8_t gap8[8];
		int64_t qword10;
		int32_t dword18;
		int8_t gap1C[4];
		int64_t qword20;
		int16_t count;
		int16_t word2A;
		int8_t gap2C[4];
		T** values;
		int32_t dword38;

	public:
		int GetCount() const
		{
			return count;
		}

		T* GetValue(int index)
		{
			if (index < 0 || index >= count)
				return nullptr;

			return values[index];
		}
	};
}