#pragma once
#include "boost/bind.hpp"
#include "../ImGuiExtensions.h"

namespace sapp
{
	class ImGuiApp_MaterialEditor : public imgui_rage::ImGuiApp
	{
		static constexpr int INPUT_BUFFER_SIZE = 64;
		static inline char ms_ModelNameInput[INPUT_BUFFER_SIZE] = "deluxo";

		int m_EditMode = 0;
		int m_SelectedMaterialIndex = 0;
		int m_FragLodMode = 0;
		bool m_CacheOutdated = true;

		rage::grmShaderGroup* m_EditShaderGroup = nullptr;

		std::vector<std::string> m_ShaderPackNames;

		void OnFragmentRender(uint32_t hash, rage::grmShaderGroup** lpShaderGroup) const
		{
			if (m_EditShaderGroup == nullptr)
				return;

			// No idea why game may pass null grmShaderGroup but this happens
			if (lpShaderGroup == nullptr)
				return;

			// Vehicle have extra '_hi' model as lod0, this gives us 2 grmShaderGroup's in total
			// (that's why there's two CCustomShaderEffectVehicle's in CVehicleDrawHandler, primary and lod one)
			// usually (at least with game models) lod grmShaderGroup have less materials, for i.e.
			// adder_hi has 18 materials when simply adder has only 14.
			// We can use that to determine what lod type is given. Setting grmShaderGroup
			// to one with exactly the same amount of materials won't hurt even if our assert failed.
			// It won't crash game though materials will be assigned wrong.

			if (hash != fwHelpers::joaat(ms_ModelNameInput))
				return;

			if ((*lpShaderGroup)->GetMaterialCount() != m_EditShaderGroup->GetMaterialCount())
				return;

			*lpShaderGroup = m_EditShaderGroup;
		}

		void OnCustomShaderEffectVehicleUse(uint32_t hash, bool& execute) const
		{
			// Prevent game overwriting material values
			if (hash == fwHelpers::joaat(ms_ModelNameInput))
				execute = false;
		}

		void DrawMaterialListForDrawable(rage::gtaDrawable* drawable)
		{
			rage::grmShaderGroup* shaderGroup = drawable->grmShaderGroup;

			ImGui::BeginChild("MaterialList", ImVec2(230, 0), true);

			ImGui::Text("Drawable: %p", reinterpret_cast<uintptr_t>(drawable));
			ImGui::Text("grmShaderGroup: %p", reinterpret_cast<uintptr_t>(shaderGroup));

			if (m_CacheOutdated)
			{
				g_Log.LogD("ImGuiApp_MaterialEditor::DrawMaterialListForDrawable() -> Rebuilding materials name list");

				m_ShaderPackNames.clear();

				for (int i = 0; i < shaderGroup->GetMaterialCount(); i++)
				{
					rage::grmMaterial* material = shaderGroup->GetMaterialAt(i);
					rage::grmShaderPack* shaderPack = material->GetShaderPack();

					std::string shaderPackName = shaderPack->GetFileName();
					m_ShaderPackNames.push_back(shaderPackName);
				}
			}

			ImGui::SetNextItemWidth(230);
			ImGui::ListBox("##MAT_LISTBOX", &m_SelectedMaterialIndex, m_ShaderPackNames);

			ImGui::EndChild();
		}

		void DrawShaderGroupForDrawable(rage::gtaDrawable* drawable)
		{
			DrawMaterialListForDrawable(drawable);

			ImGui::SameLine();

			// Material Info Window
			ImGui::BeginChild("MAT_INFO", ImVec2(0, 0), true);
			ImGui::Text("Material Information");

			// Material Info Table
			ImGui::BeginTable("MAT_INFO_TABLE", 3, APP_COMMON_TABLE_FLAGS);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			rage::grmShaderGroup* shaderGroup = drawable->grmShaderGroup;
			rage::grmMaterial* selectedMaterial = shaderGroup->GetMaterialAt(m_SelectedMaterialIndex);

			// Table Rows
			for (int i = 0; i < selectedMaterial->numVariables; i++)
			{
				ImGui::TableNextRow();

				rage::grmMaterialVariable* materialVar = selectedMaterial->GetVariableAtIndex(i);
				rage::grmShaderVariable* shaderVar = selectedMaterial->shaderPack->variables[i];

				rage::eGrmValueType type = shaderVar->GetValueType();

				// Name
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(shaderVar->Name);

				// Type
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%i", static_cast<int>(type));

				// Value
				ImGui::TableSetColumnIndex(2);

				// Controls must have unique ID so use variable name
				// since it doesn't appear twice on screen
				const char* valueLabel = shaderVar->Name;

				// TODO: Add matrix support
				switch (type)
				{
				case rage::GRM_VALUE_FLOAT:
					ImGui::InputFloat(valueLabel, materialVar->GetFloatPtr()); break;
				case rage::GRM_VALUE_VECTOR2:
					ImGui::InputFloat2(valueLabel, materialVar->GetFloatPtr()); break;
				case rage::GRM_VALUE_VECTOR3:
					ImGui::InputFloat3(valueLabel, materialVar->GetFloatPtr()); break;
				case rage::GRM_VALUE_VECTOR4:
					ImGui::InputFloat4(valueLabel, materialVar->GetFloatPtr()); break;
				case rage::GRM_VALUE_BOOL:
					ImGui::Checkbox(valueLabel, materialVar->GetBoolPtr()); break;;
				case rage::GRM_VALUE_TEXTURE:
				{
					rage::grcTexture* texture = materialVar->GetTexture();
					ImGui::Text(texture != nullptr ? texture->GetName() : "-");
					break;
				}
				default:
					ImGui::Text("NOT SUPPORTED"); break;
				}
			}
			ImGui::EndTable(); // MAT_INFO_TABLE
			ImGui::EndChild(); // MAT_INFO
		}

