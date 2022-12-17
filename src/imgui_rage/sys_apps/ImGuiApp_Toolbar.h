#pragma once
#include "imgui_rage/sys_apps/ImGuiApp_MaterialEditor.h"
#include "imgui_rage/sys_apps/ImGuiApp_Effects.h"
#include "imgui_rage/sys_apps/ImGuiApp_Log.h"
#include "../../rage_hook/file_observer/FileObserverThread.h"

namespace sapp
{
	class ImGuiApp_Toolbar : public imgui_rage::ImGuiApp
	{
		ImGuiApp_MaterialEditor m_MatEditor;
		ImGuiApp_Effects m_Effects;
		ImGuiApp_Log m_Log;
	public:
		ImGuiApp_Toolbar()
		{
			IsVisible = true;
			// m_MatEditor.IsVisible = true;
			//m_Effects.IsVisible = true;
			m_Log.IsVisible = true;
		}

		void OnRender() override
		{
			ImGui::Begin("rageAm Toolbar", nullptr, ImGuiWindowFlags_MenuBar);

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("View"))
				{
					ImGui::MenuItem("Material Editor", nullptr, &m_MatEditor.IsVisible);
					ImGui::MenuItem("Effect Viewer", nullptr, &m_Effects.IsVisible);
					ImGui::MenuItem("Log Viewer", nullptr, &m_Log.IsVisible);
					ImGui::EndMenu();
				} // BeginMenu
				ImGui::EndMenuBar();
			} // BeginMenuBar

			ImGui::Checkbox("Global Texture Swap", &fiobs::g_TextureSwapThread.IsGlobalSwapOn);
			ImGui::SameLine(); ImGui::HelpMarker(
				"Switches global texture swap (rageAm/Textures/global/).\n"
				"WARNING: For purposes development only! May affect game performance.");

			m_MatEditor.Render();
			m_Effects.Render();
			m_Log.Render();

			ImGui::End();
		}
	};
}
