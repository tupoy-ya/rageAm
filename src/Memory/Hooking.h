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
	// Searches for IDA style pattern address.
	uintptr_t FindPattern(const char* pattern);
	// Searches for call relative offset (mov [gta5.exe+offset], 0x0).
	uintptr_t FindOffset(uintptr_t origin);

	// Searches for IDA style pattern address and logs result.
	uintptr_t FindPattern(std::string name, const char* pattern);
	// Searches for call relative offset (mov [gta5.exe+offset], 0x0) and logs result.
	uintptr_t FindOffset(std::string name, uintptr_t origin);

	template <typename T>
	void SetHook(intptr_t target, LPVOID detour, T** original)
	{
		SetHook((LPVOID)target, detour, (LPVOID*)original);
	}

	template <typename T>
	void SetHook(LPVOID target, LPVOID detour, T** original)
	{
		SetHook(target, detour, (LPVOID*)original);
	}

	// Hooks target func with detour. Suitable for function replacement.
	void SetHook(LPVOID target, LPVOID detour);
	// Hooks target func with detour and keeping original entry point.
	void SetHook(LPVOID target, LPVOID detour, LPVOID* original);
	// Hooks target func by IDA pattern with detour and keeping original entry point.
	void SetHook(const char* pattern, LPVOID detour, LPVOID* original);
	// Removes hook from target function.
	void UnHook(LPVOID target);
	// Removes hook from all processed functions.
	void UnHookAll();
};

extern Hooker* g_hook;
