#pragma once

// TODO: Inheritance
#include "../fiPackfile.h"

namespace rage::hooks
{
	static gm::gmAddress gPtr_fiCollection_sm_Collections = gm::Scan([]() -> uintptr_t
		{
			// lea r9, rage::fiCollection::sm_Collections
			gm::gmAddress addr = gm::Scan("rage::fiPackfile::ctor", "44 88 51 08 4C 89 41 10")
				.GetAt(-0x12)
				.GetAt(0x35 + 0x3)
				.GetRef();

			AM_TRACEF("rage::fiCollection::sm_Collections found at: {:X}", addr.GetAddress());
			return addr;
		});
}

namespace rage
{
	static constexpr u16 FI_COLLECTIONS_SLOT_COUNT = 3672;

	class fiCollection
	{
		inline static fiPackfile** sm_Collections = hooks::gPtr_fiCollection_sm_Collections.Cast<fiPackfile**>();

	public:
		static u16 GetCollectionsSlotCount()
		{
			return FI_COLLECTIONS_SLOT_COUNT;
		}

		static fiPackfile* GetCollection(u16 index)
		{
			return sm_Collections[index];
		}
	};
}
