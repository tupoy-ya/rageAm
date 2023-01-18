#pragma once
#include "datResourceMap.h"
//#include "gmFunc.h"

namespace rage
{
	struct datResourceInfo
	{
		u32 VirtualData;
		u32 PhysicalData;

		static u8 GetSizeShift(u32 data);
		static u8 GetNumChunks(u32 data);
		static u8 GenerateChunks(u32 data, u64 baseSize, datResourceMap& map, u8 startChunk, u64 baseAddr);

		u8 GetNumVirtualChunks() const;
		u8 GetNumPhysicalChunks() const;

		void GenerateMap(datResourceMap& map) const;
	};

//	namespace hooks
//	{
//		inline gm::gmFunc<void, const datResourceInfo*, datResourceMap*> gImpl_datResourceInfo_GenerateMap;
//
//		//static inline gm::gmFuncHook gSwap_datResourceInfo_GenerateMap(
//		//	"rage::datResourceInfo::GenerateMap",
//		//	"48 89 5C 24 08 57 48 83 EC 30 44 8B 09",
//		//	&datResourceInfo::GenerateMap,
//		//	&gImpl_datResourceInfo_GenerateMap);
//
//		static void RegisterResource()
//		{
//#ifdef RAGE_HOOK_SWAP_DATRESOURCE
//
//			gImpl_datResourceInfo_GenerateMap = gm::Hook("rage::datResourceInfo::GenerateMap",
//				"48 89 5C 24 08 57 48 83 EC 30 44 8B 09",
//				&datResourceInfo::GenerateMap);
//#endif
//	}
//	}
}
