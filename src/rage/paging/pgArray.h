#pragma once

#include "template/array.h"
#include "paging/datResource.h"

namespace rage
{
	/**
	 * \brief Simple array for resource binaries.
	 * \remark For pointer, see rage::pgPtrArray;
	 * \tparam T Type to store.
	 */
	template<typename T>
	class pgArray : public atArray<T>
	{
	public:
		using atArray<T>::atArray;

		pgArray(const datResource& rsc) : atArray<T>(rsc)
		{
			
		}
	};
}
