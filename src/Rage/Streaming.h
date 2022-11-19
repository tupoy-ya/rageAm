#pragma once
#include "Pool.h"

namespace rage
{
	class strStreamingModule;
	typedef intptr_t atIndexMap;

	enum eStreamingModule
	{
		STORE_TXD = 0x3,
		STORE_MODELS = 0x15,
	};

	class strStreamingModuleMgr
	{
	public:
		static strStreamingModule* GetStreamingModule(const eStreamingModule module)
		{
			return *reinterpret_cast<strStreamingModule**>(
				g_streamingModules + sizeof(void*) * static_cast<int>(module));
		}

		//strStreamingModule* GetStreamingKeyEntry(const strStreamingModule* streamingModule, uint32_t index)
		//{
		//	return reinterpret_cast<streamingKeyEntry*>(
		//		g_streamingEntryKeys + sizeof(void*) * (streamingModule->keysOffset + static_cast<uint64_t>(index)));
		//}
	};

	class strStreamingModule
	{

	};

	template<typename T>
	class fwAssetStore : public strStreamingModule
	{
	public:
		int64_t vftable;
		int64_t unk8;
		int32_t poolSize;
		char pad_0014[4];
		char* storeName;
		char pad_0020_relatedToName[8];
		int8_t N000027F1;
		int8_t N00005ABD;
		int8_t N00005AC0;
		char pad_002B[1];
		int32_t N00005ABB;
		int32_t fileTypeId;
		char pad_0034[4];
		fwBasePool pool;
		int64_t pFileType;
		int32_t N0000447E;
		char pad_006C[4];
		atIndexMap hashset;
		char pad_008C[4];
		int8_t IsPoolInitialized;

	public:
		T* FindSlotByHashKey(int& outIndex, int nameHash)
		{
			return nullptr;
		}

		fwBasePool GetPool() const
		{
			return pool;
		}
	};
}