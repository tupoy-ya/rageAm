#pragma once
#include "boost/bind.hpp"
#include "../ImGuiExtensions.h"
#include "../vendor/directxtk/include/DDSTextureLoader.h"
#include "../vendor/tinyxml2/tinyxml2.h"

namespace sapp
{
	enum eShaderUIWidget
	{
		UI_WIDGET_NONE = 0,
		UI_WIDGET_COLOR_RGB,
		UI_WIDGET_SLIDER_FLOAT,
		UI_WIDGET_SLIDER_FLOAT2,
		UI_WIDGET_SLIDER_FLOAT3,
		UI_WIDGET_SLIDER_FLOAT4,
		UI_WIDGET_TOGGLE_FLOAT,
	};

	struct ShaderUIVariable
	{
		const char* Name;
		const char* Description;
		eShaderUIWidget Widget;
		union
		{
			struct
			{
				float Min;
				float Max;
				ImGuiSliderFlags Flags;
			} Slider;

			struct
			{
				float Disabled;
				float Enabled;
			} Toggle;
		};
	};

	class ShaderUIConfig
	{
		static inline tinyxml2::XMLDocument sm_Doc;
		static inline std::unordered_map<std::string_view, ShaderUIVariable> sm_Variables;

		static bool ParseSlider(ShaderUIVariable* uiVar, tinyxml2::XMLElement* widget, const char* widgetName)
		{
			auto min = widget->FirstChildElement("Min");
			auto max = widget->FirstChildElement("Max");
			if (!min || !max)
			{
				AM_ERRF("ShaderUIConfig::Parse() -> Widget of type: {} requires 'Min' and 'Max' parameters.", widgetName);
				return false;
			}

			uiVar->Slider.Min = min->FloatText();
			uiVar->Slider.Max = max->FloatText();

			if (widget->FirstChildElement("Logarithmic"))
				uiVar->Slider.Flags |= ImGuiSliderFlags_Logarithmic;

			return true;
		}
		static bool ParseWidget(ShaderUIVariable* uiVar, tinyxml2::XMLElement* widget)
		{
			const char* widgetName = nullptr;

			widget->QueryStringAttribute("Name", &widgetName);
			if (!widgetName)
			{
				AM_ERR("ShaderUIConfig::Parse() -> Widget must have name attribute.");
				return false;
			}

			// Color
			if (strcmp(widgetName, "ColorRGB") == 0)
			{
				uiVar->Widget = UI_WIDGET_COLOR_RGB;
			}
			// Slider Float
			else if (strcmp(widgetName, "SliderFloat") == 0)
			{
				uiVar->Widget = UI_WIDGET_SLIDER_FLOAT;
				ParseSlider(uiVar, widget, widgetName);
			}
			else if (strcmp(widgetName, "SliderFloat2") == 0)
			{
				uiVar->Widget = UI_WIDGET_SLIDER_FLOAT2;
				ParseSlider(uiVar, widget, widgetName);
			}
			else if (strcmp(widgetName, "SliderFloat3") == 0)
			{
				uiVar->Widget = UI_WIDGET_SLIDER_FLOAT3;
				ParseSlider(uiVar, widget, widgetName);
			}
			else if (strcmp(widgetName, "SliderFloat4") == 0)
			{
				uiVar->Widget = UI_WIDGET_SLIDER_FLOAT4;
				ParseSlider(uiVar, widget, widgetName);
			}
			// Toggle Float
			else if (strcmp(widgetName, "ToggleFloat") == 0)
			{
				uiVar->Widget = UI_WIDGET_TOGGLE_FLOAT;

				auto disabled = widget->FirstChildElement("Disabled");
				auto enabled = widget->FirstChildElement("Enabled");
				if (!disabled || !enabled)
				{
					AM_ERRF("ShaderUIConfig::Parse() -> Widget of type: {} requires 'Disabled' and 'Enabled' parameters.", widgetName);
					return false;
				}

				uiVar->Toggle.Disabled = disabled->FloatText();
				uiVar->Toggle.Enabled = enabled->FloatText();
			}
			// Unknown
			else
			{
				AM_ERRF("ShaderUIConfig::Parse() -> Widget of type: {} is not supported.", widgetName);
				return false;
			}
			return true;
		}
	public:
		static void Parse()
		{
			sm_Variables.clear();
			sm_Doc.Clear();

			tinyxml2::XMLError result = sm_Doc.LoadFile("rageAm/ShaderUIConfig.xml");

			if (result != tinyxml2::XML_SUCCESS)
			{
				AM_ERR("ShaderUIConfig::Parse() -> Unable to parse config.");
				return;
			}

			auto config = sm_Doc.FirstChildElement("Config");

			// Parse variable list
			auto variable = config->FirstChildElement("Variables")->FirstChildElement("Item");
			while (variable != nullptr)
			{
				const char* shaderVarName;
				variable->QueryStringAttribute("Name", &shaderVarName);

				auto name = variable->FirstChildElement("Name");
				auto desc = variable->FirstChildElement("Description");
				auto widget = variable->FirstChildElement("Widget");

				ShaderUIVariable uiVar{};

				if (!name)
				{
					AM_ERRF("ShaderUIConfig::Parse() -> Name is required property of variable! {}", shaderVarName);
					return;
				}

				uiVar.Name = name->GetText();
				if (desc)
					uiVar.Description = desc->GetText();

				if (widget && !ParseWidget(&uiVar, widget))
					return;

				sm_Variables[shaderVarName] = uiVar;
				variable = variable->NextSiblingElement("Item");
			}
		}

