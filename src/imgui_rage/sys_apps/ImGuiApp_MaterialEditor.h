#pragma once
#include "boost/bind.hpp"
#include "../ImGuiExtensions.h"
#include "../vendor/directxtk/include/DDSTextureLoader.h"

namespace sapp
{
	class ImGuiApp_MaterialEditor : public imgui_rage::ImGuiApp
	{
		static constexpr int INPUT_BUFFER_SIZE = 64;
		static constexpr int TEXT_BUFFER_SIZE = 256;

		static inline char ms_ModelNameInput[INPUT_BUFFER_SIZE] = "deluxo";
		static inline char m_TextBuffer[TEXT_BUFFER_SIZE]{};

		int m_EditMode = 0;
		int m_SelectedMaterialIndex = 0;
		int m_FragLodMode = 0;
		bool m_CacheOutdated = true;
		int m_SelectedTextureIndex = -1;

		rage::grcTexture* m_SelectedTexture = nullptr;

		uint32_t m_ModelHash;

		rage::gtaDrawable* m_Drawable;
		rage::grmShaderGroup* m_EditShaderGroup = nullptr;
		std::vector<std::string> m_ShaderPackNames{};

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

			if (hash != m_ModelHash)
				return;

			if ((*lpShaderGroup)->GetMaterialCount() != m_EditShaderGroup->GetMaterialCount())
				return;

			*lpShaderGroup = m_EditShaderGroup;
		}

		void OnCustomShaderEffectVehicleUse(uint32_t hash, bool& execute) const
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

