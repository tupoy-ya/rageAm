#pragma once

#include "datResourceInfo.h"

namespace rage
{
#define MAGIC_RSC MAKEFOURCC('R', 'S', 'C', '7')

	struct ResourceHeader
	{
		u32 Magic;
		u32 Version;
		datResourceInfo Info;
	};

	struct ResourceFile : ResourceHeader
	{
		u8 Data;
	};
	static_assert(offsetof(ResourceFile, Data) == 0x10);
}
