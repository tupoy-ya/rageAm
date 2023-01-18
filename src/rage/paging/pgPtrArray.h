#pragma once
#include "pgArray.h"

namespace rage
{
	/**
	 * \brief Array of pointers (for e.g. rage::grcTexture*) for resource binaries.
	 * \tparam T Pointer type to store. (without pointer itself). e.g. rage::grcTexture
	 */
	template <typename T>
	class pgPtrArray : public pgArray<T*>
	{
	public:
		using pgArray<T*>::pgArray;

		pgPtrArray(const datResource& rsc) : pgArray<T*>(rsc)
		{
			for (u16 i = 0; i < this->GetSize(); i++)
				rsc.Place(this->Get(i));
		}
	};
}
