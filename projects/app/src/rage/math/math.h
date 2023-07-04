//
// File: math.h
//
// Copyright (C) 2023 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include <cmath>

namespace rage
{
	class Math
	{
	public:
		static bool AlmostEquals(float a, float b, float maxDelta = 0.01f) { return abs(a - b) <= maxDelta; }

		template<typename T>
		static const T& Max(const T& left, const T& right) { return left >= right ? left : right; }

		template<typename T>
		static const T& Min(const T& left, const T& right) { return left <= right ? left : right; }
	};
}
