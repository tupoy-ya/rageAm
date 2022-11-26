#include "fiStream.h"

rage::fiStream::fiStream(fiDevice* pDevice, intptr_t pData, char* buffer)
{
	p_Device = pDevice;
	p_FileData = pData;
	m_buffer = buffer;

	m_deviceCursorPos = 0;
	m_bufferCursorPos = 0;
	m_bufferContentEnd = 0;
	m_dword2C = 0;

	// Not sure what is the point of every stream having
	// size of buffer when it's constant. (is it?)
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

	// TODO: fiDevice->vftable + 0x28 (resourceName)
	intptr_t pData = -1;

	if (pData == -1)
		return nullptr;

	return AllocStream(resourceName, pData, device);
}

rage::fiStream* rage::fiStream::Open(const char* resourceName)
{
	if (!HasAvailableStreams())
		return nullptr;

	// TODO: fiDevice::GetDeviceImpl
	fiDevice* device = nullptr;

	if (!device)
		return nullptr;

	// TODO: fiDevice->vftable + 0x8 (resourceName, true)
	intptr_t pData = -1;

	if (pData == -1)
		return nullptr;

	return AllocStream(resourceName, pData, device);
}

rage::fiStream* rage::fiStream::AllocStream(const char* resourceName, intptr_t pData, fiDevice* device)
{
	// Resource name is actually passed in fiStream::AllocStream but not used,
	// most likely was simply logged somewhere.
	g_logger->Log("Allocating stream for: {}. Data Ptr: {:X}. Device: {:X}",
		resourceName, reinterpret_cast<intptr_t>(pData), reinterpret_cast<intptr_t>(device));

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

	p_FileData = -1;
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
