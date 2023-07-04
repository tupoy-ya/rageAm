#include "writer.h"

#include "compiler.h"
#include "am/system/asserts.h"
#include "common/logger.h"
#include "helpers/format.h"
#include "rage/system/tls.h"
#include "rage/zlib/stream.h"

#include "snapshotallocator.h"
#include "am/file/fileutils.h"

rage::datResourceHeader rage::datCompileData::GetHeader() const
{
	u32 virtualData = datResourceInfo::EncodeChunks(VirtualChunks);
	u32 phyiscalData = datResourceInfo::EncodeChunks(PhysicalChunks);

	datResourceInfo info(virtualData, phyiscalData);
	return datResourceHeader(MAGIC_RSC, Version, info);
}

u32 rage::pgRscWriter::ComputeUsedSize(const datPackedChunks& packedPage, const pgSnapshotAllocator* pAllocator) const
{
	// Accumulate allocate size, simply add up chunk sizes
	u32 bufferSize = 0;
	for (u8 i = 0; i < PG_MAX_BUCKETS; i++)
	{
		for (const auto& chunk : packedPage.Buckets[i])
		{
			for (u16 index : chunk)
			{
				bufferSize += pAllocator->GetBlockSize(index);
			}
		}
	}
	return bufferSize;
}

void rage::pgRscWriter::WriteHeader() const
{
	datResourceHeader header = m_WriteData->GetHeader();

	AM_DEBUGF("pgRscWriter::WriteHeader() -> Version: %u, Virtual Data: %#x, Physical Data: %#x",
		m_WriteData->Version, header.Info.VirtualData, header.Info.PhysicalData);

	DWORD dwBytesWritten;
	WriteFile(m_File, &header, sizeof datResourceHeader, &dwBytesWritten, NULL);
}

void rage::pgRscWriter::WriteData(const datPackedChunks& packedPage, const pgSnapshotAllocator* pAllocator)
{
	if (packedPage.IsEmpty)
		return;

	u32 chunkSize = PG_MIN_CHUNK_SIZE << packedPage.SizeShift;

	// Allocate buffer that is large enough to compress all chunks in single pass (it is much faster)
	u32 bufferSize = ComputeUsedSize(packedPage, pAllocator);
	char* buffer = new char[bufferSize];
	memset(buffer, 0, bufferSize);

	AM_DEBUGF("pgRscWriter::WriteData() -> Using %u as buffer size", bufferSize);

	u32 bufferOffset = 0;
	for (u8 i = 0; i < PG_MAX_BUCKETS; i++)
	{
		// Each bucket has different amount of chunks (from 1 to 127),
		// each chunk has infinitely many blocks (with sum size not exceeding chunk size)
		const auto& bucketChunks = packedPage.Buckets[i];
		for (const auto& chunk : bucketChunks)
		{
			if (!chunk.Any())
				continue;

			// Copy every block data to chunk buffer
			for (u16 index : chunk)
			{
				pVoid block = pAllocator->GetBlock(index);
				u32 blockSize = pAllocator->GetBlockSize(index);

				m_RawSize += blockSize;

				if (bufferOffset + blockSize > bufferSize)
				{
					AM_DEBUGF("pgRscWriter::WriteData() -> Compressing %u bytes", bufferOffset);
					CompressAndWrite(buffer, bufferOffset);
					bufferOffset = 0;

					// Erase buffer from previous chunks, trash will make compression worse
					memset(buffer, 0, bufferSize);
				}

				memcpy(buffer + bufferOffset, block, blockSize);
				bufferOffset += blockSize;
			}
			m_AllocSize += chunkSize;
		}
		chunkSize /= 2;
	}

	// Finalize remaining bytes, if there's anything (in best scenario we must compress once here)
	if (bufferOffset > 0)
		CompressAndWrite(buffer, bufferOffset);

	delete[] buffer;
}

void rage::pgRscWriter::CompressAndWrite(pVoid buffer, u32 bufferSize)
{
	// Assuming compression ratio 2:1
	u32 compressBufferSize = bufferSize / 2;
	zLibCompressor compressor(compressBufferSize);

	DWORD dwBytesWritten;

	bool done = false;
	while (!done)
	{
		pVoid compressedBuffer;
		u32	compressedSize;

		done = compressor.Compress(buffer, bufferSize, compressedBuffer, compressedSize);

		WriteFile(m_File, compressedBuffer, compressedSize, &dwBytesWritten, NULL);
		m_FileSize += compressedSize;

		AM_DEBUGF("pgRscWriter::CompressAndWrite() -> Compressed %u to %u", bufferSize, compressedSize);
	}
}

bool rage::pgRscWriter::OpenResource()
{
	m_File = rageam::file::CreateNew(m_Path);

	return AM_VERIFY(m_File != INVALID_HANDLE_VALUE,
		L"pgRscWriter::Open(%ls) -> Unable to open resource file for writing.", m_Path);
}

void rage::pgRscWriter::CloseResource() const
{
	CloseHandle(m_File);
}

void rage::pgRscWriter::ResetWriteStats()
{
	m_RawSize = 0;
	m_FileSize = sizeof(datResourceHeader); // Header present only in file
	m_AllocSize = 0;
}

void rage::pgRscWriter::PrintWriteStats() const
{
	AM_TRACEF(L"pgRscWriter -> Exported resource %ls", m_Path);
	AM_TRACEF("Raw Size: %s", FormatBytes(m_RawSize).GetCStr());
	AM_TRACEF("File Size: %s", FormatBytes(m_FileSize).GetCStr());
	AM_TRACEF("Alloc Size: %s", FormatBytes(m_AllocSize).GetCStr());
}

bool rage::pgRscWriter::Write(const wchar_t* path, const datCompileData& writeData)
{
	ResetWriteStats();

	m_Path = path;
	m_WriteData = &writeData;

	if (!OpenResource())
		return false;

	WriteHeader();
	WriteData(writeData.VirtualChunks, pgRscCompiler::GetVirtualAllocator());
	WriteData(writeData.PhysicalChunks, pgRscCompiler::GetPhysicalAllocator());

	PrintWriteStats();
	CloseResource();

	m_WriteData = nullptr;
	m_Path = nullptr;

	return true;
}
