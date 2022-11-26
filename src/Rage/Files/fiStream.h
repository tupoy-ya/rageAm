#pragma once
#include <mutex>

#include "../fwTypes.h"
#include "../../Logger.h"

namespace rage
{
	typedef int64_t fiDevice;

	/**
	 * \brief Stream is the low-level bridge to fiDevice (which provides information about remote file on disk / internet or in .RPF).
	 * \n It allows to abstract from the actual file implementation and work with any type of data the same way,
	 * which includes reading and writing bytes.
	 * \n Internally uses buffer to prevent huge amounts of calls to file system / internet which can be slow.
	 * \n To get stream on specific resource file, use rage::fiAssetManager::Open function and it's overloads.
	 */
	class fiStream
	{
		static constexpr int STREAM_BUFFER_SIZE = 4096;

		// Is it parsed from GameConfig.xml? It's not constant.
		// Additionally there's checks if it's not set to zero.
		static constexpr int sm_maxStreams = 64;

		static std::mutex sm_mutex;
		static fiStream sm_streams[sm_maxStreams];

		static int sm_activeStreams;

		inline static char sm_streamBuffers[sm_maxStreams][STREAM_BUFFER_SIZE]{ {0} };

		fiDevice* p_Device;
		intptr_t p_FileData;
		char* m_buffer;

		int64 m_deviceCursorPos;
		// Read / write operations are processed relative to position of cursor in buffer.
		int m_bufferCursorPos;
		// If steam reads any data in buffer it will point to position where this data ends.
		int m_bufferContentEnd;

		int m_bufferSize;

		int m_dword2C;

		fiStream(fiDevice* pDevice, intptr_t pData, char* buffer);

	public:
		static int GetNumActiveStreams();
		static bool HasAvailableStreams();

		/**
		 * \brief Creates new resource, overwriting existing and allocates stream.
		 * \param resourceName Name of the resource that will be resolved by fiDevice.
		 * \return Allocated stream for created resource or nullptr if unable to create resource.
		 */
		static fiStream* Create(const char* resourceName);

		/**
		 * \brief Opens existing resource and allocates stream.
		 * \param resourceName Name of the resource that will be resolved by fiDevice.
		 * \return Allocated stream for the resource or nullptr if resource was not found.
		 */
		static fiStream* Open(const char* resourceName);

		/**
		 * \brief Creates a new stream for given resource.
		 * After stream is no longer needed, call Close().
		 * \param resourceName Name of the resource. i.e. 'x64/audio/audio_rel.rpf'.
		 * \param pData Pointer to data that can be interpreted by fiDevice.
		 * \param device Pointer to fiDevice, which data belongs to.
		 * \return Pointer to fiStream or nullptr if was unable to allocate new stream.
		 */
		static fiStream* AllocStream(const char* resourceName, intptr_t pData, fiDevice* device);

		/**
		 * \brief Frees up allocated slot. Has to be called after stream is no longer needed.
		 */
		void Close();

		/**
		 * \brief Immediately sends all data in buffer to fiDevice.
		 */
		void Flush();

		/**
		 * \brief Gets size of resource.
		 * \return Size of resource in bytes.
		 */
		u32 Size();

		/**
		 * \brief Reads requested amount of bytes from stream.
		 * \param dest Pointer where to write data from stream.
		 * \param size Number of bytes to read.
		 * \return Number of bytes was read. -1 if unsuccessful.
		 */
		int Read(const void* dest, u32 size);

		/**
		 * \brief Writes given data to stream.
		 * Depending on size of the data, it can be temporally
		 * stored in buffer or immediately sent to device if size is too large.
		 * \param data Pointer to data.
		 * \param size Number of bytes to write from data.
		 * \return Number of bytes written. -1 if data was unable to write.
		 */
		int Write(const void* data, u32 size);

		/**
		 * \brief Writes single character to stream.
		 * \param c Character to write.
		 * \return Whether char was written successfully or not.
		 */
		bool WriteChar(char c);

		/**
		 * \brief Writes formatted line to stream using modern C++ standard.
		 * \param fmt Format string.
		 * \param args Formatting arguments.
		 * \return Number of characters written. -1 if unsuccessful.
		 */
		template<typename... Args>
		u32 WriteLine(std::_Fmt_string<Args...> fmt, Args&&... args)
		{
			// This is different from game implementation (game uses printf)
			// But for consistency purposes we will modern formatting, at least for now.
			std::string str = std::format(fmt, std::forward<Args>(args)...);

			return Write(str.c_str(), str.length());
		}
	};
	static_assert(sizeof(fiStream) == 0x30);
}
