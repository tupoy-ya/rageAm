#pragma once

#include <vector>
#include <string>
#include <Windows.h>
#include "Template/atSingleton.h"

class Hooker : public atSingleton<Hooker>
{
	std::vector<LPVOID> _hooks;
	std::vector<std::string> Split(const std::string& s, char delimiter);
public:
	uintptr_t FindPattern(const char* pattern);
	uintptr_t FindRelativeAddressWithOffset(uintptr_t origin, int offset = 4);

	void SetHook(LPVOID target, LPVOID detour);
	void SetHook(LPVOID target, LPVOID detour, LPVOID* original);
	void SetHook(const char* pattern, LPVOID detour, LPVOID* original);
	void UnHook(LPVOID target);
	void UnHookAll();
};

extern Hooker* g_hook;
