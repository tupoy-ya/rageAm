#pragma once
#include <memory>
#include <vector>
#include "thread"
#include "../vendor/directxtk/include/DDSTextureLoader.h"

#include "DirectXMath.h"
#include <D3Dcompiler.h>
#pragma comment(lib,"D3dcompiler.lib")

#include <filesystem>
#include <future>
#include <ranges>
#include <shared_mutex>

#include "imgui.h"
#include "rage/fwModelId.h"
#include "rage/CModelInfo.h"
#include "../../rage/Streaming.h"
#include "../../memory/gmHelper.h"
#include "D3D.h"
#include "rage/TlsManager.h"

#include "boost/signals2.hpp"
#include "../../rage/Vec3V.h"

#include "../file_observer/FileObserverThread.h"

namespace rh
{
	class fwRenderThreadInterface
	{
		typedef void(*gDef_DefaultRenderFunction)(int64_t);
		typedef void(*gDef_DoRenderFunction)(int64_t);

		static inline gDef_DefaultRenderFunction gImpl_DefaultRenderFunction;
		static inline gDef_DoRenderFunction gImpl_DoRenderFunction;

		static void aImpl_DoRenderFunction(int64_t inst)
		{
			gImpl_DoRenderFunction(inst);
		}

	public:
		fwRenderThreadInterface()
		{
			// TODO: Somehow being invoked after DLL unloaded, causing exception (doesn't seem to happen on release target)
			/*
			gm::ScanAndHook(
				"fwRenderThreadInterface::DoRenderFunction",
				"48 89 5C 24 08 57 48 83 EC 20 48 8D 99 B0 00 00 00 48 8B F9 48 83",
				aImpl_DoRenderFunction,
				&gImpl_DoRenderFunction);

			gm::ScanAndSet(
				"fwRenderThreadInterface::DefaultRenderFunction",
				"48 89 5C 24 08 56 48 83 EC 20 48 8B F1 E8 ?? ?? ?? ?? 33 DB 84 C0 0F 84 96 00 00 00 38 1D",
				&gImpl_DefaultRenderFunction);
			*/
		}
	};

	class Rendering
	{
		// This command is called by render thread before rendering vehicle to update
		// shader buffers from CCustomShaderEffectVehicle.
		// We can use it to override various params right after without breaking any functionality.
		typedef void(*gDef_DrawCustomShaderEffectCommand)(intptr_t renderCtx);
		// This command handler (called by draw fragment draw command) executed by render thread
		// to draw fragment. Disabling this will disable vehicles, street lamps and other .yft models.
		typedef void (*gDef_DrawFragmentCommandHandler)(int64_t renderCtx, bool a2);
		// Draws non-standard (tuning) wheels on cars. Shares ms_CurrentShaderFxVehicle variable with
		// DrawCustomShaderEffectCommand (which sets it).
		typedef void (*gDef_DrawTuningWheelsCommandHandler)(int64_t renderCtx);
		// Override of rmcDrawable::DrawSkinned.
		typedef void(*gDef_fragDrawable_DrawSkinned)(int64_t renderData, rage::grmShaderGroup* shaderGroup, int64_t matrices, int64_t a4, uint32_t unkMask, uint32_t modelIndex);

		static inline gDef_DrawCustomShaderEffectCommand gImpl_DrawCustomShaderEffectCommand;
		static inline gDef_DrawFragmentCommandHandler gImpl_DrawFragmentCommandHandler;
		static inline gDef_DrawTuningWheelsCommandHandler gImpl_DrawTuningWheelsCommandHandler;
		static inline gDef_fragDrawable_DrawSkinned gImpl_fragDrawable_DrawSkinned;

		static void aImpl_DrawCustomShaderEffectCommand(intptr_t renderCtx)
		{
			rage::fwModelId modelId(*reinterpret_cast<uint16_t*>(renderCtx + 8));
			rage::CBaseModelInfo* modelInfo = rage::CModelInfo::GetModelInfoFromId(&modelId);

			m_UpdateVehicleShaderEffect = true;
			OnCustomShaderEffectVehicleUse(modelInfo->GetNameHash(), m_UpdateVehicleShaderEffect);

			if (m_UpdateVehicleShaderEffect)
				gImpl_DrawCustomShaderEffectCommand(renderCtx);
		}

