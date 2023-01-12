#pragma once
#include <mutex>

#include "fwTypes.h"
#include "fiDevice.h"
#include "gmFunc.h"
#include "unionCast.h"

namespace rage
{
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
		static constexpr int sm_MaxStreams = 64;

		inline static std::mutex sm_mutex;

		inline static fiStream* sm_streams = gm::GetGlobal<fiStream*>("sm_Streams"); // [sm_MaxStreams];
		inline static char* sm_streamBuffers = gm::GetGlobal<char*>("sm_Buffers"); // [sm_MaxStreams][STREAM_BUFFER_SIZE]{ {0} };
		inline static int sm_ActiveStreams;

		fiDevice* m_pDevice;
		FI_HANDLE m_FileHandle; // Device specific handle. For i.e. in fiDeviceLocal (win api) it's file handle.
		char* m_Buffer; // Temporary buffer where stream writes and reads data from
		int64 m_DeviceCursorPos;
		int m_BufferCursorPos; // Read / write operations are processed relative to position of cursor in buffer.
		int m_BufferContentEnd; // If steam reads any data in buffer it will point to position where this data ends.
		int m_BufferSize;
	public:
		fiStream();
		fiStream(fiDevice* pDevice, FI_HANDLE handle, char* buffer);

		static int GetNumActiveStreams();
		static bool HasAvailableStreams();

		/**
		 * \brief Creates new resource, overwriting existing and allocates stream.
		 * \param resourceName Name of the resource that will be resolved by fiDevice.
		 * \param pDevice Device to use for working with file.
		 * \return Allocated stream for created resource or nullptr if unable to create resource.
		 * \remark Not supported on following devices: fiPackfile
		 */
		static fiStream* CreateWithDevice(const char* resourceName, fiDevice* pDevice);

		/**
		 * \brief Creates new resource, overwriting existing and allocates stream.
		 * \param resourceName Name of the resource that will be resolved by fiDevice.
		 * \return Allocated stream for created resource or nullptr if unable to create resource.
		 * \remark Not supported on following devices: fiPackfile
		 */
		static fiStream* Create(const char* resourceName);

		/**
		 * \brief Opens existing resource using given device and allocates stream.
		 * \param resourceName Name of the resource that will be resolved by fiDevice.
		 * \param pDevice Device to use for working with file.
		 * \param isReadOnly Whether file needs to be opened only with read access or read & write.
		 * \return Allocated stream for the resource or nullptr if resource was not found.
		 * \remark Write mode not supported on following devices: fiPackfile
		 */
		static fiStream* OpenWithDevice(const char* resourceName, fiDevice* pDevice, bool isReadOnly = true);

		/**
		 * \brief Opens existing resource and allocates stream.
		 * \param resourceName Name of the resource that will be resolved by fiDevice.
		 * \param isReadOnly Whether file needs to be opened only with read access or read & write.
		 * \return Allocated stream for the resource or nullptr if resource was not found.
		 * \remark Write mode not supported on following devices: fiPackfile
		 */
		static fiStream* Open(const char* resourceName, bool isReadOnly = true);

		/**
		 * \brief Creates a new stream for given resource.
		 * After stream is no longer needed, call Close().
		 * \param resourceName Name of the resource. i.e. 'x64/audio/audio_rel.rpf'.
		 * \param handle Handle returned by fiDevice.
		 * \param pDevice Pointer to fiDevice, which data belongs to.
		 * \return Pointer to fiStream or nullptr if was unable to allocate new stream.
		 */
		static fiStream* AllocStream(const char* resourceName, FI_HANDLE handle, fiDevice* pDevice);

		/**
		 * \brief Frees up allocated slot. Has to be called after stream is no longer needed.
		 */
		void Close();

		/**
		 * \brief Immediately sends all data in buffer to fiDevice.
		 * \return Value indicating whether any data was written or not.
		 */
		bool Flush();

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
		int Read(const char* dest, u32 size);

		/**
		 * \brief Writes given data to stream.
		 * Depending on size of the data, it can be temporally
		 * stored in buffer or immediately sent to device if size is too large.
		 * \param data Pointer to data.
		 * \param size Number of bytes to write from data.
		 * \return Number of bytes written. -1 if data was unable to write.
		 */
		int Write(const char* data, u32 size);

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
	static_assert(sizeof(fiStream) == 0x30); // + 4 byte alignment

	// TODO: Crash on 2802
	namespace hooks
	{
#ifdef RAGE_HOOK_SWAP_FISTREAM
		static inline gm::gmFuncSwap gSwap_FiStream_CreateWithDeivce(
			"fiStream::CreateWithDevice",
			"48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 20 4C 8B F2 48 8B F1",
			fiStream::CreateWithDevice);

		static inline gm::gmFuncSwap gSwap_FiStream_Create(
			[]() -> uintptr_t
			{
				return gm::Scan("fiStream::Create", "E8 ? ? ? ? 48 8B F8 48 8B 73 08").GetCall();
			},
			fiStream::Create);

		static inline gm::gmFuncSwap gSwap_FiStream_OpenWithDevice(
			"fiStream::OpenWithDevice",
			"48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 56 48 83 EC 20 48 8B 02 48 8B FA",
			fiStream::OpenWithDevice);

		static inline gm::gmFuncSwap gSwap_FiStream_Open(
			"fiStream::Open",
			"48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 40 8A EA 48 8B F1 E8",
			fiStream::Open);

		static inline gm::gmFuncSwap gSwap_FiStream_AllocStream(
			"fiStream::AllocStream",
			"48 89 5C 24 08 57 48 83 EC 20 48 8D 0D ? ? ? ? ? ? ? ? 49",
			fiStream::AllocStream);

		static inline gm::gmFuncSwap gSwap_FiStream_Close(
			"fiStream::Close",
			"40 53 48 83 EC 20 83 79 24 00 48 8B D9 75",
			gm::CastAny(&fiStream::Close));
#endif
	}
}
