#pragma once
#include <memory>
#include <vector>

#include <D3Dcompiler.h>
#pragma comment(lib,"D3dcompiler.lib")

#include "rage/fwModelId.h"
#include "rage/CModelInfo.h"
#include "../../rage/Streaming.h"
#include "../../memory/gmHelper.h"
#include "rageDX11.h"

#include "boost/signals2.hpp"

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
