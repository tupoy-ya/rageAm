#include "datAllocator.h"

#include <cstdlib>
#include <cstring>

#include "helpers/align.h"
#include "paging/dat.h"

u32 rage::datAllocator::Header::GetGuard() const
{
	return reinterpret_cast<u32>(this); // NOLINT(clang-diagnostic-pointer-to-int-cast)
}

rage::datAllocator::Header::Header(u32 size)
{
	m_Guard = GetGuard();
	m_Size = size;
}

bool rage::datAllocator::Header::IsValid() const
{
	return m_Guard == GetGuard();
}

void rage::datAllocator::Header::AddRef(void** pRef)
{
	m_Refs.push_back(pRef);
}

void rage::datAllocator::Header::FixupRefs(u64 newAddress) const
{
	for (void** pRef : m_Refs)
		*pRef = reinterpret_cast<void*>(newAddress);
}

void* rage::datAllocator::Header::GetBlock() const
{
	return (char*)this + sizeof(Header);  // NOLINT(clang-diagnostic-cast-qual)
}

rage::datAllocator::Header* rage::datAllocator::Header::Next() const
{
	Header* pHeader = reinterpret_cast<Header*>((char*)this + m_Size); // NOLINT(clang-diagnostic-cast-qual)
	if (!pHeader->IsValid())
		return nullptr;
	return pHeader;
}

u32 rage::datAllocator::Header::GetSize() const
{
	return m_Size;
}

void* rage::datAllocator::GetHeapAt(u32 offset) const
{
	return reinterpret_cast<void*>(GetHeap64() + offset);
}

u64 rage::datAllocator::GetHeap64() const
{
	return reinterpret_cast<u64>(m_Heap);
}

u32 rage::datAllocator::GetHeaderOffset(const Header* pHeader) const
{
	return (u32)(GetHeap64() - (u64)pHeader);
}

rage::datAllocator::Header* rage::datAllocator::GetHeaderFromAddress(void* addr) const
{
	u64 headerAddr = reinterpret_cast<u64>(addr) - sizeof(Header);
	Header* pHeader = reinterpret_cast<Header*>(headerAddr);

	if (!pHeader->IsValid())
		return nullptr;
	return pHeader;
}

void* rage::datAllocator::DoAllocate(u32 size, u32 align)
{
	m_Offset = Align(m_Offset, align);

	Header* header = new (GetHeapAt(m_Offset)) Header(size);

	m_Offset += size + sizeof(Header);
	m_Headers.push_back(header);

	return header->GetBlock();
}

rage::datAllocator::datAllocator(u32 size, bool isVirtual)
{
	m_Heap = malloc(size);
	m_IsVirtual = isVirtual;
	m_Offset = 0;

	if (!m_Heap)
		throw;

	memset(m_Heap, 0, size);
}

rage::datAllocator::~datAllocator()
{
	free(m_Heap);
}

void* rage::datAllocator::Allocate(size_t size, size_t align)
{
	u32 size32 = static_cast<u32>(size);
	u32 align32 = static_cast<u32>(align);

	return DoAllocate(size32, align32);
}

void rage::datAllocator::GetChunks(std::vector<datAllocatorChunk>& outChunks) const
{
	for (const Header* header : m_Headers)
		outChunks.emplace_back(header->GetSize());
}

void rage::datAllocator::FixupRefs(u16 chunkIndex, u64 newAddress) const
{
	Header* header = m_Headers[chunkIndex];
	header->FixupRefs(newAddress);
}

u64 rage::datAllocator::GetBaseAddress() const
{
	return IsVirtual() ? DAT_VIRTUAL_BASE : DAT_PHYSICAL_BASE;
}

char* rage::datAllocator::GetChunkData(u16 index) const
{
	return static_cast<char*>(m_Headers[index]->GetBlock());
}

u32 rage::datAllocator::GetChunkSize(u16 index) const
{
	return m_Headers[index]->GetSize();
}

bool rage::datAllocator::IsVirtual() const
{
	return m_IsVirtual;
}
