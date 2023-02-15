#pragma once

#include "zLibStream.h"

namespace rage
{
	class zLibCompressor : public zLibStream
	{
		// 4 MB
		static constexpr u32 ZLIB_DEFAULT_COMPRESSOR_BUFFER_SIZE = 4ull * 1024ull * 1024ull;

		bool m_OwnBuffer = false;

		int InitInternal() override;
		void ResetInternal() override;
	public:
		using zLibStream::zLibStream;

		zLibCompressor();
		~zLibCompressor() override;

		int Deflate(u32& size);
	};
}
