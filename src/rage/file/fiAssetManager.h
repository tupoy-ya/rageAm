#pragma once

#include "gmFunc.h"

namespace rage
{
	class fiAssetManager;

	namespace hooks
	{
		static inline gm::gmAddress gPtr_fiAssetManager;
		static inline gm::gmFunc<void, fiAssetManager*, char*, u32, const char*, const char*, u32> gImpl_FiAssetManager_FullPath;
		static inline gm::gmFunc<bool, fiAssetManager*, char*, u32, const char*, const char*> gImpl_FiAssetManager_FullReadPath;

		static void RegisterAssetManager()
		{
			// lea rcx, g_fiAssetManager
			gPtr_fiAssetManager = gm::Scan(
				"rage::pgRscBuilder::ConstructName",
				"48 89 5C 24 10 57 48 81 EC 40")
				.GetAt(0x79 + 0x3)
				.GetRef();

			gImpl_FiAssetManager_FullPath.SetTo(gm::Scan(
				"rage::fiAssetManager::FullPath",
				"48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 56 48 83 EC 40 48 8B FA 49"));
		}
	}

	class fiAssetManager
	{
	public:
		static fiAssetManager* GetInstance()
		{
			return hooks::gPtr_fiAssetManager.Cast<fiAssetManager*>();
		}

		void FullPath(char* dest, u32 maxLength, const char* path, const char* extension, int rootIndex = 0)
		{
			hooks::gImpl_FiAssetManager_FullPath(this, dest, maxLength, path, extension, rootIndex);
		}

		bool FullReadPath(char* dest, u32 maxLength, const char* path, const char* extension)
		{
			return hooks::gImpl_FiAssetManager_FullReadPath(this, dest, maxLength, path, extension);
		}
	};
}