		static void aImpl_DrawFragmentCommandHandler(int64_t renderCtx, bool a2)
		{
			rage::fwModelId modelId(*reinterpret_cast<uint16_t*>(renderCtx + 0xB4));
			rage::CBaseModelInfo* modelInfo = rage::CModelInfo::GetModelInfoFromId(&modelId);

			m_CurrentRenderingFragmentHash = modelInfo->GetNameHash();
			gImpl_DrawFragmentCommandHandler(renderCtx, a2);
			m_CurrentRenderingFragmentHash = -1;
		}

		static void aImpl_DrawTuningWheelsCommandHandler(int64_t a1)
		{
			// This function can't be called without DrawCustomShaderEffectCommand
			if (m_UpdateVehicleShaderEffect)
				gImpl_DrawTuningWheelsCommandHandler(a1);
		}

		static void aImpl_fragDrawable_DrawSkinned(int64_t renderData, rage::grmShaderGroup* shaderGroup, int64_t matrices, int64_t a4, uint32_t unkMask, uint32_t modelIndex)
		{
			OnFragRender(m_CurrentRenderingFragmentHash, shaderGroup ? &shaderGroup : nullptr);

			gImpl_fragDrawable_DrawSkinned(renderData, shaderGroup, matrices, a4, unkMask, modelIndex);
		}

#define BYTEn(x, n)   (*((_BYTE*)&(x)+n))
#define BYTE1(x)   BYTEn(x,  1)

		struct grmModel
		{
			int64_t vftable;
			int64_t* geometryList;
			int64_t qword10;
			int64_t qword18;
			int64_t materialIndeces;
			int16_t word28;
			int8_t byte2A;
			int8_t byte2B;
			int8_t byte2C;
			int8_t byte2D;
			int16_t word2E;
		};
		static_assert(sizeof(grmModel) == 0x30);

		typedef int8_t _BYTE;
		typedef int32_t _DWORD;
		typedef int64_t _QWORD;

