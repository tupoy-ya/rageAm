#pragma once
#include <d3d11.h>

namespace rage
{
	class grcTexture;
	/*
	 * grcTextureDX11
	 * - vftable
	 *		0x0		~grcTextureDX11();
	 *		0x8		uint32_t Get0x44();
	 *		0x10	Set0x44(uint32_t);
	 *		0x18	// ret 0
	 *		0x20	// ret 0
	 *		0x28	GetWidth();
	 *		0x30	GetHeight();
	 *		0x38	GetDepth();
	 *		0x40	uint8_t Get0x5D();
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
		void(__stdcall* Get_5D)();
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
		char pad_0008[32]; //0x0008
		const char* name; //0x0028
		char pad_0030[8]; //0x0030
		ID3D11Texture2D* pTexture; //0x0038
		char pad_0040[4]; //0x0040
		int32_t N00002CD4; //0x0044
		char pad_0048[8]; //0x0048
		int16_t width; //0x0050
		int16_t height; //0x0052
		int16_t depth; //0x0054
		char pad_0056[2]; //0x0056
		int16_t N00002C92; //0x0058
		char pad_005A[3]; //0x005A
		int8_t N00002C8E; //0x005D
		char pad_005E[1]; //0x005E
		int8_t N00002C8C; //0x005F
		char pad_0060[4]; //0x0060
		int32_t N00002C34; //0x0064
		char pad_0068[4]; //0x0068
		int32_t N00002C35; //0x006C
		char pad_0070[8]; //0x0070
		ID3D11ShaderResourceView* pShaderResourceView; //0x0078
		char pad_0080[16]; //0x0080

		int GetWidth() const
		{
			return width;
		}

		int GetHeight() const
		{
			return height;
		}

		int GetDepth() const
		{
			return depth;
		}

		const char* GetName() const
		{
			if (name == nullptr)
				return "UNDEFINED";

			return name;
		}

		ID3D11Texture2D* GetTexture() const
		{
			return pTexture;
		}

		ID3D11ShaderResourceView* GetShaderResourceView()
		{
			// TODO: grcRenderTargetTextureDX11 structure is different
			//return pShaderResourceView;

			return this->vftable->GetShaderResourceView(this);
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
