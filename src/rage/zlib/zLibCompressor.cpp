#include "zLibCompressor.h"
#include "system/assert.h"

int rage::zLibCompressor::InitInternal()
{
	return deflateInit2(&m_Stream,
		ZLIB_COMPRESSION_LEVEL,
		Z_DEFLATED,
		ZLIB_WINDOW_BITS,
		ZLIB_MEMORY_LEVEL,
		Z_DEFAULT_STRATEGY);
}

void rage::zLibCompressor::ResetInternal()
{
	deflateEnd(&m_Stream);
}

rage::zLibCompressor::zLibCompressor()
{
	Bytef* buffer = new Bytef[ZLIB_DEFAULT_COMPRESSOR_BUFFER_SIZE];
	SetBuffer(buffer, ZLIB_DEFAULT_COMPRESSOR_BUFFER_SIZE);
	m_OwnBuffer = true;
}

rage::zLibCompressor::~zLibCompressor()
{
	zLibCompressor::ResetInternal();
	if (m_OwnBuffer)
		delete[] m_Buffer;
}

int rage::zLibCompressor::Deflate(u32& size)
{
	m_Stream.next_out = m_Buffer;
	m_Stream.avail_out = m_BufferSize;

	int result = deflate(&m_Stream, Z_FINISH);
	AM_ASSERT_FATAL(result > 0, "zLib Deflate failed.");

	size = m_BufferSize - m_Stream.avail_out;
	return result;
}