		static void __fastcall aImpl_grmModel_DrawGeometries(
			grmModel* model,
			int a2,
			signed int startGeometryIndex,
			signed int numGeometries,
			rage::grmShaderGroup* shaderGroup,
			__int64 a6,
			unsigned int a7)
		{
			signed int geometryIndex; // ebx
			int v9; // er11
			__int64 tlsIndex; // rdx
			int v12; // er14
			rage::grcInstanceData* matOverride; // rax
			__int64 materialIndexOffset; // rsi
			rage::grcInstanceData* matOverride_; // r8
			rage::grcInstanceData* material; // r10
			unsigned int grmShader_flags; // edx
			bool v18; // dl
			bool v19; // al

			DWORD TlsIndex = 0; // Is it really 0?

			geometryIndex = startGeometryIndex;
			v9 = a2;
			if ((BYTE1(a7) & model->byte2C) != 0)
			{
				tlsIndex = (unsigned int)TlsIndex;
				v12 = -1;

				auto pTls = (int64_t)TlsGetValue(TlsIndex);
				pTls = 0x00000252BF01A920;

				//matOverride = *(_QWORD*)(*((_QWORD*)NtCurrentTeb()->ThreadLocalStoragePointer + (unsigned int)TlsIndex) + 0x7E0i64);
				//matOverride = *(_QWORD*)(pTls + 0x7E0);
				matOverride = rage::TlsManager::Get<rage::grcInstanceData*>(0x7E0);

				if (matOverride && (*(_BYTE*)(matOverride + 0x25) & 0x10) == 0)
					matOverride->qword20; // //v12 = *(_DWORD*)(matOverride + 0x20);

				if (startGeometryIndex < numGeometries)
				{
					materialIndexOffset = 2i64 * startGeometryIndex;
					do
					{
						//matOverride_ = *(rage::grcInstanceData**)(*((_QWORD*)NtCurrentTeb()->ThreadLocalStoragePointer + tlsIndex) + 0x7E0i64);// material override?
						matOverride_ = rage::TlsManager::Get<rage::grcInstanceData*>(0x7E0);
						if (matOverride_)
							material = matOverride_;
						else
							material = shaderGroup->materials[*(unsigned __int16*)(model->materialIndeces + materialIndexOffset)];
						if (v12 == -1)
							grmShader_flags = shaderGroup->materials[*(unsigned __int16*)(model->materialIndeces + materialIndexOffset)]->qword20;
						else
							grmShader_flags = v12;
						if (((grmShader_flags >> 8) & (a7 >> 8)) != 0 && ((unsigned __int8)grmShader_flags & (unsigned __int8)a7) != 0)// Check if geometry is visible maybe?
						{
							v18 = matOverride_
								|| geometryIndex + 1 < (unsigned __int16)model->word2E
								&& *(_DWORD*)material->GetEffect()->shaderNameHash == *(_DWORD*)shaderGroup->materials[*(unsigned __int16*)(model->materialIndeces + materialIndexOffset + 2)]->GetEffect()->shaderNameHash;
							v19 = geometryIndex + 1 == (unsigned __int16)model->word2E || !v18;

							//grmHelpers::DrawModel(material, v9, model, geometryIndex, a6, v19);// calls use shader
							//reinterpret_cast<void(*)(rage::grcInstanceData*, int, grmModel*, int, uint32_t, bool)>(0x7FF6E078BA98)(material, v9, model, geometryIndex, a6, v19);
							grmHelpers_DrawModel(material, v9, model, geometryIndex, a6, v19);

							v9 = a2;
						}
						tlsIndex = TlsIndex;
						++geometryIndex;
						materialIndexOffset += 2i64;
					} while (geometryIndex < numGeometries);  // geometry index / total geometries?
				}
			}
		}

		static void __fastcall grmHelpers_DrawModel(
			rage::grcInstanceData* material,
			int a2,
			grmModel* grmModel,
			unsigned int geometryIndex,
			__int64 a5,
			char a6)
		{
			__int64 geometryIndex_; // rsi
			signed int pass; // edi
			int numPasses; // ebp
			int64_t geometry; // rcx

			geometryIndex_ = geometryIndex;
			pass = 0;

			//numPasses = grcInstanceData::GetNumPassesForTechnique(material, a2, a6, 0);
			numPasses = reinterpret_cast<int(*)(rage::grcInstanceData*, int, char, int)>(0x7FF6E078A71C)(material, a2, a6, 0);

			if (numPasses > 0)
			{
				do
				{
					//grmShader::UsePass(material->grcEffect, pass, material);

					reinterpret_cast<void(*)(rage::grcEffect*, int, rage::grcInstanceData*)>(0x7FF6E0743FA8)(
						material->GetEffect(), pass, material);

					geometry = grmModel->geometryList[geometryIndex_];

					// TODO: Is other geometry types even used? Other than 'QB'
					grmGeometryQB_Function0x28(geometry, a5);
					// (*reinterpret_cast<void(__fastcall**)(int64_t, __int64)>(*(_QWORD*)geometry + 0x28))(geometry, a5);


					//sub_7FF71FBFD578();
					reinterpret_cast<void(*)()>(0x7FF6E074D578)();

					++pass;
				} while (pass < numPasses);
			}

			*(uint64_t*)(0x7FF6E2151598) = 0;
			//ms_lastShaderTechnique = 0i64;
		}

