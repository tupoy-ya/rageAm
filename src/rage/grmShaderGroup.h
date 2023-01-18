#pragma once
#include "template/array.h"
#include "paging/pgDictionary.h"
#include "grcTexture.h"
#include "grcProgram.h"
#include "TlsManager.h"

#include <d3d11.h>

#ifndef RAGE_STANDALONE
#include "Logger.h"
#endif

namespace rage
{
	struct grcConstantBuffer;

	struct grcInstanceVar
	{
		uint8_t dataCount;
		int8_t unk1;
		uint8_t unk2;
		int8_t unk3;
		int32_t unk4;
		int64_t pDataValue;

		float GetFloat() const
		{
			return *reinterpret_cast<float*>(pDataValue);
		}

		float* GetFloatPtr() const
		{
			return reinterpret_cast<float*>(pDataValue);
		}

		bool GetBool() const
		{
			return *reinterpret_cast<bool*>(pDataValue);
		}

		bool* GetBoolPtr() const
		{
			return reinterpret_cast<bool*>(pDataValue);
		}

		grcTexture* GetTexture() const
		{
			return reinterpret_cast<grcTexture*>(pDataValue);
		}
	};

	struct grcPass
	{
		uint8_t VS;
		uint8_t PS;
		uint8_t CS;
		uint8_t DS;
		uint8_t GS;
		uint8_t HS;
		uint8_t uint6;
		uint8_t uint7;
		uint8_t uint8;
		uint8_t uint9;
		uint8_t uintA;
		uint8_t uintB;
	};

	struct grcTechnique
	{
		int8_t gap0[8];
		const char* name;
		grcPass* pPasses;
		uint16_t numPasses;
		int16_t unk1A;
		int16_t unk1C;
		int16_t unk20;
	};

	enum eEffectValueType
	{
		EFFECT_VALUE_NONE = 0,
		EFFECT_VALUE_INT = 1, // Unknown
		EFFECT_VALUE_FLOAT = 2, // 4 bytes
		EFFECT_VALUE_VECTOR2 = 3, // 8 bytes
		EFFECT_VALUE_VECTOR3 = 4, // 16 bytes (used only 3)
		EFFECT_VALUE_VECTOR4 = 5, // 16 bytes
		EFFECT_VALUE_TEXTURE = 6, // A pointer to grcTexture
		EFFECT_VALUE_BOOL = 7, // 1 byte
		EFFECT_VALUE_MATRIX34 = 8, // Unknown
		EFFECT_VALUE_MATRIX44 = 9, // 64 bytes, 4x4 matrix apparently (see vehicle_tire.fxc)
		EFFECT_VALUE_STRING = 10, // Null terminated?
		EFFECT_VALUE_INT_ = 11, // How different from the first one?
		EFFECT_VALUE_INT2 = 12, // 
		EFFECT_VALUE_INT3 = 13, // 
		EFFECT_VALUE_INT4 = 14, // 
	};

	inline const char* EffectValueTypeToString(eEffectValueType e)
	{
		switch (e)
		{
		case EFFECT_VALUE_NONE: return "NONE";
		case EFFECT_VALUE_INT: return "int";
		case EFFECT_VALUE_FLOAT: return "float";
		case EFFECT_VALUE_VECTOR2: return "vector2";
		case EFFECT_VALUE_VECTOR3: return "vector3";
		case EFFECT_VALUE_VECTOR4: return "vector4";
		case EFFECT_VALUE_TEXTURE: return "grcTexture";
		case EFFECT_VALUE_BOOL: return "bool";
		case EFFECT_VALUE_MATRIX34: return "matrix34";
		case EFFECT_VALUE_MATRIX44: return "matrix44";
		case EFFECT_VALUE_STRING: return "string";
		case EFFECT_VALUE_INT_: return "int";
		case EFFECT_VALUE_INT2: return "int2";
		case EFFECT_VALUE_INT3: return "int3";
		case EFFECT_VALUE_INT4: return "int4";
		}
		return "Unknown";
	}

	inline uint8_t GetEffectValueSize(eEffectValueType type)
	{
		// TODO: There's also: Matrix3x4, Matrix4x4
		switch (type)
		{
		case EFFECT_VALUE_FLOAT: return 4;
		case EFFECT_VALUE_VECTOR2: return 8;
		case EFFECT_VALUE_VECTOR3:
		case EFFECT_VALUE_VECTOR4: return 16;
		case EFFECT_VALUE_TEXTURE: return 8;
		case EFFECT_VALUE_BOOL: return 1;
		case EFFECT_VALUE_MATRIX34: return 48; // Not sure
		case EFFECT_VALUE_MATRIX44: return 64;

		default: return 0;
		}
	}

	class grcEffectUIVar
	{
	public:
		char* Name; //0x0000 Name
		char* N00000180; //0x0008 InShaderName
		char pad_0010[8]; //0x0010
		char* UIType; //0x0018
		char* UIName; //0x0020
		char pad_0028[40]; //0x0028
		float Min; //0x0050
		float Max; //0x0054
		float Step; //0x0058
		float N0000052F; //0x005C
		char pad_0060[64]; //0x0060
	}; //Size: 0x00A0
	static_assert(sizeof(grcEffectUIVar) == 0xA0);

	struct grcEffectVar
	{
		uint8_t dataType;
		int8_t unk1;
		int16_t unk2;
		int32_t unk4;
		const char* InShaderName;
		const char* Name;
		int64_t unk18;
		int64_t unk20;
		int64_t unk28;
		int32_t unk30;
		int32_t unk34;
		int64_t unk38;
		grcConstantBuffer* pLocalBuffer;

