#pragma once
#include "fwTypes.h"

namespace rage
{
	/**
	 * \brief Simple pointer iterator. Alternative to std::iterator;
	 * \tparam T Type of pointer.
	 */
	template<typename T>
	class atIterator
	{
		T* m_Ptr;
	public:
		atIterator(T* ptr) { m_Ptr = ptr; }

		atIterator Mid(const atIterator& rhs) { return m_Ptr + (rhs.m_Ptr - m_Ptr) / 2; }
		atIterator operator	++() { ++m_Ptr; return *this; }
		atIterator operator	--() { --m_Ptr; return *this; }
		atIterator operator	- (const atIterator& rhs) { return { m_Ptr - rhs.m_Ptr }; }
		bool operator <		(const T& rhs)	const { return *m_Ptr < rhs; }
		bool operator >		(const T& rhs)	const { return *m_Ptr > rhs; }
		bool operator <=	(const T& rhs)	const { return *m_Ptr <= rhs; }
		bool operator >=	(const T& rhs)	const { return *m_Ptr >= rhs; }
		bool operator ==	(const T& rhs)	const { return *m_Ptr == rhs; }
		bool operator ==	(const atIterator& rhs) const { return m_Ptr == rhs.m_Ptr; }
		bool operator <		(const atIterator& rhs) const { return m_Ptr < rhs.m_Ptr; }
		bool operator >		(const atIterator& rhs) const { return m_Ptr > rhs.m_Ptr; }
		bool operator <=	(const atIterator& rhs) const { return m_Ptr <= rhs.m_Ptr; }
		bool operator >=	(const atIterator& rhs) const { return m_Ptr >= rhs.m_Ptr; }
		T& operator*() { return *m_Ptr; }
		operator T* () { return m_Ptr; }
	};
}
