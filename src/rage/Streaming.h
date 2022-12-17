#pragma once
#include "Pool.h"
#include "pgDictionary.h"
#include "grcTexture.h"
#include "grmShaderGroup.h"
#include "atArray.h"
#include "fragType.h"
#include "fwFragmentDef.h"
#include "atHashString.h"

namespace rage
{
	class strStreamingModule;

	struct atLinkedNode
	{
		int nameHash;
		int index;
		int nextIndex;
	};

	class atLinkedList
	{
		atLinkedNode* indexNodes;
		int* hashToIndeces;
		unsigned int hashToIndecesSize;
		int32_t size_1;
		int32_t size_2;

	public:
		int GetNodeIndex(int nameHash) const
		{
			return hashToIndeces[nameHash % hashToIndecesSize];
		}

		int FindNodeFromNodeIndexAndHashKey(int nodeIndex, int nameHash)
		{
			if (nodeIndex > size_1 || nodeIndex < 0)
				return -1;

			const atLinkedNode indexNode = indexNodes[nodeIndex];

			if (indexNode.nameHash == nameHash)
				return indexNode.index;

			if (indexNode.nextIndex == -1)
				return -1;

			return FindNodeFromNodeIndexAndHashKey(indexNode.nextIndex, nameHash);
		}

		int FindNodeFromHashKey(int nameHash)
		{
			return FindNodeFromNodeIndexAndHashKey(GetNodeIndex(nameHash), nameHash);
		}
	};

	struct fwTxdDef
	{
		int32_t unk0;
		uint32_t nameHash;
		int32_t unk10;
		int32_t unk14;
	}; //Size: 0x0010
	static_assert(sizeof(fwTxdDef) == 0x10);

	struct fwDwdDef
	{
		int32_t unk0;
		uint32_t nameHash;
		int64_t unk10;
		int64_t unk18;
	}; //Size: 0x0018
	static_assert(sizeof(fwDwdDef) == 0x18);

	struct CScaleformDef
	{
		int64_t qword0;
		int64_t qword8;
		int64_t qword10;
		int64_t qword18;
		int64_t qword20;
		int64_t qword28;
		int64_t qword30;
		int64_t qword38;
		char FiMemoryAddress[96];
		char Name[48];
	};
	static_assert(sizeof(CScaleformDef) == 0xD0);

	class CScaleformMovieObject
	{

	}; // 96 bytes

	class gtaDrawable
	{
	public:
		int64_t vftable;
		intptr_t phBounds;
		grmShaderGroup* grmShaderGroup;
		intptr_t crSkeletonData;
	};

	template<typename TValue, typename TDef>
	struct fwAssetKeyValuePair
	{
		TValue* value;
		TDef key;
	};

	// Note: there's much more streaming stores, but this is
	// enumeration of every supported one.
	enum eStreamingModule
	{
		STORE_DRAWABLE = 2,
		STORE_TXD = 3,
		STORE_FRAGMENTS = 4,
		STORE_SCALEFORM = 13,
		STORE_MODELS = 21,
	};

	class strStreamingModuleMgr
	{
	public:
		template<typename T>
		T* GetStreamingModule(const eStreamingModule module)
		{
			return *reinterpret_cast<T**>(
				*(intptr_t*)(this + 0x18) + sizeof(void*) * static_cast<int>(module));
		}

		//strStreamingModule* GetStreamingKeyEntry(const strStreamingModule* streamingModule, uint32_t index)
		//{
		//	return reinterpret_cast<streamingKeyEntry*>(
		//		g_streamingEntryKeys + sizeof(void*) * (streamingModule->keysOffset + static_cast<uint64_t>(index)));
		//}
	};

	class CStreaming
	{
		char pad_0000[440]; //0x0000
		strStreamingModuleMgr strStreamingMgr; //0x01B8

	public:
		strStreamingModuleMgr GetStreamingModuleMgr()
		{
			return strStreamingMgr;
		}
	};

