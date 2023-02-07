#pragma once
#include "grcTexture.h"
#include "TextureUtils.h"

namespace rage
{
	enum ePCTextureType : u8
	{
		GRC_TEXTURE_TYPE_DEFAULT = 0,	// Regular.
		GRC_TEXTURE_TYPE_CUBEMAP = 1,	// 2D Cubemap.
		GRC_TEXTURE_TYPE_UNK2 = 2,		// Unused.
		GRC_TEXTURE_TYPE_VOLUME = 3,	// 3D Texture.
	};

	class grcTexturePC : public grcTexture
	{
	protected:
		int32_t m_PcFlags1;
		int32_t dword4C;
		u16 m_Width;
		u16 m_Height;
		u16 m_Depth;
		u16 m_Stride;
		DXGI_FORMAT m_Format;
		ePCTextureType m_Type;
		u8 m_MipLevels;
		u8 m_DefaultMipLevel;
		u8 m_PcFlags2;
		grcTexturePC* m_PreviousTexture;
		grcTexturePC* m_NextTexture;
	public:
		grcTexturePC(const datResource& rsc) : grcTexture(rsc) {}

		u8 GetBitsPerPixel() const override { return D3D::BitsPerPixel(m_Format); }

		u16 GetHeight() const override { return m_Height; }
		u16 GetWidth() const override { return m_Width; }
		u16 GetDepth() const override { return m_Depth; }

		u8 GetMipLevels() const override { return m_MipLevels; }

		grcTextureInfo GetTextureInfo() override { return {}; }

		u32 GetUnkFlags(u32& flags) override;
		bool GetUnk19() override { return m_PcFlags2 & 1; }

		bool IsVolume() const { return m_Type == GRC_TEXTURE_TYPE_VOLUME; }
		bool IsCubeMap() const { return m_Type == GRC_TEXTURE_TYPE_CUBEMAP; }
	};
	static_assert(sizeof(grcTexturePC) == 0x70);
}