		void FindDrawableAndDrawShaderGroup()
		{
			ImGui::Text("Model Type:");
			ImGui::SameLine();

			rage::gtaDrawable* drawable;

			// TODO: Other drawable types
			if ((drawable = TryGetFragmentDrawable()))
				ImGui::Text("Fragment");

			if (drawable == nullptr)
			{
				ImGui::Text("Not Found / Not Streamed");
				m_EditShaderGroup = nullptr;
				return;
			}

			// If model name was already specified, but model wasn't streamed.
			// Now it got streamed and we have to rebuild the cache
			if (m_EditShaderGroup == nullptr)
			{
				g_Log.LogD("ImGuiApp_MaterialEditor::FindDrawableAndDrawShaderGroup() -> Streaming started for drawable, invalidating cache");

				m_CacheOutdated = true;
			}

			DrawShaderGroupForDrawable(drawable);
			m_EditShaderGroup = drawable->grmShaderGroup;
		}

		void ConvertModelNameTo(bool highDetail) const
		{
			int len = strlen(ms_ModelNameInput);

			char* hiPtr = strstr(ms_ModelNameInput, "_hi");
			if (highDetail && hiPtr == nullptr) // _hi
			{
				ms_ModelNameInput[len + 0] = '_';
				ms_ModelNameInput[len + 1] = 'h';
				ms_ModelNameInput[len + 2] = 'i';
				ms_ModelNameInput[len + 3] = '\0';
			}
			else if (m_FragLodMode == 1 && hiPtr != nullptr)
			{
				*strstr(ms_ModelNameInput, "_hi") = '\0';
			}
		}

		rage::gtaDrawable* TryGetFragmentDrawable()
		{
			// ISSUE: Fragments rendering is a little bit complicated,
			// as usual there's grmShaderGroup but it's not one per model,
			// it's per-lod. So simply editing grmShaderGroup will lead to nothing.
			// Further more, vehicles and other entities with 'specific' shaders
			// such as tint or paint have a thing called 'CCustomShaderEffect' (later - shaderFx)
			// which basically provides an interface to edit shader parameters,
			// and just before when constant buffers are being updated with new values
			// from grmShaderGroup->Materials (later materials), shaderFx sets it's own
			// values in materials. It's another reason why we can't
			// simply alter grmShaderGroup without disabling shaderFx,
			// but it will completely break rendering pipeline. For i.e. cars
			// will appear with green mud, DamageRT (in GTA damage
			// to cars applied via displacement texture and tessellation) won't be applied whatsoever,
			// license plates will appear bugged.
			// SOLUTION: So what we do, is hook render command method that invoked just right after shaderFx
			// values were set, compare if model hash matches ours, and if so, cancel execution.

			static const char* modelMode[] = { "High Detail (_hi)", "Standard" };
			if (ImGui::Combo("LOD##FRAG_LOD", &m_FragLodMode, modelMode, IM_ARRAYSIZE(modelMode)))
				m_CacheOutdated = true;

			// If High Detail mode chosen, append '_hi' to model name, other wise remove it
			if (m_CacheOutdated)
				ConvertModelNameTo(m_FragLodMode == 0);

			auto slot = g_FragmentStore->FindSlot(ms_ModelNameInput);

			if (!slot) return nullptr;

			auto fragType = slot->value;

			// If model is not streamed fragType gonna be null
			if (fragType != nullptr)
			{
				// TODO: Proper structure def
				return *reinterpret_cast<rage::gtaDrawable**>(fragType + 0x30);
			}
			return nullptr;
		}

	public:
		ImGuiApp_MaterialEditor()
		{
			rh::Rendering::OnFragRender.connect(
				boost::bind(&ImGuiApp_MaterialEditor::OnFragmentRender, this, _1, _2));
			rh::Rendering::OnCustomShaderEffectVehicleUse.connect(
				boost::bind(&ImGuiApp_MaterialEditor::OnCustomShaderEffectVehicleUse, this, _1, _2));
		}

	protected:
		void OnRender() override
		{
			ImGui::Begin("Material Editor", &IsVisible);

			// TODO: Support these
			static const char* editorModes[] = { "Model", "Instance" };
			ImGui::Combo("Mode##MAT_MODE", &m_EditMode, editorModes, IM_ARRAYSIZE(editorModes));

			if (ImGui::InputText("Name", ms_ModelNameInput, IM_ARRAYSIZE(ms_ModelNameInput)))
				m_CacheOutdated = true;

			FindDrawableAndDrawShaderGroup();

			m_CacheOutdated = false;

			ImGui::End(); // Material Editor
		}
	};

	class ImGuiApp_Toolbar : public imgui_rage::ImGuiApp
	{
		ImGuiApp_MaterialEditor m_MatEditor;
	public:
		ImGuiApp_Toolbar()
		{
			IsVisible = true;
			m_MatEditor.IsVisible = true;
		}

		void OnRender() override
		{
			ImGui::Begin("rageAm Toolbar", nullptr, ImGuiWindowFlags_MenuBar);

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("View"))
				{
					ImGui::MenuItem("Material Editor", nullptr, &m_MatEditor.IsVisible);
					ImGui::EndMenu();
				} // BeginMenu
				ImGui::EndMenuBar();
			} // BeginMenuBar

			m_MatEditor.Render();

			ImGui::End();
		}
	};
}
