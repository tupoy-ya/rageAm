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

gm::gmAddress gm::gmAddress::GetAt(int offset) const
{
	if (!MayBeValid())
		return gmAddress(0);

	return gmAddress(m_address + offset);
}
