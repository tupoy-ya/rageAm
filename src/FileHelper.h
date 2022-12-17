#pragma once
#include "Windows.h"

/**
 * \brief Creates 'rageAm' directory in GTA root folder and sub folders.
 */
inline void EnsureDataFoldersExist()
{
	CreateDirectoryW(L"rageAm", nullptr);
	CreateDirectoryW(L"rageAm/Textures", nullptr);
	CreateDirectoryW(L"rageAm/Shaders", nullptr);
}
