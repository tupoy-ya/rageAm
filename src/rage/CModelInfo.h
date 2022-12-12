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

	class CModelInfo
	{
	public:
		static CBaseModelInfo* GetModelInfoFromId(fwModelId* modelId)
		{
			return gImpl_GetModelInfoFromId(modelId);
		}

		typedef CBaseModelInfo* (__fastcall* gDef_GetModelInfoFromId)(fwModelId*);
		static inline gDef_GetModelInfoFromId gImpl_GetModelInfoFromId;
	};
}
