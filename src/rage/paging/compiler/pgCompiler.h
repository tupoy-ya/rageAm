#pragma once

#include "datAllocator.h"
#include "datCompiler.h"
#include "datPacker.h"
#include "datWriter.h"

#include "TlsManager.h"
#include "Logger.h"

namespace rage
{
	class pgCompiler
	{
		static inline thread_local bool tl_IsCompiling = false;

		static void SetIsCurrentThreadCompiling(bool toggle)
		{
			tl_IsCompiling = toggle;
		}
	public:
		/**
		 * \brief Performs resource compilation (copying) to file.
		 * \tparam T Type of resource root structure.
		 * \param pPaged Pointer to root resource structure.
		 * \param version Version of resource from paging/builder/pgBuilds.h;
		 * \param path Path to output file.
		 */
		template<typename T>
		static void Compile(const T* pPaged, u32 version, const char* path)
		{
			AM_TRACEF("Compiling resource {}", path);

			const T& paged = *pPaged;

			// Step 1: Copy all structures to virtual / physical allocators.
			SetIsCurrentThreadCompiling(true);
			datCompiler compiler;
			compiler.Compile(paged);
			SetIsCurrentThreadCompiling(false);

			datWriteData writeData{ version };

			// Step 2: Pack all allocations (allocator chunks) to pages.
			datPacker virtualPacker(GetVirtualAllocator());
			datPacker physicalPacker(GetPhysicalAllocator());

			virtualPacker.Pack(writeData.Virtual);
			physicalPacker.Pack(writeData.Physical);

			// Step 3: Compress and write data to file.
			datWriter writer;
			writer.Write(path, writeData);
		}

		/**
		 * \brief Gets virtual allocator (RAM) in current thread.
		 * \return Allocator instance if resource is being compiled; Otherwise nullptr.
		 */
		static datAllocator* GetVirtualAllocator()
		{
			datCompiler* rsc = TlsManager::GetCompiler();
			if (!rsc)
				return nullptr;
			return rsc->GetVirtualAllocator();
		}

		/**
		 * \brief Gets physical allocator (GPU) in current thread.
		 * \return Allocator instance if resource is being compiled; Otherwise nullptr.
		 */
		static datAllocator* GetPhysicalAllocator()
		{
			datCompiler* rsc = TlsManager::GetCompiler();
			if (!rsc)
				return nullptr;
			return rsc->GetPhysicalAllocator();
		}

		static bool IsCurrentThreadCompiling() { return tl_IsCompiling; }
	};
}
