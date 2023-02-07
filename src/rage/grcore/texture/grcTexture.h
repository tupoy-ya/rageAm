#pragma once

#include "grcTextureVFBase.h"
#include "paging/datResource.h"

namespace rage
{
	class grcTexture : public grcTextureVFBase
	{
	protected:
		int64_t unk8;
		int64_t unkArray10[3];
		const char* m_Name;
		int16_t word30;
		int8_t byte32;
		u8 m_ArraySize;
		int32_t dword34;
		void* m_Resource;
		int32_t dword40;
		int32_t dword44;
	public:
		grcTexture(const datResource& rsc)
		{
			rsc.Fixup(m_Name); // Actually atString
		}

		int32_t Get44() const override { return dword44; }
		void Set44(int32_t value) override { dword44 = value; }
		void* GetResource1() override { return m_Resource; }
		void* GetResource2() override { return m_Resource; }

		const char* GetName() const
		{
			if (m_Name == nullptr)
				return "-";
			return m_Name;
		}
		void SetName(const char* name) { m_Name = name; }
	};
	static_assert(sizeof(grcTexture) == 0x48);
}
