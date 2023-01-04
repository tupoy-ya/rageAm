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

			tinyxml2::XMLError result = sm_Doc.LoadFile("rageAm/Data/ShaderUI.xml");

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

		static inline char sm_ModelNameInput[INPUT_BUFFER_SIZE] = "cheetah";
		static inline char sm_TextBuffer[TEXT_BUFFER_SIZE]{};

		static inline DXGI_FORMAT sm_TextureFormats[] = {
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
			DXGI_FORMAT_BC1_UNORM_SRGB,
			DXGI_FORMAT_BC3_UNORM_SRGB,
			DXGI_FORMAT_BC4_UNORM,
			DXGI_FORMAT_BC5_UNORM,
			DXGI_FORMAT_BC7_UNORM,
			DXGI_FORMAT_BC7_UNORM_SRGB
		};
		static inline const char* sm_TextureFormatNames[] = {
			"None (RGBA8)", "BC1 (DXT1)", "BC3 (DXT5)","BC4 (ATI1)", "BC5 (ATI2)" }; // , "BC7", "BC7 sRGB"

		int m_EditMode = 0;
		int m_FragLodMode = 0;
		int m_SelectedMaterialIndex = 0;
		int m_SelectedTextureIndex = -1;
		bool m_EnableUIConfig = true;
		bool m_ShowInShaderNames = true;

		u32 m_LodModelHash = -1; // For fragments, game uses non-hi name in many places.
		u32 m_ModelHash = -1;
		eModelType m_ModelType = MODEL_UNKNOWN;
		rage::gtaDrawable* m_Drawable = nullptr;
		rage::grcTexture* m_SelectedTexture = nullptr;
		rage::grmShaderGroup* m_EditShaderGroup = nullptr;

		// Not used now but if we'll have grm shader group copy in future it will be useful
		void OnFragmentRender(uint32_t hash, rage::grmShaderGroup** lpShaderGroup) const
		{
			return;

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

		// Prevents fragment's CCustomShaderEffect to override material values.
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

		// Converts texture format to combobox index.
		int FormatToIndex(DXGI_FORMAT fmt) const
		{
			// Keep in sync with sm_TextureFormats
			if (fmt == DXGI_FORMAT_BC1_UNORM_SRGB) return 1;
			if (fmt == DXGI_FORMAT_BC3_UNORM_SRGB) return 2;
			if (fmt == DXGI_FORMAT_BC4_UNORM) return 3;
			if (fmt == DXGI_FORMAT_BC5_UNORM) return 4;
			if (fmt == DXGI_FORMAT_BC7_UNORM) return 5;
			if (fmt == DXGI_FORMAT_BC7_UNORM_SRGB) return 6;
			return 0;
		}

		// Draws list of textures for current material.
		void DrawMaterialTextureListForDrawable() const
		{
			ImGui::BeginChild("##Textures", ImVec2(0, 0), true);

			int openAction = -1;
			if (ImGui::Button("Open all"))
				openAction = 1;
			ImGui::SameLine();
			if (ImGui::Button("Close all"))
				openAction = 0;

			rage::grmShaderGroup* shaderGroup = m_Drawable->grmShaderGroup;
			rage::grcInstanceData* selectedMaterial = shaderGroup->GetInstanceDataAt(m_SelectedMaterialIndex);

			for (int i = 0; i < selectedMaterial->numVariables; i++)
			{
				rage::grcInstanceVar* instVar = selectedMaterial->GetVariableAtIndex(i);
				rage::grcEffectVar* shaderVar = selectedMaterial->GetEffect()->variables[i];
				rage::eEffectValueType type = shaderVar->GetValueType();

				const char* samplerName = m_ShowInShaderNames ? shaderVar->InShaderName : shaderVar->GetDisplayName();

				if (type != rage::EFFECT_VALUE_TEXTURE)
					continue;

				rage::grcTexture* texture = instVar->GetTexture();

				if (texture == nullptr)
					continue;

				if (texture->GetName() == nullptr)
					continue;

				// There's really no point to view it (it appears as black texture, probably not 2D)
				// + sometimes bugs out and causes weird issues in im gui
				if (strcmp(shaderVar->InShaderName, "DamageSampler") == 0)
					continue;

				if (openAction != -1)
					ImGui::SetNextItemOpen(openAction != 0);
				if (ImGui::CollapsingHeader(samplerName)) // ImGuiTreeNodeFlags_DefaultOpen
				{
					ID3D11ShaderResourceView* textureView = texture->GetShaderResourceView();

					// i.e. bullet_hi\\spec
					static char fullTextureName[256];
					sprintf_s(fullTextureName, 256, "%s\\%s", sm_ModelNameInput, texture->GetName());

					// Get global or local texture swap
					fiobs::TextureStoreEntry* textureSlot =
						rh::Rendering::bEnableGlobalTextureSwap ? g_GlobalTextureSwapThreadInterface.Find(texture->GetName()) : nullptr;
					bool streamedGlobal = textureSlot != nullptr;
					if (!textureSlot)
						textureSlot = g_LocalTextureSwapThreadInterface.Find(fullTextureName);

					// Draw streaming information & status & texture format
					if (textureSlot)
					{
						ImGui::TextColored(ImVec4(0.68f, 0.67f, 0.32f, 1.00f), streamedGlobal ? "Streamed (Global)" : "Streamed (Local)");
						ImGui::SameLine();
						ImGui::HelpMarker("Texture was successfully loaded from local or global folder and replaced default one.");

						// Can't change DDS compression
						if(!textureSlot->IsDDS)
						{
							int compressionMode = FormatToIndex(textureSlot->Format);
							sprintf_s(sm_TextBuffer, TEXT_BUFFER_SIZE, "Compression##%i", i);
							if (ImGui::Combo(sm_TextBuffer, &compressionMode, sm_TextureFormatNames, IM_ARRAYSIZE(sm_TextureFormatNames)))
							{
								textureSlot->Format = sm_TextureFormats[compressionMode];
								textureSlot->RequestReload();
							}
						}

						// Replace texture with streamed one
						textureView = textureSlot->pResourceView;
					}
					else
					{
						ImGui::TextColored({ 1.0f, 0.15f, 0.15f, 1.0f }, "Not Streamed");
						ImGui::SameLine();
						ImGui::HelpMarker("Texture was not found in local / global folders.");
					}

					// Display properties
					ImGui::Text("Name: %s", samplerName);
					ImGui::Text("Path");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(-FLT_MIN); // Stretch editors to whole area
					ImGui::BeginDisabled();
					ImGui::InputText(fullTextureName, fullTextureName, 256);
					ImGui::EndDisabled();

					// Draw texture preview & popup with bigger scale on click
					if (textureView)
					{
						sprintf_s(sm_TextBuffer, TEXT_BUFFER_SIZE, "##POPUP_%s%i", texture->GetName(), i);

						constexpr float width = 128.0f;

						// Rescale texture
						float factor = static_cast<float>(texture->GetWidth()) / width;
						float height = static_cast<float>(texture->GetHeight()) / factor;

						ImGui::Image(textureView, ImVec2(width, height));
						if (ImGui::IsItemClicked())
							ImGui::OpenPopup(sm_TextBuffer);
						ImGui::SameLine();
						ImGui::HelpMarker("Click on texture to open preview.");

						if (ImGui::BeginPopup(sm_TextBuffer))
						{
							constexpr float scale = 6.0f;
							ImGui::Image(textureView, ImVec2(width * scale, height * scale));
							ImGui::EndPopup();
						}
					}
				} // Texture Header
			}
			ImGui::EndChild(); // Textures
		}

		// Draws widget (slider, color picker) for material variable.
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

		// Draws widgets for all variables of current material.
		void DrawMaterialParamListForDrawable() const
		{
			// Material Info Table
			ImGui::BeginTable("MAT_INFO_TABLE", 3, APP_COMMON_TABLE_FLAGS | ImGuiTableFlags_Hideable);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			rage::grmShaderGroup* shaderGroup = m_Drawable->grmShaderGroup;
			rage::grcInstanceData* selectedMaterial = shaderGroup->GetInstanceDataAt(m_SelectedMaterialIndex);

			// Table Rows
			for (int i = 0; i < selectedMaterial->numVariables; i++)
			{
				const rage::grcEffect* effect = selectedMaterial->GetEffect();
				const rage::grcInstanceVar* instVar = selectedMaterial->GetVariableAtIndex(i);
				const rage::grcEffectVar* shaderVar = effect->variables[i];
				const ShaderUIVariable* uiVar = ShaderUIConfig::GetVariableFor(shaderVar->InShaderName);
				rage::eEffectValueType type = shaderVar->GetValueType();

				// We've got separate tab for them
				if (type == rage::EFFECT_VALUE_TEXTURE)
					continue;

				ImGui::TableNextRow();

				// Name
				ImGui::TableSetColumnIndex(0);
				const char* displayName = m_ShowInShaderNames ? shaderVar->InShaderName : shaderVar->GetDisplayName();
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

		// Helper function to get name for item in material list.
		static const char* GetMaterialNameAt(rage::grmShaderGroup* group, int index)
		{
			return group->GetInstanceDataAt(index)->GetEffect()->GetFileName();
		}

		// Draws list box with materials of current shader group.
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

		// Draws material list, it's variables and textures.
		void DrawShaderGroupForDrawable()
		{
			DrawMaterialListForDrawable();
			ImGui::SameLine();

			rage::grmShaderGroup* shaderGroup = m_Drawable->grmShaderGroup;
			rage::grcInstanceData* selectedMaterial = shaderGroup->GetInstanceDataAt(m_SelectedMaterialIndex);

			// Material Info Window
			ImGui::BeginChild("MAT_INFO", ImVec2(0, 0), true, ImGuiWindowFlags_NoBackground);
			ImGui::Checkbox("Use shader variable names", &m_ShowInShaderNames);
			ImGui::SameLine();
			ImGui::HelpMarker("Display variable names as they are defined in HLSL shader.");

			if (sm_DisplayDebugInfo)
			{
				ImGui::Text("Material: %s", selectedMaterial->GetEffect()->GetFileName());
				ImGui::Text("grcEffect: %p", (uint64_t)selectedMaterial->GetEffect());
			}

			ImGui::BeginTabBar("MAT_INFO_TAB_BAR");
			if (ImGui::BeginTabItem("Params"))
			{
				DrawMaterialParamListForDrawable();
				ImGui::EndTabItem(); // Table
			}
			if (ImGui::BeginTabItem("Textures"))
			{
				DrawMaterialTextureListForDrawable();
				ImGui::EndTabItem(); // Textures
			}
			ImGui::EndTabBar(); // MAT_INFO_TAB_BAR
			ImGui::EndChild(); // MAT_INFO
		}

		// Tries to find drawable and draw material window for it.
		void FindDrawableAndDrawShaderGroup()
		{
			ImGui::Text("Model Type:");
			ImGui::SameLine();

			// TODO: Cache drawable
			rage::gtaDrawable* drawable;
			if ((drawable = TryGetFragment()))
			{
				ImGui::Text("Fragment");
				m_ModelType = MODEL_FRAGMENT;

				// Maybe not the best idea to call it on tick
				AdjustFragmentName();
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

			if (sm_DisplayDebugInfo)
				ImGui::Text("grmShaderGroup: %p", (uintptr_t)drawable->grmShaderGroup);

			m_Drawable = drawable;
			DrawShaderGroupForDrawable();
			m_EditShaderGroup = drawable->grmShaderGroup;
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

		// Draws selector between high detailed and standard fragment model.
		bool DrawFragmentLodSelector()
		{
			if (m_ModelType != MODEL_FRAGMENT)
				return false;

			static const char* modelMode[] = { "High Detail (_hi)", "Standard" };
			if (ImGui::Combo("LOD##FRAG_LOD", &m_FragLodMode, modelMode, IM_ARRAYSIZE(modelMode)))
			{
				AdjustFragmentName();
				return true;
			}
			return false;
		}

		// Adds or removes '_hi' postfix in model name.
		void AdjustFragmentName()
		{
			bool highDetail = m_FragLodMode == 0;
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
			ComputeModelHashes();
		}

		// Generates name hash of regular drawable / fragment model name without '_hi' postfix.
		void ComputeModelHashes()
		{
			static char buffer[256];
			strcpy_s(buffer, sm_ModelNameInput);
			if (char* hi = strstr(buffer, "_hi"))
				*hi = '\0';
			m_LodModelHash = fwHelpers::joaat(buffer);
			m_ModelHash = fwHelpers::joaat(sm_ModelNameInput);
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

			// TODO: Support these
			static const char* editorModes[] = { "Model", "Instance" };
			ImGui::Combo("Mode##MAT_MODE", &m_EditMode, editorModes, IM_ARRAYSIZE(editorModes));

			if (ImGui::Button("Reload UI Config"))
				ShaderUIConfig::Parse();

			ImGui::Checkbox("Enable UI Config", &m_EnableUIConfig);
			ImGui::SameLine();
			ImGui::HelpMarker("Use shader variable names, widgets and other properties defined in rageAm/Data/ShaderUI.xml; "
				"Disabling this option will allow you to set any value you want.");

			ImGui::Text("Texture Streaming (Readme)");
			ImGui::SameLine();
			ImGui::HelpMarker("To replace (or stream) new texture (.DDS and raw formats), put texture in 'rageAm/Textures' "
				"global or local folders. Global folder doesn't require model name, but every texture in game with same name will be replaced. "
				"Local folder requires additional folder with model name (you can see exact path for particular texture in 'Textures' tab).");

			ImGui::Separator();

			if (ImGui::InputText("Name", sm_ModelNameInput, IM_ARRAYSIZE(sm_ModelNameInput)) || DrawFragmentLodSelector())
				ComputeModelHashes();

			if (sm_DisplayDebugInfo)
				ImGui::Text("Name hash: %#010x", m_ModelHash);
			FindDrawableAndDrawShaderGroup();

			ImGui::End(); // Material Editor
		}
	public:
		ImGuiApp_MaterialEditor()
		{
			rh::Rendering::OnFragRender.connect(
				boost::bind(&ImGuiApp_MaterialEditor::OnFragmentRender, this, _1, _2));
			rh::Rendering::OnCustomShaderEffectVehicleUse.connect(
				boost::bind(&ImGuiApp_MaterialEditor::OnCustomShaderEffectVehicleUse, this, _1, _2));
		}
	};
}
