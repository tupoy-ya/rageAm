#pragma once

#include "datAllocator.h"
#include "datPacker.h"
#include "fwTypes.h"
#include "paging/datHeader.h"
#include "zlib/zLibCompressor.h"

namespace rage
{
	/**
	 * \brief Contains resource version, grouped memory chunks and encoded page data.
	 */
	struct datWriteData
	{
		u32 Version;

		datPackedPage Virtual;
		datPackedPage Physical;

		ResourceHeader GetHeader() const;
	};

	/**
	 * \brief Writes and compresses resource from compiler streams.
	 */
	class datWriter
	{
		typedef std::shared_ptr<char[]> TBuffer;

		zLibCompressor m_Compressor;

		// Size of purely resource structures and data.
		u32 m_RawSize;
		// Size of resource data fitted on large memory pages, hence size of allocated resource.
		u32 m_AllocSize;
		// Size of compressed resource file on disk.
		u32 m_FileSize;

		std::unique_ptr<std::ofstream> m_Fs;
		const datWriteData* m_WriteData;
		const char* m_Path;

		// Allocates buffer for page and copies packed chunks to it from allocator.
		TBuffer AllocatePage(const datPackedPage& packedPage, const datAllocator* pAllocator);

		void WriteHeader() const;
		void WriteData(const datPackedPage& packedPage, const datAllocator* pAllocator);
		void CompressAndWrite(const datPackedPage& packedPage, const TBuffer& buffer);

		bool OpenResource();
		void CloseResource();

		void ResetWriteStats();
		void PrintWriteStats() const;
	public:
		datWriter() = default;

		bool Write(const char* path, const datWriteData& writeData);
	};
}
