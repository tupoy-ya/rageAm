//
// File: texturepc.h
//
// Copyright (C) 2023 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include "texture.h"
#include "utils.h"

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
		grcTexturePC()
		{
			m_PcFlags1 = 0;
			dword4C = 0;
			m_Width = 0;
			m_Height = 0;
			m_Depth = 0;
			m_Stride = 0;
			m_Format = (DXGI_FORMAT)0;
			m_Type = GRC_TEXTURE_TYPE_DEFAULT;
			m_MipLevels = 0;
			m_DefaultMipLevel = 0;
			m_PcFlags2 = 0;
			m_PreviousTexture = nullptr;
			m_NextTexture = nullptr;
		}
		grcTexturePC(u16 width, u16 height, u16 depth, u8 mipLevels) : grcTexturePC()
		{
			m_Width = width;
			m_Height = height;
			m_Depth = depth;
			m_MipLevels = mipLevels;
		}
		grcTexturePC(const grcTexturePC& other) : grcTexture(other)
		{
			m_PcFlags1 = 0;
			dword4C = 0;
			m_DefaultMipLevel = 0;
			m_PcFlags2 = 0;
			m_PreviousTexture = nullptr;
			m_NextTexture = nullptr;

			m_MipLevels = other.m_MipLevels;
			m_Stride = other.m_Stride;
			m_Width = other.m_Width;
			m_Height = other.m_Height;
			m_Depth = other.m_Depth;
			m_Format = other.m_Format;
			m_Type = other.m_Type;
		}
		// ReSharper disable once CppPossiblyUninitializedMember
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
