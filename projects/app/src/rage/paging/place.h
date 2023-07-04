#pragma once

#include "helpers/macro.h"
#include "resource.h"

/**
 * \brief Implements functions for tracking object reference count.
 * \n NOTE: Object must have 'm_RefCount' field for compatibility with native rage structs.
 */
#define IMPLEMENT_REF_COUNTER(name_) \
	void AddRef() { \
		_InterlockedIncrement(&m_RefCount); \
		AM_DEBUGF("AddRef(%s, %#llx)", typeid(this).name(), (u64)this); \
	} \
	u32 Release() { \
		AM_DEBUGF("Release(%s, %#llx) -> RefCount: %u", typeid(this).name(), (u64)(this), m_RefCount - 1); \
		u32 refs = _InterlockedDecrement(&m_RefCount); \
		if(refs == 0) this->~name_(); \
		return refs; \
	} \
	u32 GetNumRefs() const { return m_RefCount; } \
	MACRO_END

/**
 * \brief Implements place / allocate functions and new / delete operator overloads for resource building.
 * \param name Name of resource class.
 */
#define IMPLEMENT_PLACE_INLINE(name) \
		void* operator new(std::size_t size) \
		{ \
			if(const rage::datResource* rsc = rage::datResource::GetCurrent()) \
				return rsc->Map->MainChunk; \
			return ::operator new(size); \
		} \
		void* operator new(std::size_t size, void* where) \
		{ \
			return ::operator new(size, where); \
		} \
		void operator delete(void* block) \
		{ \
			if(!rage::datResource::GetCurrent()) ::delete(block); \
		} \
		static name* Place(const rage::datResource& rsc, name* that) \
		{ \
			rsc.Place(that); \
			return that; \
		} \
		static name* Allocate() \
		{ \
			return new (name)(); \
		} \
		MACRO_END
