#pragma once

#include "fwTypes.h"
// #include "datResource.h"

namespace rage
{
	struct datResource;

	class pgBase
	{
		u64 qword8;

	protected:
		static const datResource* GetResource();

	public:
		pgBase();
		virtual ~pgBase() = default;
	};
	static_assert(sizeof(pgBase) == 0x10);
}
