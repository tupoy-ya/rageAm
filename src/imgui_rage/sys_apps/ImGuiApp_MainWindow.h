#pragma once
#include "../ImGuiApp.h"

namespace sys_apps
{
	class ImGuiApp_MainWindow : public imgui_rage::ImGuiApp
	{
		bool m_isOpen = true;
		bool m_isBackground = false;


		bool crashed = false;
	public:
		void OnRender() override
		{
			if (!ImGui::Begin("Victor Window", &m_isOpen))
				return;

			rh::GameInput::DisableAllControlsThisFrame();

			ImGui::Text("Hello segfault!");
			ImGui::End();
		}
	};
}
