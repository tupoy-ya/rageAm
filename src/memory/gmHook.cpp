#include "gmHook.h"

void gm::gmHook::SetHook_Internal(LPVOID func, LPVOID detour, LPVOID* orig)
{
	MH_STATUS hookStatus = MH_CreateHook(func, detour, orig);
	MH_STATUS enableStatus = MH_EnableHook(func);

	if (hookStatus != MH_OK || enableStatus != MH_OK)
	{
		g_Log.LogE("gmHook::SetHook({:X}) -> Failed. Hook Status: {}. Enable Status: {}",
			reinterpret_cast<uintptr_t>(func),
			GetMHStatusStr(hookStatus),
			GetMHStatusStr(enableStatus));
		return;
	}
	g_Log.LogT("gmHook::SetHook({:X}) OK", reinterpret_cast<uintptr_t>(func));
	m_hookedFuncs.emplace(func);
}

const char* gm::gmHook::GetMHStatusStr(MH_STATUS e)
{
	switch (e)
	{
	case MH_UNKNOWN: return "MH_UNKNOWN";
	case MH_OK: return "MH_OK";
	case MH_ERROR_ALREADY_INITIALIZED: return "MH_ERROR_ALREADY_INITIALIZED";
	case MH_ERROR_NOT_INITIALIZED: return "MH_ERROR_NOT_INITIALIZED";
	case MH_ERROR_ALREADY_CREATED: return "MH_ERROR_ALREADY_CREATED";
	case MH_ERROR_NOT_CREATED: return "MH_ERROR_NOT_CREATED";
	case MH_ERROR_ENABLED: return "MH_ERROR_ENABLED";
	case MH_ERROR_DISABLED: return "MH_ERROR_DISABLED";
	case MH_ERROR_NOT_EXECUTABLE: return "MH_ERROR_NOT_EXECUTABLE";
	case MH_ERROR_UNSUPPORTED_FUNCTION: return "MH_ERROR_UNSUPPORTED_FUNCTION";
	case MH_ERROR_MEMORY_ALLOC: return "MH_ERROR_MEMORY_ALLOC";
	case MH_ERROR_MEMORY_PROTECT: return "MH_ERROR_MEMORY_PROTECT";
	case MH_ERROR_MODULE_NOT_FOUND: return "MH_ERROR_MODULE_NOT_FOUND";
	case MH_ERROR_FUNCTION_NOT_FOUND: return "MH_ERROR_FUNCTION_NOT_FOUND";
	default: return "unknown";
	}
}

gm::gmHook::gmHook()
{
	MH_STATUS status = MH_Initialize();

	if (status == MH_OK)
	{
		g_Log.LogT("gmHook::gmHook() -> MinHook initialized with status OK");
		return;
	}

	g_Log.LogE("gmHook::gmHook() -> MinHook failed to initialize with status {}", static_cast<int>(status));
}

gm::gmHook::~gmHook()
{
	g_Log.LogT("~gmHook()");

	for (auto& func : m_hookedFuncs)
	{
		MH_STATUS disableHook = MH_DisableHook(func);
		MH_STATUS removeHook = MH_RemoveHook(func);

		if (disableHook == MH_OK && removeHook == MH_OK)
		{
			g_Log.LogD("UnHook: {:X} finished with status OK", reinterpret_cast<uintptr_t>(func));
		}
		else
		{
			g_Log.LogD("UnHook: {:X} failed with statuses: {}, {}",
				reinterpret_cast<uintptr_t>(func),
				GetMHStatusStr(disableHook),
				GetMHStatusStr(removeHook));
		}
	}
	m_hookedFuncs.clear();

	MH_Uninitialize();
}

void gm::gmHook::UnHook(LPVOID addr)
{
	MH_DisableHook(addr);
	MH_RemoveHook(addr);
	m_hookedFuncs.erase(addr);
}
