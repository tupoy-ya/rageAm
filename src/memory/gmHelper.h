#pragma once
#include "gmHook.h"
#include "gmScanner.h"

namespace gm
{
	template<typename Detour, typename Original>
	void ScanAndHook(const char* name, const std::string& pattern, Detour detour, Original original)
	{
		gmAddress addr = g_Scanner.ScanPattern(name, pattern);
		g_Hook.SetHook(addr, detour, original);
	}

	template<typename Detour>
	void ScanAndHook(const char* name, const std::string& pattern, Detour detour)
	{
		gmAddress addr = g_Scanner.ScanPattern(name, pattern);
		g_Hook.SetHook(addr, detour);
	}

	template<typename Func>
	void ScanAndSet(const char* name, const std::string& pattern, Func* func)
	{
		gmAddress addr = g_Scanner.ScanPattern(name, pattern);
		*func = reinterpret_cast<Func>(addr.GetAddress());
	}
}
