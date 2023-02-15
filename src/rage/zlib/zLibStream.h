#pragma once

#include "fwTypes.h"
#include "zlib.h"

namespace rage
{
	class zLibStream
	{
	protected:
		// Raw deflate without header.
		static constexpr int ZLIB_WINDOW_BITS = -15;
		static constexpr int ZLIB_COMPRESSION_LEVEL = 9;
		static constexpr int ZLIB_MEMORY_LEVEL = 8;

		z_stream m_Stream;
		Bytef* m_Buffer;
		u32 m_BufferSize;
		bool m_Initialized;

		virtual int InitInternal() = 0;
		virtual void ResetInternal() = 0;
	public:
		zLibStream();
		zLibStream(void* buffer, u32 bufferSize);

		virtual ~zLibStream() = default;

		void Reset();
		void Init(void* data, u32 size);
		void SetBuffer(void* buffer, u32 bufferSize);
		char* GetBuffer() const { return reinterpret_cast<char*>(m_Buffer); }
		u32 GetBufferSize() const { return m_BufferSize; }
	};
}
