#include "fiStream.h"
#include "Windows.h"

#ifdef RAGE_STANDALONE
rage::fiStream rage::fiStream::sm_Streams[sm_MaxStreams];
#endif

rage::fiStream::fiStream()
{
	m_pDevice = nullptr;
	m_Buffer = nullptr;
	m_FileHandle = FI_INVALID_HANDLE;

	m_DeviceCursorPos = 0;
	m_BufferCursorPos = 0;
	m_BufferContentEnd = 0;

	m_BufferSize = 0;
}

rage::fiStream::fiStream(fiDevice* pDevice, fiHandle_t handle, char* buffer) : fiStream()
{
	m_pDevice = pDevice;
	m_Buffer = buffer;
	m_FileHandle = handle;

	// Not sure what is the point of every stream having
	// size of buffer when it's constant. (is it?)
	// Though game has checks if it's set to zero.
	m_BufferSize = STREAM_BUFFER_SIZE;
}

int rage::fiStream::GetNumActiveStreams()
{
	sm_Mutex.lock();
	int result = sm_ActiveStreams;
	sm_Mutex.unlock();
	return result;
}

bool rage::fiStream::HasAvailableStreams()
{
	sm_Mutex.lock();
	bool result = sm_ActiveStreams < sm_MaxStreams;
	sm_Mutex.unlock();
	return result;
}

rage::fiStream* rage::fiStream::CreateWithDevice(const char* resourceName, fiDevice* pDevice)
{
	if (!pDevice)
		return nullptr;

	if (!HasAvailableStreams())
		return nullptr;

	fiHandle_t handle = pDevice->vftable->Create(pDevice, resourceName);

	if (handle == FI_INVALID_HANDLE)
		return nullptr;

	return AllocStream(resourceName, handle, pDevice);
}

rage::fiStream* rage::fiStream::Create(const char* resourceName)
{
	fiDevice* device = fiDevice::GetDeviceImpl(resourceName, false);
	return CreateWithDevice(resourceName, device);
}

rage::fiStream* rage::fiStream::OpenWithDevice(const char* resourceName, fiDevice* pDevice, bool isReadOnly)
{
	// g_Log.LogT("fiStream::Open({}, {})", resourceName, isReadOnly);

	if (!pDevice)
		return nullptr;

	if (!HasAvailableStreams())
		return nullptr;

	fiHandle_t handle = pDevice->vftable->Open(pDevice, resourceName, isReadOnly);

	if (handle == FI_INVALID_HANDLE)
		return nullptr;

	return AllocStream(resourceName, handle, pDevice);
}

rage::fiStream* rage::fiStream::Open(const char* resourceName, bool isReadOnly)
{
	fiDevice* pDevice = fiDevice::GetDeviceImpl(resourceName, isReadOnly);
	return OpenWithDevice(resourceName, pDevice, isReadOnly);
}

rage::fiStream* rage::fiStream::AllocStream(const char* resourceName, fiHandle_t handle, fiDevice* pDevice)
{
	// Resource name is actually passed in fiStream::AllocStream but not used,
	// most likely was simply logged somewhere.

	//g_Log.Log("Allocating stream for: {}. Handle: {:X}. Device: {:X}",
	//	resourceName, handle, reinterpret_cast<intptr_t>(pDevice));

	if (!HasAvailableStreams())
		return nullptr;

	sm_Mutex.lock();

	// Find slot with closed stream
	int i = 0;
	for (; i < sm_MaxStreams; i++)
	{
		fiStream& slot = sm_Streams[i];

		if (slot.m_pDevice == nullptr)
			break;
	}

#ifdef RAGE_STANDALONE
	char* buffer = sm_StreamBuffers[i];
#else
	// We'd use sm_StreamBuffers[i] but compiler doesn't know about array dimensions
	char* buffer = sm_StreamBuffers + STREAM_BUFFER_SIZE * (uintptr_t)i;
#endif
	sm_Streams[i] = fiStream(pDevice, handle, buffer);
	sm_ActiveStreams++;

	sm_Mutex.unlock();

	return &sm_Streams[i];
}

void rage::fiStream::Close()
{
	if (m_BufferContentEnd == 0 && m_BufferCursorPos != 0)
		Flush();

	sm_Mutex.lock();

	m_pDevice->vftable->Close(m_pDevice, m_FileHandle);
	m_pDevice = nullptr;
	m_FileHandle = FI_INVALID_HANDLE;
	sm_ActiveStreams--;

	sm_Mutex.unlock();
}

bool rage::fiStream::Flush()
{
	if (m_BufferContentEnd)
	{
		m_pDevice->vftable->Seek64(
			m_pDevice, m_FileHandle, m_DeviceCursorPos + m_BufferCursorPos, SEEK_FILE_BEGIN);
	}
	else if (m_BufferCursorPos)
	{
		m_pDevice->vftable->WriteBuffer(m_pDevice, m_FileHandle, m_Buffer, m_BufferCursorPos);
	}

	m_BufferContentEnd = 0;
	m_DeviceCursorPos += m_BufferCursorPos;
	m_BufferCursorPos = 0;
	return m_pDevice->vftable->Flush(m_pDevice, m_FileHandle);
}

u32 rage::fiStream::Size()
{
	// TODO:
	// m_pDevice->vftable + 0x70 (p_FileData)

	throw;
}

int rage::fiStream::Read(u8* dest, u32 size)
{
	throw;
}

int rage::fiStream::Write(u8* data, u32 size)
{
	__int64 result; // rax
	signed int bufferSize; // eax MAPDST
	int v10; // eax
	signed int availableLength; // eax
	int32_t cursorPos; // ecx
	int v14; // eax

	if (this->m_BufferContentEnd)
	{
		if (!Flush())
			return -1;
	}
	bufferSize = this->m_BufferSize;
	if (size >= bufferSize)                     // data doesn't fit buffer at all, write directly to device
	{
		if (Flush())
		{
			result = (this->m_pDevice->vftable->Write)(this->m_pDevice, this->m_FileHandle, (LPVOID)data, size);
			if (result >= 0)
			{
				this->m_DeviceCursorPos += result;
				return result;
			}
		}
		return -1;
	}
	availableLength = bufferSize - this->m_BufferCursorPos;
	if (size >= availableLength)                // buffer has enough space, but left space is not enough. add to buffer what we can and then send to device
	{
		memmove(&this->m_Buffer[this->m_BufferCursorPos], data, availableLength);
		cursorPos = this->m_BufferCursorPos;
		bufferSize = this->m_BufferSize;
		this->m_BufferCursorPos = bufferSize;
		size += cursorPos - bufferSize;             // since part of data was already written, we need to set pointer and size for the remaining of it
		data += bufferSize - cursorPos;
		if (!Flush())
			return -1;
	}
	memmove(&this->m_Buffer[this->m_BufferCursorPos], data, size);// write all data / whats left after partial flushing
	this->m_BufferCursorPos += size;
	return size;
}

bool rage::fiStream::WriteChar(const char c)
{
	return Write((u8*)&c, 1) != -1;
}
