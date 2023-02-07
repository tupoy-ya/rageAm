#pragma once
#include "fwTypes.h"
#include "fwHelpers.h"

namespace rage
{
	class atHashString
	{
		u32 m_Hash;
	public:
		atHashString(const char* str)
		{
			m_Hash = fwHelpers::joaat(str);
		}

		u32 GetHash() const
		{
			return m_Hash;
		}

		operator u32() const { return m_Hash; }
	};
}
