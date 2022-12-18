#pragma once

namespace rage
{
	struct Vec3V
	{
		float X, Y, Z;

		// 4th component is most likely padding, but who knows.
		// For some reason it matches (sometimes?) with X
		// Apparently this is related to SIMD accelerated structs
		float W;
	};
	static_assert(sizeof(Vec3V) == 0x10);
}
