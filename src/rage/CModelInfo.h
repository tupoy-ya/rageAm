#pragma once
#include "fwModelId.h"

namespace rage
{
	class CBaseModelInfo
	{
		int64_t vftable;
		int64_t qword8;
		int64_t qword10;
		uint32_t m_NameHash;

	public:
		uint32_t GetNameHash() const
		{
			return m_NameHash;
		}
	};

	namespace hooks
	{
		static inline gm::gmFunc<CBaseModelInfo*, fwModelId*> gImpl_GetModelInfoFromId([]() -> auto
			{
				return gm::Scan("CModelInfo::GetModelInfoFromId", "E8 ?? ?? ?? ?? 44 8B 78 18")
					.GetCall();
			});
	}

	class CModelInfo
	{
	public:
		static CBaseModelInfo* GetModelInfoFromId(fwModelId* modelId)
		{
			return hooks::gImpl_GetModelInfoFromId(modelId);
		}
	};
}