		// Used only once 
		static __int64 __fastcall sub_7FF71FC3A7A0(unsigned int* a1, rage::grcConstantBuffer* cBuffer)
		{
			//__int64 pLts; // rsi
			__int64 result; // rax
			unsigned int v6; // ebx

			//pLts = *((_QWORD*)NtCurrentTeb()->ThreadLocalStoragePointer + (unsigned int)TlsIndex);
			result = *a1;
			uint32_t unk7E8 = rage::TlsManager::Get<uint32_t>(0x7E8);
			//if (*(_DWORD*)(pLts + 0x7E8) != (_DWORD)result)// check to not set the same matrix twice?
			if (unk7E8 != (uint32_t)result)// check to not set the same matrix twice?
			{
				v6 = 0x30 * *((unsigned __int8*)a1 + 0x11);

				void* dst = cBuffer->Map();
				memmove(dst, a1 + 8, v6);
				cBuffer->Unmap();
				result = *a1;

				rage::TlsManager::Set(0x7E8, result);
				//*(_DWORD*)(pLts + 0x7E8) = result;
			}
			return result;
		}

		static void __fastcall grmGeometryQB_Function0x28(__int64 inst, __int64 a2)
		{
			sub_7FF71FC3A7A0((unsigned int*)a2, (rage::grcConstantBuffer*)0x00007FF6E215BFF0); // , g_ConstBuffer_RageBoneMtx
			(*(void(__fastcall**)(__int64))(*(_QWORD*)inst + 8i64))(inst);
		}












		static constexpr uint8_t GRC_MAX_NUM_SAMPLERS = 42;
		static constexpr uint8_t GRC_NUM_DRAW_BUCKETS = 6;

		// Possibly draw bucket, water_rivershallow.sps has bucket 6th render bucket
		// This is actually multi-dimensional array
		static inline rage::grcTexture** ms_TextureBucketArray = // [GRC_MAX_NUM_SAMPLERS * GRC_NUM_DRAW_BUCKETS]{};
			gm::GetGlobal<rage::grcTexture**>("ms_TextureBucketArray");

		static void RenderThread_AddTextureInRenderBucketSlot(uint8_t bucket, uint8_t slot, rage::grcTexture* texture)
		{
			uint8_t index = GRC_MAX_NUM_SAMPLERS * bucket;
			index += slot;

			ms_TextureBucketArray[index] = texture;
		}

		static inline ID3D11SamplerState** ms_SamplerList = *gm::GetGlobal<ID3D11SamplerState***>("ms_SamplerList");

		static void __fastcall SetTextureSampler(int slot, rage::grcTexture* texture, unsigned __int16 samplerIndex)
		{
			ID3D11DeviceContext* deviceContext = rage::TlsManager::GetD3D11Context();

			if (samplerIndex)
			{
				if (slot >= 16)
					return;

				auto sampler = ms_SamplerList + samplerIndex;
				deviceContext->PSSetSamplers(
					slot,
					1u,
					sampler);
			}

			RenderThread_AddTextureInRenderBucketSlot(1, slot, texture);
		}

		static bool __fastcall grcConstantBuffer_SetValue(
			const rage::grcConstantBuffer* inst,
			int a2,
			const void* pValue,
			int dataCount,
			int a5,
			int dataSize)
		{
			size_t size; // rbp
			char* v9; // r14

			size = dataSize * dataCount;
			v9 = (char*)inst->pValues + (unsigned int)(a2 * inst->dword38);
			if (!memcmp(&v9[a5 + 16], pValue, size))
				return false;
			v9[4] = 1;
			memmove(&inst->pValues->dataBegin + (unsigned int)(a2 * inst->dword38) + a5, pValue, size);

			return true;
		}

		static bool __fastcall grmHelpers_SetShaderLocalsValues(
			const void* pDataValue,
			int dataCount,
			int a5,
			rage::grcConstantBuffer* pLocalBuffer,
			unsigned __int8 dataType)
		{
			return grcConstantBuffer_SetValue(pLocalBuffer, 0, pDataValue, dataCount, a5, ms_GrmDataSizes[dataType]);
		}

