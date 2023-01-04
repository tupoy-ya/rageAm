#include "gmAddress.h"
#include "pattern.h"

bool gm::gmAddress::MayBeValid() const
{
	return !IsBadReadPtr(reinterpret_cast<void*>(m_Address), 0x8);
}

gm::gmAddress gm::gmAddress::GetRef() const
{
	if (!MayBeValid())
		return { nullptr };

	return { FindRef(m_Address) };
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
		return { nullptr };

	return { m_Address + offset };
}
