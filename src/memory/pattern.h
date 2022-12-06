#pragma once
#include <string>

namespace gm
{
	uintptr_t FindPattern(const char* module, const std::string& pattern);
	uintptr_t FindRef(uintptr_t origin);
}
