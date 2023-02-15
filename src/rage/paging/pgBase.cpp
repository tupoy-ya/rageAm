#include "pgBase.h"

#include "Logger.h"
#include "TlsManager.h"
#include "datResource.h"
#include "system/assert.h"

void rage::pgBase::Map::GenerateFromMap(const datResourceMap& map)
{
	VirtualChunkCount = map.VirtualChunkCount;
	PhysicalChunkCount = map.PhysicalChunkCount;
	MainChunkIndex = map.MainChunkIndex;

	bUnknown = false;

	for (u8 i = 0; i < map.GetChunkCount(); i++)
	{
		const datResourceChunk& chunk = map.Chunks[i];

		// NOTE: Destination address has to be aligned to 16 for this to work!
		// Explanation:
		// If value is multiple of 16, lowest 8 bits are always all null.
		AddressAndShift[i] = chunk.DestAddr | chunk.GetSizeShift();
	}
}

void rage::pgBase::Map::RegenerateMap(datResourceMap& map) const
{
	map.PhysicalChunkCount = 0;
	map.VirtualChunkCount = VirtualChunkCount;

	for (u8 i = 0; i < VirtualChunkCount; i++)
	{
		u64 address = AddressAndShift[i] & MAP_ADDRESS_MASK;
		u8 shift = AddressAndShift[i] & MAP_SIZE_SHIFT_MASK;

		map.Chunks[i] = datResourceChunk(address, address, DAT_MIN_CHUNK_SIZE << shift);
	}

	map.MainChunkIndex = MainChunkIndex;
	map.MainPage = reinterpret_cast<pgBase*>(map.Chunks[MainChunkIndex].DestAddr);
}

void rage::pgBase::MakeDefragmentable(const datResourceMap& map) const
{
	if (!HasMap())
		return;

	m_Map->GenerateFromMap(map);
}

void rage::pgBase::FreeMemory(const datResourceMap& map) const
{
	if (m_Map->bUnknown)
		return;

	// TODO: Virtual allocator (1)

	for (u8 i = 0; i < map.VirtualChunkCount; i++)
		delete reinterpret_cast<void*>(map.Chunks[i].DestAddr);
}

rage::pgBase::pgBase()
{
	datResource* rsc = TlsManager::GetResource();

	if (rsc && AM_ASSERT(m_Map == nullptr, "Builded resource has no internal map."))
	{
		rsc->Fixup(m_Map);
		MakeDefragmentable(*rsc->Map);
		return;
	}
	m_Map = nullptr;
}

rage::pgBase::~pgBase()
{
	Destroy();
}

rage::pgBase::pgBase(const pgBase& other)
{
	// Just like in atArray, we have to allocate memory for
	// mini map (which presents only in compiled / file resources) manually.
	datAllocator* pAllocator = TlsManager::GetVirtualAllocator();
	if (!pAllocator)
	{
		// We're not compiling resource so no need to have a map.
		m_Map = nullptr;
		return;
	}
	pAllocator->AllocateRef(m_Map);
}

void rage::pgBase::Destroy() const
{
	AM_TRACEF("pgBase::Destroy()");
	if (!HasMap())
		return;

	datResourceMap map;
	RegenerateMap(map);
	FreeMemory(map);
}

bool rage::pgBase::HasMap() const
{
	return m_Map != nullptr && m_Map->bUnknown == false;
}

void rage::pgBase::RegenerateMap(datResourceMap& map) const
{
	if (!HasMap())
		return;

	m_Map->RegenerateMap(map);
}
