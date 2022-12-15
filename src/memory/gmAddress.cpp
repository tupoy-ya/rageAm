#include "gmAddress.h"
#include "pattern.h"

bool gm::gmAddress::MayBeValid() const
{
	return !IsBadReadPtr(reinterpret_cast<void*>(m_address), 0x8);
}

gm::gmAddress gm::gmAddress::GetRef() const
{
	if (!MayBeValid())
		return gmAddress(0);

	return gmAddress(FindRef(m_address));
}

gm::gmAddress gm::gmAddress::GetCall() const
{
	return GetAt(0x1).GetRef();
}

gm::gmAddress gm::gmAddress::GetAt(uint32_t offset) const
{
	return GetAt64(offset);
}

gm::gmAddress gm::gmAddress::GetAt64(uint64_t offset) const
{
	if (!MayBeValid())
		return {0};

	return {m_address + offset};
}
