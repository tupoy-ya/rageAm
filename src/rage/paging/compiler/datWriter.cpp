#include "datWriter.h"

#include "TlsManager.h"
#include "datCompiler.h"
#include "system/assert.h"
#include "helpers/format.h"

rage::ResourceHeader rage::datWriteData::GetHeader() const
{
	datResourceInfo info{ Virtual.Data, Physical.Data };
	return { MAGIC_RSC, Version, info };
}

rage::datWriter::TBuffer rage::datWriter::AllocatePage(const datPackedPage& packedPage, const datAllocator* pAllocator)
{
	// NOTE: We're allocating memory for whole page, but actually used memory in most cases covers only small part of it.
	// There's some trade offs for this though:
	// - File size is still small because it's compressed
	// - It's better to allocate chunks of same size to reduce fragmentation
	TBuffer buffer = std::make_shared<char[]>(packedPage.Size);

	m_AllocSize += packedPage.Size;

	// Fit chunks into page buffer
	u32 chunkOffset = 0;
	for (u16 index : packedPage.Indices)
	{
		u32 chunkSize = pAllocator->GetChunkSize(index);

		memcpy(buffer.get() + chunkOffset, pAllocator->GetChunkData(index), chunkSize);

		chunkOffset += chunkSize;
	}
	m_RawSize += chunkOffset;

	return buffer;
}

void rage::datWriter::WriteHeader() const
{
	AM_TRACEF("datWriter::WriteHeader() -> Writing header (Version: {}, Virtual Data: {:X}, Physical Data: {:X})",
		m_WriteData->Version, m_WriteData->Virtual.Data, m_WriteData->Physical.Data);

	ResourceHeader header = m_WriteData->GetHeader();
	m_Fs->write(reinterpret_cast<char*>(&header), sizeof(ResourceHeader));
}

void rage::datWriter::WriteData(const datPackedPage& packedPage, const datAllocator* pAllocator)
{
	TBuffer buffer = AllocatePage(packedPage, pAllocator);

	CompressAndWrite(packedPage, buffer);
}

void rage::datWriter::CompressAndWrite(const datPackedPage& packedPage, const TBuffer& buffer)
{
	m_Compressor.Init(buffer.get(), packedPage.Size);

	u32 compressedSize;
	int status;
	do
	{
		status = m_Compressor.Deflate(compressedSize);
		m_Fs->write(m_Compressor.GetBuffer(), compressedSize);

		m_FileSize += compressedSize;
	} while (status != Z_STREAM_END);
}

bool rage::datWriter::OpenResource()
{
	m_Fs = std::make_unique<std::ofstream>(m_Path, std::ios::binary | std::ios::trunc);
	if (AM_ASSERT(m_Fs->is_open(), "Unable to open resource file for writing."))
		return false;

	return true;
}

void rage::datWriter::CloseResource()
{
	if (m_Fs)
		return;

	m_Fs->close();
	m_Fs.release();
}

void rage::datWriter::ResetWriteStats()
{
	// Header present only in file.

	m_RawSize = 0;
	m_FileSize = sizeof(ResourceHeader);
	m_AllocSize = 0;
}

void rage::datWriter::PrintWriteStats() const
{
	AM_TRACEF("datWriter -> Exported resource {}", m_Path);
	AM_TRACEF("Raw Size: {}", FormatBytes(m_RawSize));
	AM_TRACEF("File Size: {}", FormatBytes(m_FileSize));
	AM_TRACEF("Alloc Size: {}", FormatBytes(m_AllocSize));
}

bool rage::datWriter::Write(const char* path, const datWriteData& writeData)
{
	ResetWriteStats();

	m_Path = path;
	m_WriteData = &writeData;

	if (!OpenResource())
		return false;

	datCompiler* pCompiler = TlsManager::GetCompiler();

	WriteHeader();
	WriteData(writeData.Virtual, pCompiler->GetVirtualAllocator());
	WriteData(writeData.Physical, pCompiler->GetPhysicalAllocator());

	PrintWriteStats();
	CloseResource();

	m_WriteData = nullptr;

	return true;
}
