#pragma once
#include <d3d11.h>

namespace rage
{
	class grcTexture
	{
		intptr_t vftable;
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

	public:
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

		ID3D11ShaderResourceView* GetShaderResourceView() const
		{
			return pShaderResourceView;
		}
	};
}