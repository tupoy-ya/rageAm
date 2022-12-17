#pragma once
#include "../../memory/gmFunc.h"

namespace rage::hooks
{
	static inline gm::gmFuncHook<grcEffect*, uint32_t> __FindEffectByHashKey(
		"grcEffectMgr::FindEffectByHashKey",
		"48 89 5C 24 08 4C 63 1D ?? ?? ?? ?? 48");
}

namespace rage
{
	class grcEffectMgr
	{
	public:
		static grcEffect* FindEffectByHashKey(uint32_t hashKey)
		{
			return hooks::__FindEffectByHashKey(hashKey);
		}
	};
}
