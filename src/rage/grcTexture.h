#pragma once
#include <d3d11.h>
#include "paging/datResource.h"

#ifndef RAGE_STANDALONE
#include "GameVersion.h"
#endif

namespace rage
{
	class grcTexture;
	/*
	 * grcTextureDX11
	 * - vftable
	 *		2802 Adds 6 methods with static variable in beginning
	 *		0x0		~grcTextureDX11();
	 *		0x8		uint32_t Get0x44();
	 *		0x10	Set0x44(uint32_t);
	 *		0x18	// ret 0
	 *		0x20	// ret 0
	 *		0x28	GetWidth();
	 *		0x30	GetHeight();
	 *		0x38	GetDepth();
	 *		0x40	GetMipLevels();
	 *		0x48
	 *		0x50	// calls grcTextureFactory
	 *		0x58	// ret 1
	 *		0x60
	 *		0x68	// ret 0
	 *		0x70	// ret 0
	 *		0x78	// ret 0
	 *		0x80	// ret 0
	 *		0x88	// ret this
	 *		0x90	// ret this
	 *		0x98	// ret byte5F & 1
	 *		0x100	// ret 0x38
	 *		0x108	// ret 0x38
	 *		0x110	GetShaderResourceView();
	 */

	struct grcTextureDX11_vftable
	{
		void(__stdcall* Dctor)();
		void(__stdcall* Get_0x44)();
		void(__stdcall* Set_0x44)();
		int64_t nullsub1;
		int64_t nullsub2;
		void(__stdcall* GetWidth)();
		void(__stdcall* GetHeight)();
		void(__stdcall* GetDepth)();
		void(__stdcall* GetMipLevels)();
		void(__stdcall* Function9)();
		void(__stdcall* Get_SmthFromGrcTxtFactory)();
		void(__stdcall* nullsub_ret_true)();
		int64_t nullsub3;
		int64_t nullsub4;
		int64_t nullsub5;
		int64_t nullsub6;
		int64_t nullsub7;
		int64_t nullsub8;
		int64_t nullsub9;
		void(__stdcall* Get0x5F_HasFlag0x1)();
		void(__stdcall* GetTexture)();
		void(__stdcall* GetTexture_duplicate)();
		ID3D11ShaderResourceView* (__thiscall* GetShaderResourceView)(grcTexture*);
		int64_t nullsub10;
		void(__stdcall* GetUnk24)();
		void(__stdcall* Function25)();
		void(__stdcall* Function26)();
		int64_t nullsub11;
		int64_t nullsub12;
		int64_t nullsub13;
		int64_t nullsub14;
		int64_t nullsub15;
		int64_t nullsub16;
	};

	class grcTexture
	{
	public:

		grcTextureDX11_vftable* vftable;
		int64_t unk8;
		int64_t unk10;
		intptr_t unk18;
		intptr_t unk20;
		const char* m_Name;
		int16_t word30;
		int8_t byte32;
		int8_t m_ArraySize;
		int32_t dword34;
		ID3D11Texture2D* pTexture; //0x0038
		int32_t dword40;
		int32_t dword44;
		int32_t dword48;
		int16_t word4C;
		int16_t word4E;
		int16_t m_Width;
		int16_t m_Height;
		int16_t m_Depth;
		int16_t m_Stride;
		uint32_t m_Format;
		uint8_t byte5C;
		uint8_t m_MipLevels;
		uint8_t byte5E;
		uint8_t byte5F;
		grcTexture* m_PreviousTexture;
		grcTexture* m_NextTexture;
		void* pTextureData;
		ID3D11ShaderResourceView* pShaderResourceView; //0x0078
		int64_t unk80;
		int64_t unk88;

		grcTexture() {}

		grcTexture(const datResource& rsc)
		{
			rsc.Fixup(m_Name); // Actually atString
			rsc.Fixup(pTextureData);
		}

		int GetWidth() const
		{
			return m_Width;
		}

		int GetHeight() const
		{
			return m_Height;
		}

		int GetDepth() const
		{
			return m_Depth;
		}

		const char* GetName() const
		{
			if (m_Name == nullptr)
				return "UNDEFINED";

			return m_Name;
		}

		ID3D11Texture2D* GetTexture() const
		{
			return pTexture;
		}

		ID3D11ShaderResourceView* GetShaderResourceView()
		{
#ifdef RAGE_STANDALONE
			throw;
#else
			// TODO: grcRenderTargetTextureDX11 structure is different
			//return pShaderResourceView;

			// TODO: 2802 altered virtual table slightly, find a good solution for this
			//return this->vftable->GetShaderResourceView(this);

			if (GameVersion::IsGreaterOrEqual(VER_2802))
			{
				return decltype(this->vftable->GetShaderResourceView)(*(uintptr_t*)(*(uintptr_t*)this + 0xE0))(this);
			}

			return this->vftable->GetShaderResourceView(this);
#endif
		}

		void SetTexture(ID3D11Texture2D* texture)
		{
			pTexture = texture;
		}

		void SetTexture(ID3D11Resource* texture)
		{
			*reinterpret_cast<int64_t*>(&pTexture) = reinterpret_cast<int64_t>(texture);
		}

		void SetShaderResourceView(ID3D11ShaderResourceView* resourceView)
		{
			pShaderResourceView = resourceView;
		}
	};
	static_assert(sizeof(grcTexture) == 0x90);
	static_assert(offsetof(grcTexture, pTexture) == 0x38);
	static_assert(offsetof(grcTexture, pShaderResourceView) == 0x78);
}