		static ShaderUIVariable* GetVariableFor(const char* name)
		{
			auto item = sm_Variables.find(name);
			if (item == sm_Variables.end())
				return nullptr;

			return &item->second;
		}
	};

	enum eModelType
	{
		MODEL_UNKNOWN,
		MODEL_DRAWABLE,
		MODEL_FRAGMENT,
	};

	class ImGuiApp_MaterialEditor : public imgui_rage::ImGuiApp
	{
		static constexpr int INPUT_BUFFER_SIZE = 64;
		static constexpr int TEXT_BUFFER_SIZE = 256;

		static inline char sm_ModelNameInput[INPUT_BUFFER_SIZE] = "jackal";
		static inline char sm_TextBuffer[TEXT_BUFFER_SIZE]{};

		int m_EditMode = 0;
		int m_FragLodMode = 0;
		int m_SelectedMaterialIndex = 0;
		int m_SelectedTextureIndex = -1;
		bool m_EnableUIConfig = true;

		uint32_t m_LodModelHash = 0; // For fragments
		uint32_t m_ModelHash = 0;
		eModelType m_ModelType = MODEL_UNKNOWN;
		rage::gtaDrawable* m_Drawable = nullptr;
		rage::grcTexture* m_SelectedTexture = nullptr;
		rage::grmShaderGroup* m_EditShaderGroup = nullptr;

		void OnFragmentRender(uint32_t hash, rage::grmShaderGroup** lpShaderGroup) const
		{
			if (m_ModelType != MODEL_FRAGMENT)
				return;

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

			if (hash != m_LodModelHash)
				return;

			if ((*lpShaderGroup)->GetEffectCount() != m_EditShaderGroup->GetEffectCount())
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
			if (hash == m_LodModelHash)
				execute = false;
		}

