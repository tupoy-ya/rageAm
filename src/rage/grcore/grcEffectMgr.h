#pragma once
#include "gmFunc.h"

namespace rage::hooks
{
	static inline gm::gmFuncScan<grcEffect*, uint32_t> gImpl_FindEffectByHashKey(
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
			return hooks::gImpl_FindEffectByHashKey(hashKey);
		}
	};
}
