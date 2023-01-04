#pragma once
#include "Windows.h"

inline void CreateDefaultFolders()
{
	CreateDirectoryW(L"rageAm", nullptr);
	CreateDirectoryW(L"rageAm/Textures", nullptr);
	CreateDirectoryW(L"rageAm/Shaders", nullptr);
	CreateDirectoryW(L"rageAm/Data", nullptr);
}
