#pragma once

namespace rage
{
	template<typename T>
	class pgDictionary
	{
		_QWORD vftable;
		_BYTE gap8[8];
		_QWORD qword10;
		_DWORD dword18;
		_BYTE gap1C[4];
		_QWORD qword20;
		int16_t count;
		int16_t word2A;
		_BYTE gap2C[4];
		T** values;
		_DWORD dword38;

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