		const char* GetDisplayName() const
		{
			//static char buffer[128];
			//sprintf_s(buffer, 128, "%s; %s", Name, InShaderName);
			//return buffer;
			return Name;
		}

		eEffectValueType GetValueType() const
		{
			return static_cast<eEffectValueType>(dataType);
		}
	};
	static_assert(sizeof(grcEffectVar) == 0x48);

	struct grcConstantBufferData
	{
		int32_t dword0;
		int8_t byte4;
		int8_t byte5;
		int8_t byte6;
		int8_t byte7;
		int64_t qword8;
		int8_t dataBegin;
	};

	struct grcConstantBuffer
	{
		int32_t size;
		int32_t word4;
		int64_t qword8;
		int64_t qword10;
		const char* name;
		ID3D11Resource* pConstantBufferD3D;
		grcConstantBufferData* pValues;
		grcConstantBufferData* pValues2;
		int32_t dword38;
		int32_t dword3C;

		void* Map() const
		{
			ID3D11DeviceContext* deviceContext = TlsManager::GetD3D11Context();
			D3D11_MAPPED_SUBRESOURCE resource;

			deviceContext->Map(
				pConstantBufferD3D,
				0,
				D3D11_MAP_WRITE_DISCARD,
				0,
				&resource);

			pValues->byte5 = 1;
			pValues->byte6 = 0;

			return resource.pData;
		}

		void Unmap() const
		{
			ID3D11DeviceContext* deviceContext = TlsManager::GetD3D11Context();

			deviceContext->Unmap(pConstantBufferD3D, 0);

			grcConstantBufferData* result = pValues;
			*(int16_t*)&result->byte4 = 0; // Looks like flags?
		}
	};
	static_assert(sizeof(grcConstantBuffer) == 0x40);

	struct grcEffect
	{
		const char* GetFilePath() const
		{
			return shaderFilePath;
		}

		const char* GetFileName() const
		{
			return strrchr(GetFilePath(), '/') + 1;
		}

		atArray<grcEffectVar> GetVariables()
		{
			return variables;
		}

		atArray<grcTechnique> techniques;
		atArray<grcEffectVar> variables;
		atArray<grcConstantBuffer> locals;
		atArray<grcVertexProgram> pVertexPrograms;
		atArray<grcFragmentProgram> pFragmentPrograms;

		int8_t gap50[508];
		int64_t qword250;
		int32_t dword258;
		int32_t unk25c;
		const char* name;
		int32_t unk268;
		int32_t unk26c;
		int8_t gap25C[24];
		int64_t qword288;
		int64_t qword290;
		int16_t word298;
		int16_t dword29A;
		int16_t word29C;
		int16_t word29E;
		int64_t qword2A0;
		int32_t dword2A8;
		int32_t qword2AC;
		int32_t dword2B0;
		int32_t dword2B4;
		int64_t fileTime;
		const char* shaderFilePath;
		int32_t shaderNameHash;
		int32_t dword2CC;
		int64_t qword2D0;
		int32_t dword2D8;
		int32_t qword2dc;
		int32_t qword2e0;
		int16_t unusedint16_t2e4;
		int16_t word2e6;
		// int64_t qword2E8;
		atArray<grcEffectUIVar> UIVariables;
		atArray<grcComputeProgram> pComputePrograms;
		atArray<grcDomainProgram> pDomainPrograms;
		atArray<grcGeometryProgram> pGeometryPrograms;
		atArray<grcHullProgram> pHullPrograms;
	};
	static_assert(sizeof(grcEffect) == 0x338);
	static_assert(offsetof(grcEffect, shaderFilePath) == 0x2C0);

	struct grcInstanceData
	{
		grcInstanceVar* variables;
		grcEffect* effect;
		int8_t numVariables;
		int64_t qword18;
		int64_t qword20;

		eEffectValueType GetValueTypeAt(u16 index) const
		{
			return GetEffect()->GetVariables()[index].GetValueType();
		}

		grcInstanceVar* GetVariableAtIndex(int index) const
		{
			return &variables[index];
		}

		grcEffect* GetEffect() const
		{
			return effect;
		}
	};
	static_assert(sizeof(grcInstanceData) == 0x28);

	/**
	 * \brief Though it's called shader group, in fact it's 'material library'
	 * of some game model (shared per - model, not per - instance).
	 * \remarks
	 * - While it's possible to easily edit material values for regular drawables' (such as static map props),
	 * for vehicles, weapons etc with 'specific shaders' there's CCustomShaderEffect (later - shaderFx)
	 * that provides abstraction on shader and allows to easily set some parameters like car paint, dirt level, enveff.
	 * \n - Just before rendering model, shaderFx update function is invoked, updating material values in grmShaderGroup,
	 * and that makes it impossible to easily edit material values. (except for the ones that are
	 * unused by shaderFx). So for material editor we have to disable it to be able to edit any value.
	 */
	struct grmShaderGroup
	{
		int64_t qword0;
		pgDictionary<grcTexture>* pEmbedTextures;
		grcInstanceData** InstanceData;
		int16_t numMaterials;
		int16_t unk1A;
		int64_t qword20;
		int32_t dword28;
		__declspec(align(8)) int32_t dword30;
		int64_t qword38;

		int GetEffectCount() const
		{
			return numMaterials;
		}

		grcInstanceData* GetInstanceDataAt(int index) const
		{
			return InstanceData[index];
		}

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
			return 0;
		}
	};
}
