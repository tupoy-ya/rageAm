#include "gmAddress.h"
#include "gmHelper.h"
#include "gmScanner.h"

bool gm::gmAddress::MayBeValid() const
{
	return !IsBadReadPtr((void*)Addr, 0x8);
}

gm::gmAddress gm::gmAddress::GetRef() const
{
	if (!MayBeValid())
		return {};

	// Offset m_address + size of offset (int) + offset value
	return Addr + 4 + *(int*)Addr;
}

gm::gmAddress gm::gmAddress::GetCall() const
{
	return GetAt(0x1).GetRef();
}

gm::gmAddress gm::gmAddress::GetAt(int32_t offset) const
{
	return GetAt64(offset);
}

gm::gmAddress gm::gmAddress::GetAt64(int64_t offset) const
{
	if (!MayBeValid())
		return {};

	return { Addr + offset };
}