		// Matrix values not match here for some reason, 16 instead of 64 (one row?)
		static inline uint8_t ms_GrmDataSizes[40]
		{
			0,
			0x4,
			0x4,
			0x8,
			0xC,
			0x10,
			0x0,
			0x4,
			0x10,
			0x10,
			0x0,
			0x4,
			0x8,
			0xC,
			0x10,
			0x0,
			0x0,
			0x4,
			0x4,
			0x4,
			0x4,
			// 20 - 39
			0x0, 0x0, 0x0, 0x0, 0x0,
			0x0, 0x0, 0x0, 0x0, 0x0,
			0x0, 0x0, 0x0, 0x0, 0x0,
			0x0, 0x0, 0x0, 0x0
		};

		static inline rage::grcFragmentProgram** ms_CurrentFragmentProgram =
			gm::GetGlobal<rage::grcFragmentProgram**>("ms_CurrentFragmentProgram");

		static inline uint32_t* ms_CurrentFragmentProgramFlag =
			gm::GetGlobal<uint32_t*>("ms_CurrentFragmentProgramFlag");

		static void __fastcall grcPixelProgram_Bind(rage::grcFragmentProgram* program)
		{
			if (*ms_CurrentFragmentProgram == program)
				return;

			*ms_CurrentFragmentProgram = program;

			ID3D11DeviceContext* context = rage::TlsManager::GetD3D11Context();
			context->PSSetShader((ID3D11PixelShader*)program->pShaderD3D, nullptr, 0);

			if (program->pShaderD3D)
				*ms_CurrentFragmentProgramFlag |= 2u;
			else
				*ms_CurrentFragmentProgramFlag &= 0xFFFFFFFD; // ~2u
		}

		static inline uint16_t* ms_SamplerIndices = gm::GetGlobal<uint16_t*>("ms_SamplerIndices");
		// Size of 42 too
		static inline rage::grcTexture** ms_TextureArray = gm::GetGlobal<rage::grcTexture**>("ms_TextureArray");

		static void __fastcall grcEffect_BeginPass_FragmentProgram(
			rage::grcInstanceData* material,
			rage::grcFragmentProgram* program,
			rage::grcEffectVar** variables)
		{
			__int64 numVars; // rsi
			uint32_t variableIndex; // rdx
			uint8_t slot;
			uint16_t samplerArrayIndex; // r8
			rage::grcTexture* texture; // rdx
			int dataType; // er10
			int slot_unk34; // eax
			rage::grcInstanceVar* materialValue; // rcx

			for (int i = 0; i < program->numUnkVars; i++)
			{
				variableIndex = program->FlagsAndIndices[i];
				if ((program->FlagsAndIndices[i] & 0xF00) == 0)
				{
					dataType = (*variables)[variableIndex].dataType;
					slot_unk34 = (*variables)[variableIndex].unk34;
					materialValue = material->GetVariableAtIndex(variableIndex);
					if (dataType == rage::EFFECT_VALUE_TEXTURE)
					{
						samplerArrayIndex = materialValue->unk2;
						texture = materialValue->GetTexture();
						slot = slot_unk34;
						goto LABEL_14;
					}
					else
					{
						if (dataType == 15 || dataType == 21)
							// Not really texture, some weird structure
							RenderThread_AddTextureInRenderBucketSlot(1, slot_unk34, materialValue->GetTexture());
						else
							// Without peds appear emissive green
							grmHelpers_SetShaderLocalsValues(
								(void*)materialValue->pDataValue,
								materialValue->dataCount,
								slot_unk34,
								(*variables)[variableIndex].pLocalBuffer,
								dataType);
					}
				}
				else
				{
					slot = variableIndex & 0xFF;
					if ((program->FlagsAndIndices[i] & 0xF00) == 256)
					{
						if (slot < 16u)
							// Without this world starts shinning,
							// a bit breaks graphics
							// maybe some spec related texture?
							// Also remove 'blur / roughness' from textures
							//samplerArrayIndex = ((uint16_t*)0x7FF6E21517F0)[slot]; // sampler indices
							samplerArrayIndex = ms_SamplerIndices[slot]; // sampler indices
						else
							samplerArrayIndex = 0;
						//texture = ((rage::grcTexture**)0x7FF6E21516A0)[slot]; // texture list
						texture = ms_TextureArray[slot];
					LABEL_14:

						SetTextureSampler(slot, texture, samplerArrayIndex);
					}
				}
			}

			grcPixelProgram_Bind(program);
		}

