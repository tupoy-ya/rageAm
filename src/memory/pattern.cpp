#include "pattern.h"

#include <Windows.h>
#include <Psapi.h>
#include <sstream>
#include <vector>

uintptr_t gm::FindPattern(const char* module, const std::string& pattern)
{
	std::vector<std::string> tokens;
	std::stringstream ss(pattern);
	std::string item;
	while (std::getline(ss, item, ' ')) {
		tokens.push_back(item);
	}

	MODULEINFO modInfo{};
	GetModuleInformation(GetCurrentProcess(), GetModuleHandle(module), &modInfo, sizeof(MODULEINFO));

	auto* start_offset = static_cast<uint8_t*>(modInfo.lpBaseOfDll);
	const auto size = static_cast<uintptr_t>(modInfo.SizeOfImage);

	uintptr_t pos = 0;
	const uintptr_t searchLen = tokens.size();

	for (auto* retAddress = start_offset; retAddress < start_offset + size; retAddress++) {
		if (tokens[pos] == "??" || tokens[pos] == "?" ||
			*retAddress == static_cast<uint8_t>(std::strtoul(tokens[pos].c_str(), nullptr, 16))) {
			if (pos + 1 == tokens.size())
				return (reinterpret_cast<uintptr_t>(retAddress) - searchLen + 1);
			pos++;
		}
		else
		{
			pos = 0;
		}
	}
	return 0;
}

uintptr_t gm::FindRef(uintptr_t origin)
{
	if (origin == 0)
		return 0;

	// Offset m_address + size of offset (int) + offset value
	return origin + 4 + *reinterpret_cast<int*>(origin);
}
