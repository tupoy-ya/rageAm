#pragma once

#include "datHelper.h"
#include "datResourceMap.h"
#include "fwTypes.h"

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
		 * \n Full datResourceMap contains redundant information so this is a 'mini' version of it.
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

			// Always false.
			// Could be related to metadata / owns map.
			bool bUnknown;

			u64 AddressAndShift[128];

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
