#pragma once

#include <cstdio>
#include <vector>
#include "windows.h"
#include "winver.h"
#include "Logger.h"

#pragma comment(lib, "Version.lib")

constexpr uint16_t VER_2699 = 2699;
constexpr uint16_t VER_2802 = 2802;

class GameVersion
{
	static inline char sm_GameVersion[16]{};
	static inline std::vector sm_SupportedVersions =
	{
		"1.0.2699.16",
		"1.0.2802.0",
	};
	static inline uint16_t sm_Version = 0;
	static inline uint16_t sm_SubVersion = 0;

	static bool CheckGameVersion()
	{
		for (const char* version : sm_SupportedVersions)
		{
			if (strcmp(version, sm_GameVersion) == 0)
			{
				AM_TRACEF("Supported game version found: {}", version);
				return true;
			}
		}
		AM_TRACEF("Unsupported game version found: {}", sm_GameVersion);
		return false;
	}

	static bool GetGameVersion()
	{
		const char* path = "GTA5.exe";

		DWORD versionInfoBufferSize = GetFileVersionInfoSizeA(path, nullptr);

		char* versionInfoBuffer = new char[versionInfoBufferSize];
		GetFileVersionInfoA(path, 0, versionInfoBufferSize, versionInfoBuffer);

		VS_FIXEDFILEINFO* pVsInfo = nullptr;
		UINT vsInfoSize = sizeof(VS_FIXEDFILEINFO);
		bool ok = VerQueryValueA(versionInfoBuffer, "\\", (LPVOID*)&pVsInfo, &vsInfoSize);
		if (ok)
		{
			sprintf_s(sm_GameVersion, sizeof sm_GameVersion, "%d.%d.%d.%d",
				HIWORD(pVsInfo->dwFileVersionMS),
				LOWORD(pVsInfo->dwFileVersionMS),
				HIWORD(pVsInfo->dwFileVersionLS),
				LOWORD(pVsInfo->dwFileVersionLS));

			sm_Version = HIWORD(pVsInfo->dwFileVersionLS);
			sm_SubVersion = LOWORD(pVsInfo->dwFileVersionLS);
		}
		else
		{
			AM_ERR("Unable to determine game version. Further behaviour is undefined.");
		}
		delete[] versionInfoBuffer;
		return ok;
	}
public:
	GameVersion()
	{
		if (!GetGameVersion())
			return;

		CheckGameVersion();
	}

	static bool IsGreaterOrEqual(uint16_t version, uint16_t sub = 0)
	{
		// Greater version will have higher subversion in any case
		if (sm_Version == version)
			return sm_Version >= version && sm_SubVersion >= sub;
		return sm_Version >= version;
	}
};
inline GameVersion g_GameVersion;
