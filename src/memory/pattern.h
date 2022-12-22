#pragma once
#include <string>

namespace gm
{
	uintptr_t FindPattern(const char* pattern, const uint8_t* address, uint32_t size, uint8_t align = 8);
	uintptr_t FindRef(uintptr_t origin);
}