		static inline ID3D11ShaderResourceView** ms_ShaderResourceViews = //[GRC_MAX_NUM_SAMPLERS]{};
			gm::GetGlobal<ID3D11ShaderResourceView**>("ms_ShaderResourceViews");

		static void __fastcall grcFragmentProgram_SetBucketResources(rage::grcFragmentProgram* shaderProgram, uint8_t bucket)
		{
			int shaderParams; // eax
			unsigned int __endSlot; // ebp
			char v6; // cl
			int effectVarIndex; // esi
			unsigned int startSlot; // er13
			__int64 shaderResourceIndex; // r12
			ID3D11ShaderResourceView* resourceView; // rax
			char v17; // [rsp+50h] [rbp+8h]

			shaderParams = shaderProgram->numEffectVars;
			bucket = (int)bucket;
			if (!shaderParams)
				return;
			__endSlot = 0;
			v6 = 0;
			startSlot = 41;
			v17 = 0;
			if (shaderParams <= 0)
				return;
			for (int i = 0; i < shaderProgram->numEffectVars; i++)
			{
				rage::grcEffectVar* effectVar = (&shaderProgram->effectVarArray)[i];

				if (!effectVar)
					continue;

				int dataType = effectVar->dataType;
				uint32_t resourceViewIndex = effectVar->unk34;
				if (dataType == rage::EFFECT_VALUE_TEXTURE)
				{
					shaderResourceIndex = resourceViewIndex;
					rage::grcTexture* texture = ms_TextureBucketArray[GRC_MAX_NUM_SAMPLERS * bucket + resourceViewIndex];
					if (!texture)
						goto LABEL_17;

					resourceView = texture->GetShaderResourceView();

					// Global texture swap
					// TODO: This is slow in debug mode!
					fiobs::g_TextureSwapThread.GetTextureSwap(texture->GetName(), &resourceView);
				}
				else
				{
					// what about 21?
					if (dataType != 15)
						goto LABEL_17;
					shaderResourceIndex = effectVar->unk34;

					// Some unknown structure (grc texture alternative)
					resourceView = *(ID3D11ShaderResourceView**)(((uint64_t*)ms_TextureBucketArray)[GRC_MAX_NUM_SAMPLERS * bucket + resourceViewIndex] + 0x28);
				}
				if (!resourceView || ms_ShaderResourceViews[shaderResourceIndex] == resourceView)
				{
				LABEL_17:
					v6 = v17;
					continue;
				}
				v6 = 1;
				ms_ShaderResourceViews[shaderResourceIndex] = resourceView;
				if (startSlot >= resourceViewIndex)
					startSlot = resourceViewIndex;
				v17 = 1;
				if (__endSlot <= resourceViewIndex)
					__endSlot = resourceViewIndex;
			}

			if (v6 && bucket && bucket == 1)
			{
				ID3D11DeviceContext* pContext = rage::TlsManager::GetD3D11Context();
				ID3D11ShaderResourceView** resourceViews = ms_ShaderResourceViews + startSlot;

				uint32_t numViews = __endSlot - startSlot + 1;
				pContext->PSSetShaderResources(startSlot, numViews, resourceViews);
			}
		}

		static inline bool m_UpdateVehicleShaderEffect;
		static inline uint32_t m_CurrentRenderingFragmentHash;
	public:
		/**
		 * \brief Invoked before fragment rendered.
		 * \param hash Name hash of the model.
		 * \param lpShaderGroup Long pointer on grmShaderGroup that used to render fragment.
		 * \remarks - lpShaderGroup parameter can be NULL.
		 * \n - lpShaderGroup can be replaced with custom one (see material editor).
		 * \n - Make sure that the only difference in shader group is material values, otherwise
		 * geometry render function will crash with out of bounds exception while iterating materials.
		 * \n - _hi and regular model name hashes are different.
		 */
		static inline boost::signals2::signal<void(uint32_t hash, rage::grmShaderGroup** lpShaderGroup)> OnFragRender;