	class strStreamingModule
	{
		int64_t vftable;
		int64_t unk8;
		int32_t size;
		int32_t dword14;
		char* m_DebugName;
		int64_t qword20_relatedToName;
		int8_t N000027F1;
		int8_t N00005ABD;
		int8_t N00005AC0;
		int8_t byte2B;
		int32_t N00005ABB;
		int32_t fileTypeId;
		int32_t dword34; // Not sure if it belongs to strStreamingModule or not (originally was in fwAssetStore)
	};

	class CModelInfoStreamingModule : public strStreamingModule // : public fwArchetypeStreamingModule
	{

	};

	template<typename T1, typename T2>
	class fwAssetStore : public strStreamingModule
	{
		// str streaming module
		/*int64_t vftable;
		int64_t unk8;
		int32_t size;
		int32_t dword14;
		char* m_DebugName;
		int64_t qword20_relatedToName;
		int8_t N000027F1;
		int8_t N00005ABD;
		int8_t N00005AC0;
		int8_t byte2B;
		int32_t N00005ABB;
		int32_t fileTypeId;*/


		// I Suppose pool (memory) + linked list (lookup) can be classified
		// as hashset?

		// This is not used like ped pool,
		// key list is unused, meaning IsSlotActive() cannot be used.
		// Additionally in RDR2 symbols fwAssetStore don't have
		// GetPool() or any other way to iterate through each items,
		// it's functionality is based fully on getting entry by hash key.
		// For modding purposes, we extend that with few additional methods.
		fwBasePool pool;
		int64_t pFileType;
		int32_t N0000447E;
		char pad_006C[4];
		// Serves purpose of hash look-up table for pool, as pool by default
		// doesn't have such functionality.
		// https://www.geeksforgeeks.org/separate-chaining-collision-handling-technique-in-hashing/
		atLinkedList linkedList;
		char pad_008C[4];
		int8_t m_PoolInitialized;

	public:
		typedef fwAssetKeyValuePair<T1, T2> fwAssetStoreValue;

		fwAssetStoreValue* FindSlotByHashKey(int& outIndex, int nameHash)
		{
			const int index = linkedList.FindNodeFromHashKey(nameHash);

			outIndex = index;

			if (index == -1)
				return nullptr;

			return pool.GetSlot<fwAssetStoreValue>(index);
		}

		fwAssetStoreValue* FindSlot(const char* name)
		{
			int index;
			return FindSlotByHashKey(index, atHashString(name));
		}

		bool Exists(const char* name)
		{
			const int index = linkedList.FindNodeFromHashKey(atHashString(name));
			return index != -1;
		}

		int GetSize() const
		{
			return pool.GetSize();
		}

		intptr_t GetSlotPtr(int index) const
		{
			return pool.GetSlotPtr(index);
		}

		fwAssetStoreValue* GetSlot(int index) const
		{
			return reinterpret_cast<fwAssetStoreValue*>(pool.GetSlotPtr(index));
		}

		bool IsSlotActive(int index) const
		{
			// Game still does this key check even though keys are untouched...
			// Could be inlined fwBasePool function.
			// Need to look more into this
			if (!pool.IsSlotActive(index))
				return false;

			const intptr_t templateEntryPtr = GetSlotPtr(index);
			if (templateEntryPtr <= 0xFFFFFFFFF)
				return false;

			// Might be not the best way to check it, but it works.
			// Around index 30,000 of TxdStore empty entries entriesStartIndex appearing that trigger this
			const intptr_t templateEntry = *reinterpret_cast<intptr_t*>(templateEntryPtr);
			if (templateEntry <= 0xFFFFFFFFF)
				return false;

			return true;
		}
	};
	static_assert(sizeof(fwAssetStore<void, void>) == 0x98);

	typedef fwAssetStore<CScaleformMovieObject, CScaleformDef> ScaleformStore;
	typedef fwAssetStore<pgDictionary<grcTexture>, fwTxdDef> TxdStore;
	typedef fwAssetStore<gtaDrawable, fwDwdDef> DrawableStore;
	typedef fwAssetStore<fragType, fwFragmentDef> FragmentStore;
}
