#pragma once

#include "fwTypes.h"

namespace fwHelpers
{
	inline char ToLower(char c)
	{
		if (c >= 65 && c - 65 <= 25)
			return c + 32;
		return c;
	}

	inline bool IsPathSeparator(char c)
	{
		return c == '/' || c == '\\';
	}

	inline bool IsEndOfString(char c)
	{
		return c == '\0';
	}

	constexpr uint32_t jooat(const char* str)
	{
		size_t i = 0;
		uint32_t hash = 0;
		while (str[i] != '\0') {
			hash += str[i++];
			hash += hash << 10;
			hash ^= hash >> 6;
		}
		hash += hash << 3;
		hash ^= hash >> 11;
		hash += hash << 15;
		return hash;
	}
}
