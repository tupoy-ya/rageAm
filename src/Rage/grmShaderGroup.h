#pragma once
#include "../Logger.h"
#include "atArray.h"
#include "fwTypes.h"
#include "grcTexture.h"
#include "pgDictionary.h"
#include <d3d11.h>

namespace rage
{
	typedef int64_t grcVertexProgram;

	struct grcFragmentProgram
	{
		int8_t gap0[552];
		ID3D11PixelShader* pPixelShaderD3D11;
		int64_t qword230;
	};
	static_assert(sizeof(grcFragmentProgram) == 0x238);

	struct grmMaterialVariable
	{
		uint8_t dataCount;
		int8_t unk1;
		int8_t unk2;
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

	struct grmPass
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

	struct grmTechnique
	{
		int8_t gap0[8];
		const char* name;
		grmPass* pPasses;
		uint16_t numPasses;
		int16_t unk1A;
		int16_t unk1C;
		int16_t unk20;
	};

	struct grmShaderVariable
	{
		uint8_t dataType;
		int8_t unk1;
		int16_t unk2;
		int32_t unk4;
		char* Usage;
		char* Name;
		int64_t unk18;
		int64_t unk20;
		int64_t unk28;
		int32_t unk30;
		int32_t unk34;
		int64_t unk38;
		int64_t pLocalBuffer;
	};
	static_assert(sizeof(grmShaderVariable) == 0x48);

	struct grmConstantBufferData
	{
		int32_t dword0;
		int8_t byte4;
		int8_t byte5;
		int8_t byte6;
		int8_t byte7;
		int64_t qword8;
		int8_t dataSrc;
	};

	struct __declspec(align(4)) grmConstantBuffer
	{
		int32_t size;
		int32_t word4;
		int64_t qword8;
		int64_t qword10;
		const char* name;
		int64_t pConstantBufferD3D;
		grmConstantBufferData* pValues;
		grmConstantBufferData* pValues2;
		int32_t dword38;
	};

	struct grmShaderPack
	{
		atArray<grmTechnique> techniques;
		atArray<grmShaderVariable> variables;
		atArray<grmConstantBuffer> locals;
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
		int8_t shaderNameHash[8];
		int64_t qword2D0;
		int32_t dword2D8;
		int32_t qword2dc;
		int32_t qword2e0;
		int16_t unusedint16_t2e4;
		int16_t word2e6;
		int64_t qword2E8;
		int32_t dword2F0;
		int8_t gap2F4[4];
		int64_t pComputePrograms;
		int32_t dword300;
		int8_t gap304[4];
		int64_t pDomainPrograms;
		int32_t dword310;
		int8_t gap314[4];
		int64_t pGeometryPrograms;
		int32_t dword320;
		int8_t gap324[4];
		int64_t pHullPrograms;
		int32_t dword330;
	};

	// Unique per drawable instance.
	struct grmMaterial
	{
		grmMaterialVariable* variables;
		grmShaderPack* shaderPack;
		int8_t numVariables;
		int64_t qword18;
		int64_t qword20;

		grmMaterialVariable* GetVariableAtIndex(int index) const
		{
			return &variables[index];
		}
	};
	static_assert(sizeof(grmMaterial) == 0x28);

	struct grmShaderGroup
	{
		//char pad_0000[16];
		//grmMaterial** materials;
		//int16_t numMaterials;
		int64_t qword0;
		pgDictionary<grcTexture>* pEmbedTextures;
		grmMaterial** materials;
		int16_t numMaterials;
		int16_t unk1A;
		int64_t qword20;
		int32_t dword28;
		__declspec(align(8)) int32_t dword30;
		int64_t qword38;

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
