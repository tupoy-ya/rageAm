#pragma once
#include "Windows.h"

/**
 * \brief Creates 'rageAm' directory in GTA root folder
 */
inline void CreateDataFolderIfNotExists()
{
	CreateDirectoryW(L"rageAm", nullptr);
}
