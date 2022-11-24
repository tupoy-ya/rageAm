#pragma once

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
}