#include "main.h"
#include <windows.h>
#include <fstream>
#include <format>

#include "Memory/Hooking.h"
#include "Logger.h"
#include "../vendor/minhook-1.3.3/include/MinHook.h"
#include "../vendor/scripthook/include/main.h"

typedef bool(*SettingMgr__Save)();
typedef bool(*SettingMgr__BeginSave)(uintptr_t a1);

typedef int(*WriteDebugStateToFile)(const WCHAR* fileName);

SettingMgr__Save gimpl_SettingMgr__Save;
WriteDebugStateToFile gimpl_WriteDebugStateToFile;

uintptr_t save;
uintptr_t beginSave;
uintptr_t beginSave_setting_64;
uintptr_t beginSave_settingDump;
uintptr_t writeDebugStateToFile;

bool __fastcall aimpl_SettingMgr__BeginSave(uintptr_t a1)
{
	g_logger->Log(std::format("SettingMgr::Save({:x})", a1));

	int v1; // edx

	v1 = 203;
	if (*(unsigned __int16*)(a1 + 64) < 203u) // a1 + 64 seems to be always on 350
		v1 = *(unsigned __int16*)(a1 + 64);
	*(int*)beginSave_setting_64 = v1;
	memmove((void*)beginSave_settingDump, *(const void**)(a1 + 56), 8i64 * v1);
	return gimpl_SettingMgr__Save();
}

void Main()
{
	g_logger->Log("Init rageAm", true);
	g_logger->Log(std::format("MH_Initialize: {}", MH_Initialize() == MH_OK));

	g_logger->Log("Scanning patterns...");

	save = g_hook->FindPattern("SettingMgr::Save", "48 83 EC 48 48 83 3D");
	beginSave = g_hook->FindPattern("SettingMgr::BeginSave", "40 53 48 83 EC 20 0F B7 41 40");
	beginSave_setting_64 = g_hook->FindOffset("SettingMgr::BeginSave_setting64", beginSave + 0x1C);
	beginSave_settingDump = g_hook->FindOffset("SettingMgr::BeginSave_settingDump", beginSave + 0x27);
	writeDebugStateToFile = g_hook->FindPattern("WriteDebugStateToFile", "48 83 EC 48 48 83 64 24 30 00 83 64 24 28 00 45");

	gimpl_SettingMgr__Save = (SettingMgr__Save)save;
	gimpl_WriteDebugStateToFile = (WriteDebugStateToFile)writeDebugStateToFile;

	g_hook->SetHook((LPVOID)beginSave, &aimpl_SettingMgr__BeginSave);

	gimpl_WriteDebugStateToFile(L"victor.txt");
}

void Abort()
{
	Hooker::GetInstance()->UnHookAll();
	MH_Uninitialize();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH: scriptRegister(hModule, Main); break;
	case DLL_PROCESS_DETACH: Abort(); scriptUnregister(hModule); break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:break;
	}

	return TRUE;
}
