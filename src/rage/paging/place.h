#pragma once

#include "macro.h"

// Used by macro
#include "TlsManager.h"
#include "paging/compiler/pgCompiler.h"

 /**
  * \brief Implements place / allocate functions and new / delete operator overloads for resource building.
  * \param name Name of resource class.
  */
#define IMPLEMENT_PLACE_INLINE(name) \
		void* operator new(std::size_t size) \
		{ \
			if (rage::pgCompiler::IsCurrentThreadCompiling()) \
				return rage::pgCompiler::GetVirtualAllocator()->Allocate(size); \
			if(const rage::datResource* rsc = rage::TlsManager::GetResource()) \
				return rsc->Map->MainPage; \
			return ::operator new(size); \
		} \
		void* operator new(std::size_t size, void* where) \
		{ \
			return ::operator new(size, where); \
		} \
		void operator delete(void* mem) { if(!rage::TlsManager::GetResource()) ::delete(mem); } \
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
