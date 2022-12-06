#pragma once
#include <memory>
#include <vector>

#include <D3Dcompiler.h>
#pragma comment(lib,"D3dcompiler.lib")

#include "../../rage/Streaming.h"
#include "../../memory/gmHelper.h"
#include "rageDX11.h"

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
			gm::ScanAndHook(
				"fwRenderThreadInterface::DoRenderFunction",
				"48 89 5C 24 08 57 48 83 EC 20 48 8D 99 B0 00 00 00 48 8B F9 48 83",
				aImpl_DoRenderFunction,
				&gImpl_DoRenderFunction);

			gm::ScanAndSet(
				"fwRenderThreadInterface::DoRenderFunction",
				"48 89 5C 24 08 56 48 83 EC 20 48 8B F1 E8 ?? ?? ?? ?? 33 DB 84 C0 0F 84 96 00 00 00 38 1D",
				&gImpl_DefaultRenderFunction);
		}
	};

	class DrawCommands
	{
		static inline bool m_shaderUpdated = false;

		// This command is called by render thread before rendering vehicle to update
		// shader buffers from CCustomShaderEffectVehicle.
		// We can use it to override various params right after without breaking any functionality.
		typedef void(*gDef_DrawCustomShaderEffectCommand)(intptr_t renderData);

		static inline gDef_DrawCustomShaderEffectCommand gImpl_DrawCustomShaderEffectCommand = nullptr;

		static void aImpl_DrawCustomShaderEffectCommand(intptr_t renderData)
		{
			gImpl_DrawCustomShaderEffectCommand(renderData);

			return;
			// TODO: Don't use these
			intptr_t fragType = *(intptr_t*)0x7FF7BB06B620;
			intptr_t shaderFx = *(intptr_t*)0x7FF7BB06B630;

			gImpl_DrawCustomShaderEffectCommand(renderData);

			// if (!(shaderFx == 0x1E145B21060 || shaderFx == 0x1E145C97870))
			//	return;

			const char* fragName = *(const char**)(fragType + 0x58);

			//if (strcmp(fragName, "pack:/adder_hi") != 0)
			//	return;

			rage::gtaDrawable* drawable = *(rage::gtaDrawable**)(fragType + 0x30);

			rage::grmShaderGroup* shaderGroup = drawable->grmShaderGroup;

			for (int k = 0; k < shaderGroup->numMaterials; k++)
			{
				rage::grmMaterial* material = shaderGroup->materials[k];

				if (!m_shaderUpdated)
				{
					for (int i = 0; i < material->shaderPack->pFragmentPrograms.GetSize(); i++)
					{
						auto fp = material->shaderPack->pFragmentPrograms.GetAt(i);

						// g_Log.Log("{:X}", fp);
						auto fpName = *(const char**)((intptr_t)fp + 0x8);

						//g_Log.Log(fpName);

						if (strcmp(fpName, "ranstar_paint1:PS_DeferredVehicleTextured") == 0)
						{
							m_shaderUpdated = true;
							ID3DBlob* psBlob;
							D3DCompileFromFile(L"S:/Repos/ShaderCompiler/ShaderCompiler/PS_DeferredVehicleTextured.hlsl",
								NULL, NULL, "main", "ps_5_0", NULL, NULL, &psBlob, NULL);

							ID3D11PixelShader* pPs;
							grcDX11::GetDevice()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &pPs);
							fp->pPixelShaderD3D11 = pPs;
						}

						//*(ID3D11PixelShader*)(fp + 0x228) = nullptr;
					}
				}

				for (int i = 0; i < material->numVariables; i++)
				{
					//g_Log.Log(material->shaderPack->variables.GetAt(i)->Name);
					if (strcmp(material->shaderPack->variables.GetAt(i)->Name, "ranstarScale") == 0)
					{
						//g_Log.Log("ranstarScale");
						*material->variables[i].GetFloatPtr() = 0.1f;
					}
					if (strcmp(material->shaderPack->variables.GetAt(i)->Name, "Bumpiness") == 0)
					{
						//g_Log.Log("ranstarScale");
						*material->variables[i].GetFloatPtr() = 100.0f;
					}
					if (strcmp(material->shaderPack->variables.GetAt(i)->Name, "LetterSize") == 0)
					{
						//g_Log.Log("ranstarScale");
						*material->variables[i].GetFloatPtr() = 0.2f;
					}
					//if (strcmp(material->shaderPack->variables.GetAt(i)->Name, "LetterIndex2") == 0)
					//{
					//	//g_Log.Log("ranstarScale");
					//	*material->variables[i].GetFloatPtr() = 35.0f;
					//}
					if (strcmp(material->shaderPack->variables.GetAt(i)->Name, "bDebugDisplayDamageMap") == 0)
					{
						//g_Log.Log("envEffScale");
						*material->variables[i].GetBoolPtr() = false;
					}
					if (strcmp(material->shaderPack->variables.GetAt(i)->Name, "DamageMultiplier") == 0)
					{
						//g_Log.Log("envEffScale");
						*material->variables[i].GetFloatPtr() = 2.0f;
					}
					if (strcmp(material->shaderPack->variables.GetAt(i)->Name, "envEffScale0") == 0)
					{
						//g_Log.Log("envEffScale");
						*material->variables[i].GetFloatPtr() = 0;
					}if (strcmp(material->shaderPack->variables.GetAt(i)->Name, "reflectivepower") == 0)
					{
						//g_Log.Log("envEffScale");
						*material->variables[i].GetFloatPtr() = 0;
					}if (strcmp(material->shaderPack->variables.GetAt(i)->Name, "enveffthickness") == 0)
					{
						//g_Log.Log("envEffScale");
						*material->variables[i].GetFloatPtr() = 0;
					}
					if (strcmp(material->shaderPack->variables.GetAt(i)->Name, "EmissiveMultiplier") == 0)
					{
						//g_Log.Log("envEffScale");
						*material->variables[i].GetFloatPtr() = 150;
					}

					if (strcmp(material->shaderPack->variables.GetAt(i)->Name, "Fresnel") == 0)
					{
						//g_Log.Log("Dirt");
						*material->variables[i].GetFloatPtr() = 5;
						intptr_t dirt = (intptr_t)material->variables[i].GetFloatPtr();

						// *(float*)(dirt + 0x0) = 1.0f;
						//*(float*)(dirt + 0x4) = 15.0f;
						//*(float*)(dirt + 0x8) = 15.0f;
					}
				}
			}

			// g_Log.Log("Adder Drawable: {:X}", (intptr_t)drawable);
			// g_Log.Log("DrawCustomShaderEffectCommand({:X})", renderData);
		}

	public:
		DrawCommands()
		{
			gm::ScanAndHook(
				"DrawCustomShaderEffectCommand",
				"48 89 5C 24 10 48 89 74 24 18 57 48 83 EC 30 F7",
				aImpl_DrawCustomShaderEffectCommand,
				&gImpl_DrawCustomShaderEffectCommand);
		}

		/*void AddOverrideShaderParamThisFrame(int entity, const char* shaderName, int materialId, const char* paramName)
		{

		}*/
	};

	typedef void(*RenderTask)();
	class RenderThread
	{
		typedef void(*gDef_PresentImage)();

		static inline gDef_PresentImage gImpl_PresentImage;

		static void aImpl_PresentImage()
		{
			for (auto& task : ms_renderTasks)
			{
				task();
			}
			gImpl_PresentImage();
		}

		static inline std::vector<RenderTask> ms_renderTasks;
	public:
		RenderThread()
		{
			gm::ScanAndHook("PresentImage", "40 55 53 56 57 41 54 41 56 41 57 48 8B EC 48 83 EC 40 48 8B 0D",
				aImpl_PresentImage,
				&gImpl_PresentImage);
		}

		template<typename T>
		static void AddRenderTask(T task)
		{
			ms_renderTasks.push_back(reinterpret_cast<RenderTask>(task));
		}
	};

	inline DrawCommands g_DrawCommands;
	inline fwRenderThreadInterface g_FwRenderThreadInterface;
	inline RenderThread g_RenderThread;
}