		void DrawMaterialTexturesForDrawable()
		{
			rage::grmShaderGroup* shaderGroup = m_Drawable->grmShaderGroup;
			rage::grcInstanceData* selectedMaterial = shaderGroup->GetInstanceDataAt(m_SelectedMaterialIndex);

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

				sprintf_s(sm_TextBuffer, TEXT_BUFFER_SIZE, "rageAm/Textures/%s/%s.dds",
					sm_ModelNameInput, texture->GetName());

				HANDLE hnd = CreateFile(sm_TextBuffer, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (hnd != INVALID_HANDLE_VALUE)
				{
					CloseHandle(hnd);

					ID3D11Device* pDevice = rh::D3D::GetDevice();

					ID3D11Texture2D* pOldTexture = texture->GetTexture();
					ID3D11Resource* pNewTexture;
					ID3D11ShaderResourceView* pOldResourceView = texture->GetShaderResourceView();
					ID3D11ShaderResourceView* pNewResourceView;

					int length = strlen(sm_TextBuffer);
					std::wstring text_wchar(length, L'#');
					mbstowcs(text_wchar.data(), sm_TextBuffer, length);

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

				//if (ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
				//{
				//	ImGui::TableSetColumnIndex(0);
				//	//     ImGui::AlignTextToFramePadding();
				//	ImGui::Text("Name");
				//	ImGui::TableSetColumnIndex(1);
				//	ImGui::Text(texture->GetName());

				//	ImGui::EndTable();
				//}

				if (ID3D11ShaderResourceView* shaderResourceView = texture->GetShaderResourceView())
				{
					constexpr float textureWidth = 64.0f;

					float factor = static_cast<float>(texture->GetWidth()) / textureWidth;
					float height = static_cast<float>(texture->GetHeight()) / factor;

					sprintf_s(sm_TextBuffer, INPUT_BUFFER_SIZE, "%s - %s", shaderVar->GetDisplayName(), texture->GetName());

					if (ImGui::Selectable(sm_TextBuffer, i == m_SelectedTextureIndex))
						m_SelectedTextureIndex = i;

					if (i == m_SelectedTextureIndex)
						m_SelectedTexture = texture;

					ImGui::Image(shaderResourceView, ImVec2(textureWidth, height));
				}
			}
		}

		void DrawDefaultVariableWidget(const rage::grcInstanceVar* instVar, rage::eEffectValueType type) const
		{
			float speed = 0.05f;

			// TODO: Add matrix support
			switch (type)
			{
			case rage::EFFECT_VALUE_FLOAT:
				ImGui::DragFloat(sm_TextBuffer, instVar->GetFloatPtr(), speed); break;
			case rage::EFFECT_VALUE_VECTOR2:
				ImGui::DragFloat2(sm_TextBuffer, instVar->GetFloatPtr(), speed); break;
			case rage::EFFECT_VALUE_VECTOR3:
				ImGui::DragFloat3(sm_TextBuffer, instVar->GetFloatPtr(), speed); break;
			case rage::EFFECT_VALUE_VECTOR4:
				ImGui::DragFloat4(sm_TextBuffer, instVar->GetFloatPtr(), speed); break;
			case rage::EFFECT_VALUE_BOOL:
				ImGui::Checkbox(sm_TextBuffer, instVar->GetBoolPtr()); break;;
			case rage::EFFECT_VALUE_TEXTURE:
			{
				rage::grcTexture* texture = instVar->GetTexture();
				ImGui::Text(texture != nullptr ? texture->GetName() : "-");
				break;
			}
			default:
				ImGui::Text("NOT SUPPORTED"); break;
			}
		}

		void DrawMaterialInfoForDrawable() const
		{
			// Material Info Table
			ImGui::BeginTable("MAT_INFO_TABLE", 3, APP_COMMON_TABLE_FLAGS | ImGuiTableFlags_Hideable);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			rage::grmShaderGroup* shaderGroup = m_Drawable->grmShaderGroup;
			rage::grcInstanceData* selectedMaterial = shaderGroup->GetInstanceDataAt(m_SelectedMaterialIndex);

			// Table Rows
			for (int i = 0; i < selectedMaterial->numVariables; i++)
			{
				ImGui::TableNextRow();

				const rage::grcEffect* effect = selectedMaterial->GetEffect();
				const rage::grcInstanceVar* instVar = selectedMaterial->GetVariableAtIndex(i);
				const rage::grcEffectVar* shaderVar = effect->variables[i];
				const ShaderUIVariable* uiVar = ShaderUIConfig::GetVariableFor(shaderVar->InShaderName);
				rage::eEffectValueType type = shaderVar->GetValueType();

				// Name
				ImGui::TableSetColumnIndex(0);
				const char* displayName = shaderVar->GetDisplayName();
				if (m_EnableUIConfig && uiVar)
				{
					displayName = uiVar->Name;
					if (uiVar->Description)
					{
						ImGui::HelpMarker(uiVar->Description);
						ImGui::SameLine();
					}
				}
				ImGui::Text(displayName);

				// Type
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", EffectValueTypeToString(type));

				// Value
				ImGui::TableSetColumnIndex(2);
				ImGui::SetNextItemWidth(-FLT_MIN); // Stretch editors to whole area

				// Controls must have unique ID so use variable name
				// since it doesn't appear twice on screen
				sprintf_s(sm_TextBuffer, INPUT_BUFFER_SIZE, "##%s", shaderVar->InShaderName);

				if (!m_EnableUIConfig || !uiVar || uiVar->Widget == UI_WIDGET_NONE)
				{
					DrawDefaultVariableWidget(instVar, type);
					continue;
				}

				const char* slFmt = "%.3f";
				ImGuiSliderFlags slFlags = uiVar->Slider.Flags;
				// ReSharper disable once CppIncompleteSwitchStatement
				switch (uiVar->Widget)
				{
				case UI_WIDGET_COLOR_RGB:
					ImGui::ColorEdit3(sm_TextBuffer, instVar->GetFloatPtr());
					break;
				case UI_WIDGET_SLIDER_FLOAT:
					ImGui::SliderFloat(sm_TextBuffer, instVar->GetFloatPtr(), uiVar->Slider.Min, uiVar->Slider.Max, slFmt, slFlags);
					break;
				case UI_WIDGET_SLIDER_FLOAT2:
					ImGui::SliderFloat2(sm_TextBuffer, instVar->GetFloatPtr(), uiVar->Slider.Min, uiVar->Slider.Max, slFmt, slFlags);
					break;
				case UI_WIDGET_SLIDER_FLOAT3:
					ImGui::SliderFloat3(sm_TextBuffer, instVar->GetFloatPtr(), uiVar->Slider.Min, uiVar->Slider.Max, slFmt, slFlags);
					break;
				case UI_WIDGET_SLIDER_FLOAT4:
					ImGui::SliderFloat4(sm_TextBuffer, instVar->GetFloatPtr(), uiVar->Slider.Min, uiVar->Slider.Max, slFmt, slFlags);
					break;
				case UI_WIDGET_TOGGLE_FLOAT:
					bool on = instVar->GetFloat() == uiVar->Toggle.Enabled;
					if (ImGui::Checkbox(sm_TextBuffer, &on))
						*instVar->GetFloatPtr() = on ? uiVar->Toggle.Enabled : uiVar->Toggle.Disabled;
					break;
				}
			}
			ImGui::EndTable(); // MAT_INFO_TABLE
		}

		static const char* GetMaterialNameAt(rage::grmShaderGroup* group, int index)
		{
			return group->GetInstanceDataAt(index)->GetEffect()->GetFileName();
		}

		void DrawMaterialListForDrawable()
		{
			rage::grmShaderGroup* shaderGroup = m_Drawable->grmShaderGroup;

			ImGui::BeginChild("MaterialList", ImVec2(230, 0), true);

			//ImGui::Text("Drawable: %p", reinterpret_cast<uintptr_t>(m_Drawable));
			//ImGui::Text("grmShaderGroup: %p", reinterpret_cast<uintptr_t>(shaderGroup));
			ImGui::Text("Materials (%i)", shaderGroup->GetEffectCount());

			ImGui::SetNextItemWidth(230);
			if (ImGui::ListBox<rage::grmShaderGroup*>("##MAT_LISTBOX", &m_SelectedMaterialIndex,
				shaderGroup, shaderGroup->GetEffectCount(), GetMaterialNameAt))
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

			rage::grmShaderGroup* shaderGroup = m_Drawable->grmShaderGroup;
			rage::grcInstanceData* selectedMaterial = shaderGroup->GetInstanceDataAt(m_SelectedMaterialIndex);

			// Material Info Window
			ImGui::BeginChild("MAT_INFO", ImVec2(0, 0), true);
			ImGui::Text("Material: %s", selectedMaterial->GetEffect()->GetFileName());
			ImGui::Text("grcEffect: %p", (uint64_t)selectedMaterial->GetEffect());

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

			// TODO: Cache it
			rage::gtaDrawable* drawable;

			// TODO: Other drawable types
			if ((drawable = TryGetFragment()))
			{
				ImGui::Text("Fragment");
				m_ModelType = MODEL_FRAGMENT;
			}
			else if ((drawable = TryGetDrawable()))
			{
				ImGui::Text("Drawable");
				m_ModelType = MODEL_DRAWABLE;
			}
			else
			{
				m_ModelType = MODEL_UNKNOWN;
			}

			if (drawable == nullptr)
			{
				ImGui::Text("Not Found / Not Streamed");
				m_EditShaderGroup = nullptr;
				return;
			}

			ImGui::Text("grmShaderGroup: %p", (uintptr_t)drawable->grmShaderGroup);

			m_Drawable = drawable;
			DrawShaderGroupForDrawable();
			m_EditShaderGroup = drawable->grmShaderGroup;
		}

		/**
		 * \brief Adds or removes '_hi' postfix in model name.
		 * \param highDetail Whether to add or remove the postfix.
		 */
		void ConvertModelNameTo(bool highDetail)
		{
			static char buffer[256];
			strcpy_s(buffer, sm_ModelNameInput);
			if (char* hi = strstr(buffer, "_hi"))
				*hi = '\0';
			m_LodModelHash = fwHelpers::joaat(buffer);

			int len = (int)strlen(sm_ModelNameInput);

			char* hiPtr = strstr(sm_ModelNameInput, "_hi");
			if (highDetail && hiPtr == nullptr) // _hi
			{
				sm_ModelNameInput[len + 0] = '_';
				sm_ModelNameInput[len + 1] = 'h';
				sm_ModelNameInput[len + 2] = 'i';
				sm_ModelNameInput[len + 3] = '\0';
			}
			else if (!highDetail && hiPtr != nullptr)
			{
				*strstr(sm_ModelNameInput, "_hi") = '\0';
			}
		}

		rage::gtaDrawable* TryGetFragment() const
		{
			auto slot = g_FragmentStore->FindSlot(sm_ModelNameInput);

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

		rage::gtaDrawable* TryGetDrawable() const
		{
			auto slot = g_DrawableStore->FindSlot(sm_ModelNameInput);

			if (!slot) return nullptr;

			if (slot->value != nullptr)
				return slot->value;
			return nullptr;
		}

		bool DrawFragmentLodSelector()
		{
			if (m_ModelType != MODEL_FRAGMENT)
				return false;

			static const char* modelMode[] = { "High Detail (_hi)", "Standard" };
			if (ImGui::Combo("LOD##FRAG_LOD", &m_FragLodMode, modelMode, IM_ARRAYSIZE(modelMode)))
			{
				ConvertModelNameTo(m_FragLodMode == 0);
				return true;
			}
			return false;
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
		void OnStart() override
		{
			ShaderUIConfig::Parse();
		}

		void OnRender() override
		{
			if (!ImGui::Begin("Material Editor", &IsVisible))
				return;

			if (ImGui::Button("Reload UI Config"))
				ShaderUIConfig::Parse();

			ImGui::Checkbox("Enable UI Config", &m_EnableUIConfig);
			ImGui::SameLine();
			ImGui::HelpMarker("Use shader variable names, widgets and other properties defined in rageAm/ShaderUIConfig.xml; "
				"Disabling this option will allow you to set any value you want.");

			// TODO: Support these
			static const char* editorModes[] = { "Model", "Instance" };
			ImGui::Combo("Mode##MAT_MODE", &m_EditMode, editorModes, IM_ARRAYSIZE(editorModes));

			if (ImGui::InputText("Name", sm_ModelNameInput, IM_ARRAYSIZE(sm_ModelNameInput)) ||
				DrawFragmentLodSelector())
			{
				m_ModelHash = fwHelpers::joaat(sm_ModelNameInput);
			}

			ImGui::Text("Name hash: %#010x", m_ModelHash);
			FindDrawableAndDrawShaderGroup();

			ImGui::End(); // Material Editor
		}
	};
}
