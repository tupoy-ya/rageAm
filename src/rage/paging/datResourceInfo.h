#pragma once
#include "datResourceMap.h"
#include "helpers/align.h"

#ifndef RAGE_STANDALONE
#include "gmFunc.h"
#endif

namespace rage
{
	/**
	 * \brief Resource info is resource header that contains encoded virtual and physical chunks.
	 */
	struct datResourceInfo
	{
		u32 VirtualData;
		u32 PhysicalData;

		static u8 GetSizeShift(u32 data);
		static u8 GetNumChunks(u32 data);

		/**
		 * \brief Generates chunk definitions from encoded data.
		 * \param data Encoded data. (Virtual or Physical).
		 * \param baseSize Default page size, see dat.h;
		 * \param map Destination map.
		 * \param startChunk Number of chunks to skip in map.
		 * \param baseAddr Chunk address base, see dat.h;
		 * \return Count of generated chunks.
		 */
		static u8 GenerateChunks(u32 data, u64 baseSize, datResourceMap& map, u8 startChunk, u64 baseAddr);

		u8 GetNumVirtualChunks() const;
		u8 GetNumPhysicalChunks() const;

		/**
		 * \brief Decodes virtual and physical data into resource offset map.
		 * \param map Destination map.
		 */
		void GenerateMap(datResourceMap& map) const;
	};

	namespace hooks
	{
#ifndef RAGE_STANDALONE
		inline gm::gmFunc<void, const datResourceInfo*, datResourceMap*> gImpl_datResourceInfo_GenerateMap;
#endif

		static void RegisterResourceInfo()
		{
#ifndef RAGE_STANDALONE
#ifdef RAGE_HOOK_SWAP_DATRESOURCE
			gImpl_datResourceInfo_GenerateMap = gm::Hook(
				"rage::datResourceInfo::GenerateMap",
				"48 89 5C 24 08 57 48 83 EC 30 44 8B 09",
				&datResourceInfo::GenerateMap);
#endif
#endif
		}
	}
}
