#pragma once

#include "DirectXTex.h"
#include "D3DFormat.h"
#include "fwTypes.h"

// Undef conflicting Windows.h macro
#undef min
#undef max

namespace D3D
{
	inline bool IsCompressed(DXGI_FORMAT fmt)
	{
		return DirectX::IsCompressed(fmt);
	}

	inline u32 BitsPerPixel(DXGI_FORMAT fmt)
	{
		return DirectX::BitsPerPixel(fmt);
	}

	inline bool IsPowerOfTwo(u32 value)
	{
		return value != 0 && (value & value - 1) == 0;
	}

	/**
	 * \brief If size is not multiple of 4, aligns it up to next multiple of 4.
	 * \n For e.g. 511 will be aligned to 512; 256 will remain as 256.
	 */
	inline u32 AlignSize(u32 size)
	{
		return size + 3 & ~3;
	}

	/**
	 * \brief Converts texture dimension to mip level.
	 * \n For e.g. 512 on 2nd mip level is 512 >> 2 = 512 / 4 = 128;
	 * \n NOTE: If result value is 0, (for e.g. 512 on mip 8 is 0), 1 returned.
	 * \param value Width / Height / Depth of the texture.
	 * \param mip Mip level.
	 */
	inline u32 MipSize(u32 value, u32 mip)
	{
		value >>= mip;
		return value > 0 ? value : 1;
	}

	/**
	 * \brief Computes row and slice pitches.
	 * \n - Row pitch is distance (in bytes) one line of texture occupies (Line is pixels from '0' to 'Width').
	 * \n - Row slice pitch is distance (in bytes) of one depth level. (Row Pitch * Height).
	 * \remark Out params can be NULL if not required.
	 */
	inline HRESULT ComputePitch(DXGI_FORMAT fmt, u32 width, u32 height, u32 mip, u32* pRowPitch, u32* pSlicePitch)
	{
		size_t rowPitch, slicePitch;
		width = MipSize(width, mip);
		height = MipSize(height, mip);
		HRESULT result = DirectX::ComputePitch(fmt, width, height, rowPitch, slicePitch);
		if (pRowPitch) *pRowPitch = rowPitch;
		if (pSlicePitch) *pSlicePitch = slicePitch;
		return result;
	}

	/**
	 * \brief Gets texture size (in bytes) at given mip level.
	 */
	inline HRESULT GetTextureMipSize(DXGI_FORMAT fmt, u32 width, u32 height, u32 depth, u32 mip, u32& size)
	{
		u32 slicePitch;

		HRESULT result = ComputePitch(fmt, width, height, mip, nullptr, &slicePitch);
		if (result != S_OK) return result;

		depth = MipSize(depth, mip);
		size = depth * slicePitch;
		return S_OK;
	}

	/**
	 * \brief Gets texture size in bytes.
	 */
	inline HRESULT GetTextureSize(DXGI_FORMAT fmt, u32 width, u32 height, u32 depth, u32 mipLevels, u32& size)
	{
		u32 sum = 0;
		for (u32 i = 0; i < mipLevels; i++)
		{
			u32 mipSize;
			HRESULT result = GetTextureMipSize(fmt, width, height, depth, i, mipSize);
			if (result != S_OK) return result;
			sum += mipSize;
		}
		size = sum;
		return S_OK;
	}

	/**
	 * \brief Gets maximum number of mip levels for given resolution.
	 * \n If size is not power of two, 0 is returned.
	 */
	inline u8 GetAvailableMipLevels(u32 width, u32 height)
	{
		if (!IsPowerOfTwo(width) || !IsPowerOfTwo(height))
			return 0;

		unsigned long highBit;
		_BitScanReverse(&highBit, std::min(width, height));
		return (u8)highBit;
	}

	/**
	 * \brief Ensures that given number of mip levels does not exceed
	 * available number of mip levels for given resolution.
	 */
	inline u32 ClampMipLevels(u32 width, u32 height, u8 mipLevels)
	{
		u8 available = GetAvailableMipLevels(width, height);
		return std::min(available, mipLevels);
	}

	/**
	 * \brief Converts legacy (D3DFORMAT?) grc texture format to DXGI.
	 */
	inline DXGI_FORMAT TextureFormatToDXGI(u32 fmt, u8 flags)
	{
		if (flags != 0)
		{
			switch (fmt)
			{
			case BC7:	return DXGI_FORMAT_BC7_UNORM_SRGB;
			case DXT1:	return DXGI_FORMAT_BC1_UNORM_SRGB;
			case DXT2:
			case DXT3:	return DXGI_FORMAT_BC2_UNORM_SRGB;
			case DXT4:
			case DXT5:	return DXGI_FORMAT_BC3_UNORM_SRGB;

			case 21:
			case 63:	return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			case 32:
			case 33:	return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			case 22:
			case 62:	return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
			case 75:
			case 77:
			case 79:
			case 83:	return DXGI_FORMAT_D24_UNORM_S8_UINT;
			case 82:
			case 84:	return DXGI_FORMAT_D32_FLOAT;
			}

			return DXGI_FORMAT_UNKNOWN;
		}

		switch (fmt)
		{
		case R16:	return DXGI_FORMAT_R16_UNORM;
		case BC6:	return DXGI_FORMAT_BC6H_UF16;
		case BC7:	return DXGI_FORMAT_BC7_UNORM;
		case ATI1:	return DXGI_FORMAT_BC4_UNORM;
		case DXT1:	return DXGI_FORMAT_BC1_UNORM;
		case ATI2:	return DXGI_FORMAT_BC5_UNORM;
		case GRGB:	return DXGI_FORMAT_G8R8_G8B8_UNORM;
		case RGBG:	return DXGI_FORMAT_R8G8_B8G8_UNORM;
		case DXT2:
		case DXT3:	return DXGI_FORMAT_BC2_UNORM;
		case DXT4:
		case DXT5:	return DXGI_FORMAT_BC3_UNORM;
		case 60:
		case G8R8:	return DXGI_FORMAT_R8G8_UNORM;
		case 41:
		case 50:
		case R8:	return DXGI_FORMAT_R8_UNORM;

		case 23:	return DXGI_FORMAT_B5G6R5_UNORM;
		case 28:	return DXGI_FORMAT_A8_UNORM;
		case 36:	return DXGI_FORMAT_R16G16B16A16_UNORM;
		case 40:	return DXGI_FORMAT_R8G8_UINT;
		case 21:
		case 63:	return DXGI_FORMAT_B8G8R8A8_UNORM;
		case 22:
		case 62:	return DXGI_FORMAT_B8G8R8X8_UNORM;
		case 24:
		case 25:	return DXGI_FORMAT_B5G5R5A1_UNORM;
		case 31:
		case 67:
		case 35:	return DXGI_FORMAT_R10G10B10A2_UNORM;
		case 32:
		case 33:	return DXGI_FORMAT_R8G8B8A8_UNORM;
		case 34:
		case 64:	return DXGI_FORMAT_R16G16_UNORM;
		case 70:
		case 80:	return DXGI_FORMAT_D16_UNORM;
		case 71:
		case 82:
		case 84:	return DXGI_FORMAT_D32_FLOAT;
		case 75:
		case 77:
		case 79:
		case 83:	return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case 111:	return DXGI_FORMAT_R16_FLOAT;
		case 112:	return DXGI_FORMAT_R16G16_FLOAT;
		case 113:	return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case 114:	return DXGI_FORMAT_R32_FLOAT;
		case 115:	return DXGI_FORMAT_R32G32_FLOAT;
		case 116:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
		return DXGI_FORMAT_UNKNOWN;
	}
}
