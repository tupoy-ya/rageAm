#pragma once

#include "datResourceMap.h"

namespace rage
{
	struct datResourceMap;

	/**
	 * \brief Base class for every paged resource.
	 * \n For more details on how this system works, see dat.h;
	 */
	class pgBase
	{
		/**
		 * \brief Mini copy of datResourceMap.
		 * \n Purpose: We have to store allocated chunks to de-allocate them when object is destroyed.
		 * \n Full datResourceMap contains redundant information for deallocation so this is a 'mini' version of it.
		 */
		struct Map
		{
			// Lowest 8 bits
			static constexpr u64 MAP_SIZE_SHIFT_MASK = 0xF;

			// Highest 56 bits
			static constexpr u64 MAP_ADDRESS_MASK = ~0xF;

			// According to Companion. Purpose is unknown.
			u32* MetaData;

			u8 VirtualChunkCount;
			u8 PhysicalChunkCount;
			u8 MainChunkIndex;

			// Whether this map is allocated dynamically or generated from compiled file.
			// NOTE: This could be wrong name / interpretation because value is never true
			// and most likely used only in 'compiler' builds of rage, just like metadata.
			// Name is only assumed from these facts:
			// - Resource is de-allocated only if value is false;
			// - 'HasMap' function does check for value being false; (can be seen as resource owning the map)
			// - It's placed right above the chunks.
			bool bIsDynamic;

			// In comparison with datResource, contains only destination (allocated) addresses.
			u64 AddressAndShift[128];

			/**
			 * \brief Whether this map was built from compiled resource.
			 */
			bool IsCompiled() const;

			/**
			 * \brief Generates mini map from original one.
			 */
			void GenerateFromMap(const datResourceMap& map);

			/**
			 * \brief Recreates resource map from this mini map.
			 */
			void RegenerateMap(datResourceMap& map) const;
		};

		Map* m_Map;

		// Not sure what is exact meaning behind this name.
		// Generates mini map from full one.
		void MakeDefragmentable(const datResourceMap& map) const;

		// Performs de-allocation of all memory chunks that
		// are owned by this resource.
		// NOTE: This includes main chunk or 'this';
		// Which means: after this function is invoked,
		// accessing 'this' will cause undefined behaviour.
		void FreeMemory(const datResourceMap& map) const;
	public:
		pgBase();
		virtual ~pgBase();

		// Compiler constructor.
		pgBase(const pgBase& other);

		/**
		 * \brief De-allocates resource memory.
		 */
		void Destroy() const;

		/**
		 * \brief Whether this paged object has internal resource map,
		 * which means that this resource is compiled and was built from file.
		 */
		bool HasMap() const;

		/**
		 * \brief Recreates resource map of this resource.
		 * \n If resource has no map, nothing is done.
		 */
		void RegenerateMap(datResourceMap& map) const;

		// Unused in final version.
		// They're present only for virtual table completeness.

		virtual u32 GetHandleIndex() { return 0; }
		virtual void SetHandleIndex(u32 index) { }
	};
	static_assert(sizeof(pgBase) == 0x10);
}
