#pragma once
#include "pgArray.h"

namespace rage
{
	/**
	 * \brief Simple array of pointers.
	 * \tparam T Pointer type to store.
	 */
	template <typename T>
	class pgPtrArray : public pgArray<T*>
	{
	public:
		using pgArray<T*>::pgArray;

		pgPtrArray(const datResource& rsc) : pgArray<T*>(rsc)
		{
			for (u16 i = 0; i < this->GetSize(); i++)
				rsc.Fixup(this->Get(i));
		}
	};
}
