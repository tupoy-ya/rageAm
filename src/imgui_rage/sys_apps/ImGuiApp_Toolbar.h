#pragma once
#include "imgui_rage/sys_apps/ImGuiApp_MaterialEditor.h"

namespace sapp
{
	class ImGuiApp_Toolbar : public imgui_rage::ImGuiApp
	{
		ImGuiApp_MaterialEditor m_MatEditor;
	public:
		ImGuiApp_Toolbar()
		{
			IsVisible = true;
			// m_MatEditor.IsVisible = true;
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

			ImGui::Checkbox("Global Texture Swap", &rh::TextureSwapThread::IsGlobalSwapOn);
			ImGui::SameLine(); ImGui::HelpMarker(
				"Switches global texture swap (rageAm/Textures/global/).\n"
				"WARNING: For purposes development only! May affect game performance.");

			m_MatEditor.Render();

			ImGui::End();
		}
	};
}
