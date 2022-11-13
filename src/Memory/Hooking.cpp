#include "Hooking.h"
#include <Psapi.h>
#include <sstream>
#include <format>

#include "../Logger.h"
#include "../../vendor/minhook-1.3.3/include/MinHook.h"

std::vector<std::string> Hooker::Split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delimiter)) {
		tokens.push_back(item);
	}
	return tokens;
}

uintptr_t Hooker::FindPattern(const char* pattern)
{
	std::vector<std::string> bytesStr = Split(pattern, ' ');

	MODULEINFO modInfo{};
	GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &modInfo, sizeof(MODULEINFO));

	auto* start_offset = static_cast<uint8_t*>(modInfo.lpBaseOfDll);
	const auto size = static_cast<uintptr_t>(modInfo.SizeOfImage);

	uintptr_t pos = 0;
	const uintptr_t searchLen = bytesStr.size();

	for (auto* retAddress = start_offset; retAddress < start_offset + size; retAddress++) {
		if (bytesStr[pos] == "??" || bytesStr[pos] == "?" ||
			*retAddress == static_cast<uint8_t>(std::strtoul(bytesStr[pos].c_str(), nullptr, 16))) {
			if (pos + 1 == bytesStr.size())
				return (reinterpret_cast<uintptr_t>(retAddress) - searchLen + 1);
			pos++;
		}
		else {
			pos = 0;
		}
	}
	return 0;
}

uintptr_t Hooker::FindPattern(std::string name, const char* pattern)
{
	intptr_t result = FindPattern(pattern);

	g_logger->Log(std::format("FP [{}] - {:x}", name, result));

	return result;
}

uintptr_t Hooker::FindOffset(uintptr_t origin)
{
	// Offset address + size of offset (int) + offset value
	return origin + 4 + *(int*)origin;
}

uintptr_t Hooker::FindOffset(std::string name, uintptr_t origin)
{
	intptr_t result = FindOffset(origin);

	// Duplicate for logging
	int offset = *(int*)origin;

	g_logger->Log(std::format("FO [{}] - ({:x}, {:x})", name, offset, result));

	return result;
}

void Hooker::SetHook(LPVOID target, LPVOID detour)
{
	Hooker::SetHook(target, detour, NULL);
}

void Hooker::SetHook(LPVOID target, LPVOID detour, LPVOID* original)
{
	MH_CreateHook(target, detour, original);
	MH_EnableHook(target);

	_hooks.push_back(target);

	g_logger->Log(std::format("Hooking: {:x}", (long)target));
}

void Hooker::SetHook(const char* pattern, LPVOID detour, LPVOID* original)
{
	Hooker::SetHook((LPVOID)FindPattern(pattern), detour, original);
}

void Hooker::UnHook(LPVOID target)
{
	MH_DisableHook(target);

	g_logger->Log(std::format("UnHooking: {:x}", (long)target));
}

void Hooker::UnHookAll()
{
	for (auto func : _hooks)
	{
		Hooker::UnHook(func);
	}
}

Hooker* g_hook = Hooker::GetInstance();