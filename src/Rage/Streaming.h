#pragma once
#include "Pool.h"
#include "pgDictionary.h"
#include "grcTexture.h"

namespace rage
{
	template<typename T>
	class atArray
	{
		intptr_t items;
		int16_t size;
		int16_t unk10;
		int32_t unk18;

	public:
		int GetSize() const
		{
			return size;
		}

		T* GetAt(int index) const
		{
			if (index < 0 || index > size)
				return nullptr;

			// Not my brightest moment but how to get pointer
			// otherwise? It's not pointer array so indexing
			// is not an option
			return reinterpret_cast<T*>(items + sizeof(T) * index);
		}
	};

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
		_DWORD size_1;
		_DWORD size_2;

	public:
		int GetNodeIndex(int nameHash) const
		{
			return hashToIndeces[nameHash % hashToIndecesSize];
		}

		int FindNodeFromNodeIndexAndHashKey(int nodeIndex, int nameHash)
		{
			const atLinkedNode& indexNode = indexNodes[nodeIndex];

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

	struct grmShaderVariable
	{
		int64_t unk0;
		int64_t pValue;

		float GetFloat() const
		{
			return *(float*)pValue;
		}

		float* GetFloatPtr() const
		{
			return (float*)pValue;
		}

		bool GetBool() const
		{
			return *(bool*)pValue;
		}

		bool* GetBoolPtr() const
		{
			return (bool*)pValue;
		}

		grcTexture* GetTexture() const
		{
			return (grcTexture*)pValue;
		}
	};

	class grmShaderVariableDef
	{
	public:
		char pad_0000[8]; //0x0000
		char* Usage; //0x0008
		char* Name; //0x0010
		char pad_0018[48]; //0x0018
	}; //Size: 0x0048
	static_assert(sizeof(grmShaderVariableDef) == 0x48);

	class grmShaderMetadata
	{
	public:
		atArray<intptr_t> techniques;
		atArray<grmShaderVariableDef> variables;
		atArray<intptr_t> locals;
		intptr_t* vertexShader;
		char pad_0038[8];
		intptr_t* fragmentShader;
		char pad_0048[520];
		const char* shaderName; // ITS NOT SHADER NAME!!! Its pointer to shader!!!
		char pad_0258[104];
		const char* shaderPath;
		char pad_02C8[16];
	}; //Size: 0x02D8
	static_assert(sizeof(grmShaderMetadata) == 0x2D8);

	struct grmShaderDef
	{
		intptr_t values;
		grmShaderMetadata* metadata;

		grmShaderVariable* GetValueAtIndex(int index) const
		{
			return (grmShaderVariable*)(values + sizeof(grmShaderVariable) * index);
		}
	};

	struct grmShaderGroup
	{
		char pad_0000[16];
		grmShaderDef** shaders;
		int16_t numShaders;

		int FindVariableIndexByHashKey(uint32_t nameHash)
		{
			//int v2; // er8
			//__int64 v3; // r9
			//int64_t** i; // rax

			//v2 = 0;
			//v3 = 0i64;
			//if (!info->variables.size)
			//	return 0i64;
			//for (i = info->variables.items + 3; *(i + 1) != nameHash && *i != nameHash; i += 9)// i += 0x48... ida must be joking here
			//{                                             // disassembler makes no sense. must be items + 0x18
			//	++v3;
			//	++v2;
			//	if (v3 >= info->variables.size)
			//		return 0i64;
			//}
			//return (v2 + 1);

		}
	};

	class gtaDrawable
	{
	public:
		int64_t vftable;
		intptr_t phBounds;
		grmShaderGroup* grmShaderGroup;
		intptr_t crSkeletonData;
	};

	template<typename T1, typename T2>
	class fwAssetKeyValuePair
	{
		T1* value;
		T2 key;

	public:
		T1* GetValue()
		{
			return value;
		}

		T2 GetKey()
		{
			return key;
		}
	};

	enum eStreamingModule
	{
		STORE_DRAWABLE = 0x2,
		STORE_TXD = 0x3,
		STORE_MODELS = 0x15,
	};


	class strStreamingModuleMgr
	{
	public:
		strStreamingModule* GetStreamingModule(const eStreamingModule module)
		{
			return *reinterpret_cast<strStreamingModule**>(
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

	};


	template<typename T1, typename T2>
	class fwAssetStore : public strStreamingModule
	{
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
		atLinkedList linkedList;
		char pad_008C[4];
		int8_t IsPoolInitialized;

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
			if (templateEntryPtr <= 0xFFFFFFFFFi64)
				return false;

			// Might be not the best way to check it, but it works.
			// Around index 30,000 of TxdStore empty entries start appearing that trigger this
			const intptr_t templateEntry = *reinterpret_cast<intptr_t*>(templateEntryPtr);
			if (templateEntry <= 0xFFFFFFFFFi64)
				return false;

			return true;
		}
	};

	typedef fwAssetStore<pgDictionary<grcTexture>, fwTxdDef> TxdStore;
	typedef fwAssetStore<gtaDrawable, fwDwdDef> DrawableStore;
}
