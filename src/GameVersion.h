#pragma once

#include <cstdio>
#include "windows.h"
#include "winver.h"
#include <vector>

#pragma comment(lib, "Version.lib")

constexpr uint16_t VER_2699 = 2699;
constexpr uint16_t VER_2802 = 2802;

class GameVersion
{
	static inline char ms_GameVersion[16]{};
	static inline std::vector ms_SupportedVersions =
	{
		"1.0.2699.16",
		"1.0.2802.0",
	};
	static inline uint16_t ms_Version = 0;
	static inline uint16_t ms_SubVersion = 0;

	static bool CheckGameVersion()
	{
		for (const char* version : ms_SupportedVersions)
		{
			if (strcmp(version, ms_GameVersion) == 0)
			{
				tracef("Supported game version found: {}", version);
				return true;
			}
		}
		tracef("Unsupported game version found: {}", ms_GameVersion);
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
			sprintf_s(ms_GameVersion, sizeof ms_GameVersion, "%d.%d.%d.%d",
				HIWORD(pVsInfo->dwFileVersionMS),
				LOWORD(pVsInfo->dwFileVersionMS),
				HIWORD(pVsInfo->dwFileVersionLS),
				LOWORD(pVsInfo->dwFileVersionLS));

			ms_Version = HIWORD(pVsInfo->dwFileVersionLS);
			ms_SubVersion = LOWORD(pVsInfo->dwFileVersionLS);
		}
		else
		{
			g_Log.LogE("Unable to determine game version. Further behaviour is undefined.");
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
		if (ms_Version == version)
			return ms_Version >= version && ms_SubVersion >= sub;
		return ms_Version >= version;
	}
};
inline GameVersion g_GameVersionMgr;