			// So simply prevent game overwriting material values
			if (hash == m_ModelHash)
				execute = false;
		}

		void DrawMaterialTexturesForDrawable()
		{
			rage::grmShaderGroup* shaderGroup = m_Drawable->grmShaderGroup;
			rage::grcInstanceData* selectedMaterial = shaderGroup->GetMaterialAt(m_SelectedMaterialIndex);

			static float textureWidth = 256.0f;
			ImGui::SliderFloat("Size", &textureWidth, 64.0f, 512.0f);

			// TODO: Make automatic load from folder with model names

			ImGui::BeginDisabled(!m_SelectedTexture);
			if (ImGui::Button("Replace"))
			{

			}
			ImGui::EndDisabled();

			for (int i = 0; i < selectedMaterial->numVariables; i++)
			{
				rage::grcInstanceVar* materialVar = selectedMaterial->GetVariableAtIndex(i);
				rage::grcEffectVar* shaderVar = selectedMaterial->GetEffect()->variables[i];
				rage::eEffectValueType type = shaderVar->GetValueType();

				if (type != rage::EFFECT_VALUE_TEXTURE)
					continue;

				rage::grcTexture* texture = materialVar->GetTexture();

				if (texture == nullptr)
					continue;

				if (texture->GetName() == nullptr)
					continue;

				sprintf_s(m_TextBuffer, TEXT_BUFFER_SIZE, "rageAm/Textures/%s/%s.dds",
					ms_ModelNameInput, texture->GetName());

				HANDLE hnd = CreateFile(m_TextBuffer, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (hnd != INVALID_HANDLE_VALUE)
				{
					CloseHandle(hnd);

					ID3D11Device* pDevice = rh::grcDX11::GetDevice();

					ID3D11Texture2D* pOldTexture = texture->GetTexture();
					ID3D11Resource* pNewTexture;
					ID3D11ShaderResourceView* pOldResourceView = texture->GetShaderResourceView();
					ID3D11ShaderResourceView* pNewResourceView;

					int length = strlen(m_TextBuffer);
					std::wstring text_wchar(length, L'#');
					mbstowcs(&text_wchar[0], m_TextBuffer, length);

					HRESULT result = DirectX::CreateDDSTextureFromFile(
						pDevice, text_wchar.c_str(), &pNewTexture, &pNewResourceView);

					// TODO: Error message
					if (result == S_OK)
					{
						g_Log.LogD("Replacing texture: {}", texture->GetName());

						texture->SetTexture(pNewTexture);
						texture->SetShaderResourceView(pNewResourceView);

						// Release them because they are no longer tracked by game.
						// Instead game will release what we've inserted in texture.
						pOldTexture->Release();
						pOldResourceView->Release();
					}
				}

				if (ID3D11ShaderResourceView* shaderResourceView = texture->GetShaderResourceView())
				{
					float factor = static_cast<float>(texture->GetWidth()) / textureWidth;
					float height = static_cast<float>(texture->GetHeight()) / factor;

					sprintf_s(m_TextBuffer, INPUT_BUFFER_SIZE, "%s - %s", shaderVar->Name, texture->GetName());

					if (ImGui::Selectable(m_TextBuffer, i == m_SelectedTextureIndex))
						m_SelectedTextureIndex = i;

					if (i == m_SelectedTextureIndex)
						m_SelectedTexture = texture;

					ImGui::Image(shaderResourceView, ImVec2(textureWidth, height));
				}
			}
		}

		void DrawMaterialInfoForDrawable() const
		{
			// Material Info Table
			ImGui::BeginTable("MAT_INFO_TABLE", 3, APP_COMMON_TABLE_FLAGS);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			rage::grmShaderGroup* shaderGroup = m_Drawable->grmShaderGroup;
			rage::grcInstanceData* selectedMaterial = shaderGroup->GetMaterialAt(m_SelectedMaterialIndex);

			// Table Rows
			for (int i = 0; i < selectedMaterial->numVariables; i++)
			{
				ImGui::TableNextRow();

				rage::grcInstanceVar* materialVar = selectedMaterial->GetVariableAtIndex(i);
				rage::grcEffectVar* shaderVar = selectedMaterial->GetEffect()->variables[i];

				rage::eEffectValueType type = shaderVar->GetValueType();

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
				sprintf_s(m_TextBuffer, INPUT_BUFFER_SIZE, "##%s", shaderVar->Name);

				// TODO: Add matrix support
				switch (type)
				{
				case rage::EFFECT_VALUE_FLOAT:
					ImGui::InputFloat(m_TextBuffer, materialVar->GetFloatPtr()); break;
				case rage::EFFECT_VALUE_VECTOR2:
					ImGui::InputFloat2(m_TextBuffer, materialVar->GetFloatPtr()); break;
				case rage::EFFECT_VALUE_VECTOR3:
					ImGui::InputFloat3(m_TextBuffer, materialVar->GetFloatPtr()); break;
				case rage::EFFECT_VALUE_VECTOR4:
					ImGui::InputFloat4(m_TextBuffer, materialVar->GetFloatPtr()); break;
				case rage::EFFECT_VALUE_BOOL:
					ImGui::Checkbox(m_TextBuffer, materialVar->GetBoolPtr()); break;;
				case rage::EFFECT_VALUE_TEXTURE:
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
		}

		void DrawMaterialListForDrawable()
		{
			rage::grmShaderGroup* shaderGroup = m_Drawable->grmShaderGroup;

			ImGui::BeginChild("MaterialList", ImVec2(230, 0), true);

			ImGui::Text("Drawable: %p", reinterpret_cast<uintptr_t>(m_Drawable));
			ImGui::Text("grmShaderGroup: %p", reinterpret_cast<uintptr_t>(shaderGroup));

			if (m_CacheOutdated)
			{
				g_Log.LogD("ImGuiApp_MaterialEditor::DrawMaterialListForDrawable() -> Rebuilding materials name list");

				m_ShaderPackNames.clear();

				for (int i = 0; i < shaderGroup->GetMaterialCount(); i++)
				{
					rage::grcInstanceData* material = shaderGroup->GetMaterialAt(i);
					rage::grcEffect* effect = material->GetEffect();

					std::string shaderPackName = effect->GetFileName();
					m_ShaderPackNames.push_back(shaderPackName);
				}
			}

			ImGui::SetNextItemWidth(230);
			if (ImGui::ListBox("##MAT_LISTBOX", &m_SelectedMaterialIndex, m_ShaderPackNames))
			{
				// Reset texture index when material changes
				m_SelectedTextureIndex = -1;
				m_SelectedTexture = nullptr;
			}

			ImGui::EndChild();
		}

		void DrawShaderGroupForDrawable()
		{
			DrawMaterialListForDrawable();
			ImGui::SameLine();

			// Material Info Window
			ImGui::BeginChild("MAT_INFO", ImVec2(0, 0), true);
			ImGui::Text("Material Information");

			ImGui::BeginTabBar("MAT_INFO_TAB_BAR");
			if (ImGui::BeginTabItem("Table"))
			{
				DrawMaterialInfoForDrawable();
				ImGui::EndTabItem(); // Table
			}
			if (ImGui::BeginTabItem("Textures"))
			{
				DrawMaterialTexturesForDrawable();
				ImGui::EndTabItem(); // Textures
			}
			ImGui::EndTabBar(); // MAT_INFO_TAB_BAR

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

			m_Drawable = drawable;
			DrawShaderGroupForDrawable();
			m_EditShaderGroup = drawable->grmShaderGroup;
		}

		/**
		 * \brief Adds or removes '_hi' postfix in model name.
		 * \param highDetail Whether to add or remove the postfix.
		 */
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

			// TODO: It still may have issues with _hi
			if (ImGui::InputText("Name", ms_ModelNameInput, IM_ARRAYSIZE(ms_ModelNameInput)))
				m_CacheOutdated = true;

			if (m_CacheOutdated)
				m_ModelHash = fwHelpers::joaat(ms_ModelNameInput);

			FindDrawableAndDrawShaderGroup();

			m_CacheOutdated = false;

			ImGui::End(); // Material Editor
		}
	};
}
