#pragma once
#include "boost/bind.hpp"

namespace sapp
{
	class ImGuiApp_MaterialEditor : public imgui_rage::ImGuiApp
	{
		int m_EditMode = 0;
		int m_SelectedMaterialIndex = 0;
		char m_ModelNameInput[64] = "deluxo";
		int m_FragLodMode = 0;

		rage::grmShaderGroup* m_EditShaderGroup = nullptr;

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

			if (hash != fwHelpers::jooat(m_ModelNameInput))
				return;

			if ((*lpShaderGroup)->GetMaterialCount() != m_EditShaderGroup->GetMaterialCount())
				return;

			*lpShaderGroup = m_EditShaderGroup;
		}

		void OnCustomShaderEffectVehicleUse(uint32_t hash, bool& execute) const
		{
			// Prevent game overwriting material values
			if (hash == fwHelpers::jooat(m_ModelNameInput))
				execute = false;
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

			static const char* editorModes[] = { "Model", "Instance" };
			ImGui::Combo("Mode##MAT_MODE", &m_EditMode, editorModes, IM_ARRAYSIZE(editorModes));

			ImGui::InputText("Name", m_ModelNameInput, IM_ARRAYSIZE(m_ModelNameInput));

			ImGui::Text("Model Type:");
			ImGui::SameLine();
			// For now only fragments
			if (g_FragmentStore->Exists(m_ModelNameInput))
			{
				ImGui::Text("Fragment");

				static const char* modelMode[] = { "High Detail (_hi)", "Standard" };
				ImGui::Combo("LOD##FRAG_LOD", &m_FragLodMode, modelMode, IM_ARRAYSIZE(modelMode));

				char* pHi = strstr(m_ModelNameInput, "_hi");
				if (m_FragLodMode == 0 && pHi == nullptr) // _hi
				{
					int len = strlen(m_ModelNameInput);
					m_ModelNameInput[len + 0] = '_';
					m_ModelNameInput[len + 1] = 'h';
					m_ModelNameInput[len + 2] = 'i';
					m_ModelNameInput[len + 3] = '\0';
				}
				else if(m_FragLodMode == 1 && pHi != nullptr)
				{
					*strstr(m_ModelNameInput, "_hi") = '\0';
				}

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
				// values were set, we can override any value we want.

				auto slot = g_FragmentStore->FindSlot(m_ModelNameInput);
				auto fragType = slot->value;

				// TODO: Find better way
				pHi = strstr(m_ModelNameInput, "_hi");
				if (pHi)
					*pHi = '\0';

				// If model is not streamed fragType gonna be null
				if (fragType != nullptr)
				{
					ImGui::BeginChild("MaterialList", ImVec2(230, 0), true);

					// TODO: Proper structure def
					rage::gtaDrawable* drawable = *reinterpret_cast<rage::gtaDrawable**>(fragType + 0x30);
					rage::grmShaderGroup* shaderGroup = drawable->grmShaderGroup;

					ImGui::Text("Drawable: %p", reinterpret_cast<uintptr_t>(drawable));
					ImGui::Text("grmShaderGroup:%p", reinterpret_cast<uintptr_t>(shaderGroup));

					int numMats = shaderGroup->GetMaterialCount();

					std::vector<std::string> shaderPackNames;
					for (int i = 0; i < numMats; i++)
					{
						rage::grmMaterial* material = shaderGroup->GetMaterialAt(i);
						rage::grmShaderPack* shaderPack = material->GetShaderPack();

						std::string shaderPackName = shaderPack->GetFileName();
						shaderPackNames.push_back(shaderPackName);
					}

					ImGui::SetNextItemWidth(230);
					// TODO: Move this to extension method
					ImGui::ListBox("##Materials", &m_SelectedMaterialIndex,
						[](void* vec, int idx, const char** out_text) {
							std::vector<std::string>* vector = static_cast<std::vector<std::string>*>(vec);
							if (idx < 0 || idx >= vector->size())return false;
							*out_text = vector->at(idx).c_str();
							return true;
						}, &shaderPackNames, shaderPackNames.size(), shaderPackNames.size());

					ImGui::EndChild(); /* Material List */

					ImGui::SameLine();

					ImGui::BeginChild("MaterialInfo", ImVec2(0, 0), true);

					ImGui::Text("Material Information");

					rage::grmMaterial* selectedMaterial = shaderGroup->GetMaterialAt(m_SelectedMaterialIndex);

					ImGui::BeginTable("Material Information Table", 3, APP_COMMON_TABLE_FLAGS);
					ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
					ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
					ImGui::TableSetupColumn("Value");
					ImGui::TableHeadersRow();

					for (int i = 0; i < selectedMaterial->numVariables; i++)
					{
						ImGui::TableNextRow();

						rage::grmMaterialVariable* materialVar = selectedMaterial->GetVariableAtIndex(i);
						rage::grmShaderVariable* shaderVar = selectedMaterial->shaderPack->variables[i];

						rage::eGrmValueType type = shaderVar->GetValueType();

						ImGui::TableSetColumnIndex(0);
						ImGui::Text(shaderVar->Name);
						ImGui::TableSetColumnIndex(1);
						ImGui::Text("%i", static_cast<int>(type));

						// Control must have unique ID
						std::string valueId = std::format("##{}-{}", shaderVar->Name, i);
						const char* valueIdStr = valueId.c_str();

						ImGui::TableSetColumnIndex(2);
						switch (type)
						{
						case rage::GRM_VALUE_FLOAT:
							ImGui::InputFloat(valueIdStr, materialVar->GetFloatPtr()); break;
						case rage::GRM_VALUE_VECTOR2:
							ImGui::InputFloat2(valueIdStr, materialVar->GetFloatPtr()); break;
						case rage::GRM_VALUE_VECTOR3:
							ImGui::InputFloat3(valueIdStr, materialVar->GetFloatPtr()); break;
						case rage::GRM_VALUE_VECTOR4:
							ImGui::InputFloat4(valueIdStr, materialVar->GetFloatPtr()); break;
						case rage::GRM_VALUE_BOOL:
							ImGui::Checkbox(valueIdStr, materialVar->GetBoolPtr()); break;;
						case rage::GRM_VALUE_MATRIX:
						{
							// This is terrible.

							std::string valueId_row1 = std::format("##{}-{}-1", shaderVar->Name, i);
							std::string valueId_row2 = std::format("##{}-{}-2", shaderVar->Name, i);
							std::string valueId_row3 = std::format("##{}-{}-3", shaderVar->Name, i);
							std::string valueId_row4 = std::format("##{}-{}-4", shaderVar->Name, i);
							const char* valueIdStr_row1 = valueId_row1.c_str();
							const char* valueIdStr_row2 = valueId_row2.c_str();
							const char* valueIdStr_row3 = valueId_row3.c_str();
							const char* valueIdStr_row4 = valueId_row4.c_str();

							ImGui::InputFloat4(valueIdStr_row1, materialVar->GetFloatPtr() + 0x0);
							ImGui::InputFloat4(valueIdStr_row2, materialVar->GetFloatPtr() + 0x10);
							ImGui::InputFloat4(valueIdStr_row3, materialVar->GetFloatPtr() + 0x20);
							ImGui::InputFloat4(valueIdStr_row4, materialVar->GetFloatPtr() + 0x30);
							break;
						}
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

					ImGui::EndTable();

					ImGui::EndChild(); /* Material Info */

					// Assign after editing of all values is done
					m_EditShaderGroup = shaderGroup;
				}
				else
				{
					ImGui::Text("Model is not streamed.");
					m_EditShaderGroup = nullptr;
				}
			}
			else
			{
				ImGui::Text("Not Found");
				m_EditShaderGroup = nullptr;
			}

			ImGui::End();
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
				}

				ImGui::EndMenuBar();
			}

			m_MatEditor.Render();

			ImGui::End();
		}
	};
}
