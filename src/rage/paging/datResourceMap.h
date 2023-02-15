#pragma once
#include <cstddef>

#include "datResourceChunk.h"
#include "pgBase.h"
#include "template/iterator.h"

namespace rage
{
	class pgBase;

	/**
	 * \brief Offset map decoded from data in rage::datResourceInfo.
	 * \n This is the 'first' stage of resource reading.
	 */
	struct datResourceMap
	{
		/**
		 * \brief Number of virtual chunks. (Which stored in RAM).
		 * For e.g. pgDictionary, grmDrawable, grmShaderGroup.
		 */
		u8 VirtualChunkCount;

		/**
		 * \brief Number of physical chunks. (GPU Data). For e.g. texture or model data.
		 * De-allocated instantly after placing (constructing) resource.
		 */
		u8 PhysicalChunkCount;

		/**
		 * \brief Index of main page. In GTA IV / V it's always 0.
		 */
		u8 MainChunkIndex;

		/**
		 * \brief Main page of resource. Can be seen as root structure / class.
		 */
		pgBase* MainPage;

		/**
		 * \brief Virtual and physical chunks (or pages).
		 */
		datResourceChunk Chunks[DAT_NUM_CHUNKS];

		u64 qwordC10; // Never used.

		/**
		 * \brief Gets total number of chunks (virtual + physical).
		 */
		u8 GetChunkCount() const { return VirtualChunkCount + PhysicalChunkCount; }

		atIterator<datResourceChunk> begin() { return Chunks; }
		atIterator<datResourceChunk> end() { return Chunks + GetChunkCount(); }
	};
	static_assert(sizeof(datResourceMap) == 0xC18);
	static_assert(offsetof(datResourceMap, Chunks) == 0x10);
}
