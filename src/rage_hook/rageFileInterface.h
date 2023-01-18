#pragma once
#include "gmHelper.h"
#include "../rage/fiPackfile.h"
#include "../rage/fiDevice.h"

namespace rh
{
	class fiPackfile
	{
	public:
		fiPackfile()
		{
			/*gm::ScanAndHook(
				"fiPackfile::GetEntry",
				"48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 81 EC 00 01 00 00 4C",
				rage::fiPackfile::vftable_FindEntryHeaderByPath);*/
		}
	};

	class fiDevice
	{
		typedef rage::fiDevice* (*gDef_GetDeviceImpl)(const char* path, bool isReadOnly);

		static inline gDef_GetDeviceImpl gImpl_GetDeviceImpl;

		static rage::fiDevice* aImpl_GetDeviceImpl(const char* path, bool isReadOnly)
		{
			return gImpl_GetDeviceImpl(path, isReadOnly);
		}
	public:
		fiDevice()
		{
			//gm::ScanAndHook("fiDevice::GetDeviceImpl", 
			//	"48 89 5C 24 08 88 54 24 10 55 56 57 41 54 41 55 41 56 41 57 48 83",
			//	aImpl_GetDeviceImpl,
			//	&gImpl_GetDeviceImpl);
		}
	};

	inline fiPackfile g_FiPackfile;
	inline fiDevice g_FiDevice;
}
