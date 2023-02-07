#pragma once
#include "fwTypes.h"

namespace rage
{
	struct grcTextureInfo
	{
		u32 MipLevels;
		uint64_t qword8;
		u32 RowPitch;
		u32 BitsPerPixel;
		u32 Width;
		u32 Height;
		u32 Format;
		uint32_t dword24;
	};
}
