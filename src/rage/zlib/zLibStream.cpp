#include "zLibStream.h"

#include "system/assert.h"

rage::zLibStream::zLibStream() : m_Stream()
{
	m_Initialized = false;
	m_Buffer = nullptr;
	m_BufferSize = 0;
}

rage::zLibStream::zLibStream(void* buffer, u32 bufferSize) : m_Stream{}
{
	SetBuffer(buffer, bufferSize);
	m_Initialized = false;
}

void rage::zLibStream::Reset()
{
	if (!m_Initialized)
		return;
	ResetInternal();
	m_Initialized = false;
}

void rage::zLibStream::Init(void* data, u32 size)
{
	AM_ASSERT_FATAL(m_Buffer, "Output buffer is not set.");

	Reset();

	int result = InitInternal();
	AM_ASSERT_FATAL(result == Z_OK, "Zlib Init failed.");

	m_Stream.next_in = static_cast<Bytef*>(data);
	m_Stream.avail_in = size;

	m_Initialized = true;
}

void rage::zLibStream::SetBuffer(void* buffer, u32 bufferSize)
{
	m_Buffer = static_cast<Bytef*>(buffer);
	m_BufferSize = bufferSize;
}
