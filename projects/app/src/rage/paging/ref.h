#pragma once

#include <algorithm>
#include "helpers/com.h"

namespace rage
{
	/**
	 * \brief Container for paged objects that automatically count refs and destructs object.
	 */
	template<typename T>
	class pgRef
	{
		using TPtr = T*;

		TPtr m_Pointer;
	public:
		pgRef(TPtr pointer) { m_Pointer = pointer; }
		pgRef() : pgRef(nullptr) {}
		pgRef(const pgRef& other) : pgRef(other.m_Pointer) { SAFE_ADDREF(other.m_Pointer); }
		pgRef(pgRef&& other) noexcept : pgRef() { std::swap(m_Pointer, other.m_Pointer); }
		~pgRef() { SAFE_RELEASE(m_Pointer); }

		TPtr& Get() { return m_Pointer; }

		pgRef& operator=(const pgRef& other)
		{
			if (m_Pointer != other.m_Pointer)
			{
				SAFE_RELEASE(m_Pointer);
				m_Pointer = other.m_Pointer;
				SAFE_ADDREF(m_Pointer);
			}
			return *this;
		}

		pgRef& operator=(pgRef&& other) noexcept
		{
			std::swap(m_Pointer, other.m_Pointer);
			return *this;
		}

		pgRef& operator=(const TPtr pointer)
		{
			if (m_Pointer != pointer)
			{
				SAFE_RELEASE(m_Pointer);
				m_Pointer = pointer;
			}
			return *this;
		}
		TPtr operator->() { return m_Pointer; }
		operator T*& () { return m_Pointer; }
	};
}
