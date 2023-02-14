#include "datCompiler.h"

#include "TlsManager.h"

rage::datCompiler::datCompiler():
	m_VirtualAllocator(DEFAULT_HEAP_SIZE_VIRTUAL, true),
	m_PhysicalAllocator(DEFAULT_HEAP_SIZE_PHYSICAL, false)
{
	TlsManager::SetCompiler(this);
}

rage::datCompiler::~datCompiler()
{
	TlsManager::SetCompiler(nullptr);
}
