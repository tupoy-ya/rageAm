//
// File: texture.h
//
// Copyright (C) 2023 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include "texturebase.h"

#include "rage/paging/resource.h"
#include "rage/paging/compiler/compiler.h"
#include "rage/paging/compiler/snapshotallocator.h"

namespace rage
{
	class grcTexture : public grcTextureBase
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
		grcTexture()
		{
			m_Name = nullptr;

			unk8 = 0;
			memset(unkArray10, 0, sizeof unkArray10);
			word30 = 0;
			m_ArraySize = 0;
			dword34 = 0;
			m_Resource = nullptr;
			dword40 = 0;
			dword44 = 0;
		}
		grcTexture(const grcTexture& other)
		{
			m_Name = other.m_Name;
			m_ArraySize = other.m_ArraySize;

			m_ArraySize = 0; // TODO: Temp...

			pgSnapshotAllocator* pAllocator = pgRscCompiler::GetVirtualAllocator();
			AM_ASSERT(pAllocator, "grcTexture -> Copy constructor is not implemented.");

			pAllocator->AllocateRefString(m_Name);
		}
		grcTexture(const datResource& rsc)
		{
			rsc.Fixup(m_Name);
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
