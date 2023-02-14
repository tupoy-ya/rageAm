#pragma once
#include "dat.h"
#include "helpers/align.h"
#include "helpers/bits.h"

namespace rage
{
	inline u64 GetPageSize(u64 value)
	{
		if (value == 0)
			return 0;

		if (value < DAT_MIN_CHUNK_SIZE)
			return DAT_MIN_CHUNK_SIZE;
		return AlignToNextPowerOf2(value);
	}

	inline u8 GetPageShift(u64 pageSize)
	{
		return BitScanReverse64(pageSize) - BitScanReverse64(DAT_MIN_CHUNK_SIZE);
	}
}
