#pragma once

#include "paging/datHeader.h"
#include "datAllocator.h"

namespace rage
{
	/**
	 * \brief Provides functionality for copying resource to file streams (not file itself)
	 * \n Creates and sets virtual & physical allocators in current thread.
	 * \n Compilation process is done via copy constructor the same way as resource building.
	 * \n Resource class has to define copy constructor as mentioned earlier and fixup all pointers in it.
	 */
	class datCompiler
	{
		// 1 MB
		static constexpr u32 DEFAULT_HEAP_SIZE_VIRTUAL = 1ul * 1024ul * 1024ul;
		// 32 MB
		static constexpr u32 DEFAULT_HEAP_SIZE_PHYSICAL = 32ul * 1024ul * 1024ul;

		datAllocator m_VirtualAllocator;
		datAllocator m_PhysicalAllocator;
	public:
		datCompiler();
		~datCompiler();

		/**
		 * \brief Performs copying of root resource to virtual stream.
		 */
		template<typename TPaged>
		TPaged* Compile(const TPaged& pPaged)
		{
			// 'Compiling' resource is done through copying it
			// to allocated page (via datAllocator).
			return new TPaged(pPaged);
		}

		datAllocator* GetVirtualAllocator() { return &m_VirtualAllocator; }
		datAllocator* GetPhysicalAllocator() { return &m_PhysicalAllocator; }
	};
}
