#include "fiStream.h"

rage::fiStream::fiStream()
{
	p_Device = nullptr;
	m_buffer = nullptr;
	m_handle = RAGE_INVALID_HANDLE;

	m_deviceCursorPos = 0;
	m_bufferCursorPos = 0;
	m_bufferContentEnd = 0;
	m_dword2C = 0;

	m_bufferSize = 0;
}

rage::fiStream::fiStream(fiDevice* pDevice, RAGE_HANDLE handle, char* buffer) : fiStream()
{
	p_Device = pDevice;
	m_buffer = buffer;
	m_handle = handle;

	// Not sure what is the point of every stream having
	// size of buffer when it's constant. (is it?)
	// Though game has checks if it's set to zero.
	m_bufferSize = STREAM_BUFFER_SIZE;
}

int rage::fiStream::GetNumActiveStreams()
{
	return sm_activeStreams;
}

bool rage::fiStream::HasAvailableStreams()
{
	return sm_activeStreams < sm_maxStreams;
}

rage::fiStream* rage::fiStream::Create(const char* resourceName)
{
	if (!HasAvailableStreams())
		return nullptr;

	// TODO: fiDevice::GetDeviceImpl
	fiDevice* device = nullptr;

	if (!device)
		return nullptr;

	RAGE_HANDLE handle = device->vftable->Create(device, resourceName);

	if (handle == RAGE_INVALID_HANDLE)
		return nullptr;

	return AllocStream(resourceName, handle, device);
}

rage::fiStream* rage::fiStream::Open(const char* resourceName, bool isReadOnly)
{
	if (!HasAvailableStreams())
		return nullptr;

	// TODO: fiDevice::GetDeviceImpl
	fiDevice* device = nullptr;

	if (!device)
		return nullptr;

	RAGE_HANDLE handle = device->vftable->Open(device, resourceName, isReadOnly);

	if (handle == RAGE_INVALID_HANDLE)
		return nullptr;

	return AllocStream(resourceName, handle, device);
}

rage::fiStream* rage::fiStream::AllocStream(const char* resourceName, intptr_t pData, fiDevice* device)
{
	// Resource name is actually passed in fiStream::AllocStream but not used,
	// most likely was simply logged somewhere.
	g_logger->Log("Allocating stream for: {}. Data Ptr: {:X}. Device: {:X}",
		resourceName, pData, reinterpret_cast<intptr_t>(device));

	if (!HasAvailableStreams())
		return nullptr;

	sm_mutex.lock();

	// Find slot with closed stream
	int i = 0;
	for (i = 0; i < sm_maxStreams; i++)
	{
		fiStream& slot = sm_streams[i];

		if (slot.p_Device == nullptr)
			break;
	}

	sm_streams[i] = fiStream(device, pData, sm_streamBuffers[i]);
	sm_activeStreams++;

	sm_mutex.unlock();

	return &sm_streams[i];
}

void rage::fiStream::Close()
{
	if (m_bufferContentEnd == 0 && m_bufferCursorPos != 0)
		Flush();

	// TODO:
	// Mark as no longer needed?
	// p_Device->vftable + 0x60 (pData)

	m_handle = RAGE_INVALID_HANDLE;
	p_Device = nullptr;

	sm_activeStreams--;
}

void rage::fiStream::Flush()
{
	throw;
}

u32 rage::fiStream::Size()
{
	// TODO:
	// p_Device->vftable + 0x70 (p_FileData)

	throw;
}

int rage::fiStream::Read(const void* dest, u32 size)
{
	throw;
}

int rage::fiStream::Write(const void* data, u32 size)
{
	throw;
}

bool rage::fiStream::WriteChar(char c)
{
	return Write(&c, 1) != -1;
}

rage::fiStream rage::fiStream::sm_streams[] = {};