		/**
		 * \brief Invoked before CCustomShaderEffectVehicle updates values in grmShaderGroup.
		 * \param hash Name hash of the model.
		 * \param execute if set to false, CCustomShaderEffectVehicle won't be executed.
		 * \remarks - Just after this function fragment is being rendered. See @OnFragRender.
		 * \n - _hi and regular model name hashes are different.
		 */
		static inline boost::signals2::signal<void(uint32_t hash, bool& execute)> OnCustomShaderEffectVehicleUse;

		Rendering()
		{
			gm::ScanAndHook("DrawTuningWheelsCommandHandler",
				"48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 68 A1 48 81 EC F8 00 00 00 44 8B 3D",
				aImpl_DrawTuningWheelsCommandHandler,
				&gImpl_DrawTuningWheelsCommandHandler);

			gm::ScanAndHook(
				"DrawCustomShaderEffectCommand",
				"48 89 5C 24 10 48 89 74 24 18 57 48 83 EC 30 F7",
				aImpl_DrawCustomShaderEffectCommand,
				&gImpl_DrawCustomShaderEffectCommand);

			// This function responsible for drawing every fragment in game
			gm::ScanAndHook("DrawFragmentCommandHandler",
				"88 54 24 10 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 E1 48 81 EC A8 00 00 00 8B",
				aImpl_DrawFragmentCommandHandler,
				&gImpl_DrawFragmentCommandHandler);

			// NOTICE: Car wheels are apparently not frag drawable
			gm::ScanAndHook("fragDrawable::DrawSkinned",
				"48 8B C4 48 89 58 08 48 89 68 10 48 89 70 20 4C 89 40 18 57 41 54 41 55 41 56 41 57 48 83 EC 70 48",
				aImpl_fragDrawable_DrawSkinned,
				&gImpl_fragDrawable_DrawSkinned);

			// TODO: Breaks windows (don't move with car door)
			/*
			gm::ScanAndHook("grmModel::DrawGeometries",
				"48 8B C4 48 89 58 08 48 89 68 18 48 89 70 20 89 50 10 57 41 54 41 55 41 56 41 57 48 83 EC 30 44",
				aImpl_grmModel_DrawGeometries);
			*/

			gm::ScanAndHook("grcConstantBuffer::SetValue",
				"48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 56 41 57 48 83 EC 20 44 8B 71 38",
				grcConstantBuffer_SetValue);

			gm::gmAddress addr = g_Scanner.ScanPattern("grcEffect::BeginPass_FragmentProgram", "E8 ?? ?? ?? ?? 4C 8B 44 24 70 48 8B");
			addr = addr.GetAt(0x10).GetCall();
			g_Hook.SetHook(addr, grcEffect_BeginPass_FragmentProgram);

			gm::ScanAndHook("grcFragmentProgram::SetBucketResources",
				"48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 41 54 41 55 41 56 41 57 48 83 EC 20 8B 81 A8",
				grcFragmentProgram_SetBucketResources);
		}

		uint32_t GetCurrentRenderingFragmentHash() const
		{
			return m_CurrentRenderingFragmentHash;
		}

		/*void AddOverrideShaderParamThisFrame(int entity, const char* shaderName, int materialId, const char* paramName)
		{

		}*/
	};

	class RenderThread
	{
		typedef void(*gDef_PresentImage)();

		static inline gDef_PresentImage gImpl_PresentImage;

		static void aImpl_PresentImage()
		{
			gImpl_PresentImage();
		}

	public:
		RenderThread()
		{
			gm::ScanAndHook("PresentImage", "40 55 53 56 57 41 54 41 56 41 57 48 8B EC 48 83 EC 40 48 8B 0D",
				aImpl_PresentImage,
				&gImpl_PresentImage);
		}
	};

	inline Rendering g_Rendering;
	inline fwRenderThreadInterface g_FwRenderThreadInterface;
	inline RenderThread g_RenderThread;
}
