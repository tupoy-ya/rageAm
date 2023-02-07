#pragma once
#include <cstdint>

/**
 * \brief Four character code of DDS compression formats.
 */
enum eFourCC : uint32_t
{
	DXT1 = 0x31545844,
	DXT2 = 0x32545844,
	DXT3 = 0x33545844,
	DXT4 = 0x34545844,
	DXT5 = 0x35545844,
	ATI1 = 0x31495441,
	ATI2 = 0x32495441,

	RGBG = 0x47424752,
	GRGB = 0x42475247,
	G8R8 = 0x38523847,
	YUY2 = 0x32595559,
	R16 = 0x20363152,
	BC6 = 0x20364342,
	BC7 = 0x20374342,
	R8 = 0x20203852,
};
