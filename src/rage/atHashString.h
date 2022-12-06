#pragma once
#include "fwTypes.h"
#include "fwHelpers.h"

namespace rage
{
	class atHashString
	{
		const char* m_str;
		uint32_t m_hash;

	public:
		atHashString(const char* str)
		{
			m_str = str;
		}

		operator uint32_t() const { return m_hash; }
	};
}
