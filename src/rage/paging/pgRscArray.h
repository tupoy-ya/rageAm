#pragma once
#include "pgArray.h"

namespace rage
{
	/**
	 * \brief Array of pointers on resource classes (for e.g. rage::grcTexture*).
	 * \tparam T Pointer type to store. (without pointer itself). e.g. rage::grcTexture
	 */
	template <typename T>
	class pgRscArray : public pgArray<T*>
	{
	public:
		using pgArray<T*>::pgArray;

		pgRscArray(const datResource& rsc) : pgArray<T*>(rsc)
		{
			for (u16 i = 0; i < this->GetSize(); i++)
				rsc.Place(this->Get(i));
		}
	};
}
