#include "pattern.h"
#include <Windows.h>
#include <sstream>
#include <vector>

constexpr uint16_t PATTERN_WILDCARD = 0xFFFF;

uintptr_t gm::FindPattern(const char* pattern, const uint8_t* address, uint32_t size, uint8_t align)
{
	static uint16_t byteBuffer[256];

	uint8_t numBytes = 0;
	uint8_t len = (uint8_t)strlen(pattern);
	for (uint8_t i = 0; i < len; i += 3)
	{
		const char* offset = pattern + i;
		if (offset[0] == '?')
			byteBuffer[numBytes] = PATTERN_WILDCARD;
		else
			byteBuffer[numBytes] = (uint8_t)strtol(offset, nullptr, 16);
		numBytes++;

		// Support single '?', since they took only one symbol, fall back by one
		if (offset[1] == ' ')
			i--;
	}

	for (uint32_t i = 0; i < size; i += align)
	{
		const uint8_t* offset = address + i;
		for (uint8_t j = 0; j < numBytes; j++)
		{
			uint16_t byte = byteBuffer[j];
			if (byte == PATTERN_WILDCARD)
				continue;

			uint8_t target = offset[j];

			if (target != byte)
				goto miss;
		}
		return (uintptr_t)offset;
	miss:;
	}

	// In best case scenario, we'll successfully scan with alignment of 8, worst - 1
	// (can this ever happen?) need to profile it
	if (align == 8)
		return FindPattern(pattern, address, size, 4);
	if (align == 4)
		return FindPattern(pattern, address, size, 1);
	return 0;
}

uintptr_t gm::FindRef(uintptr_t origin)
{
	if (origin == 0)
		return 0;

	// Offset m_address + size of offset (int) + offset value
	return origin + 4 + *reinterpret_cast<int*>(origin);
}
