#pragma once
#include "imgui_internal.h"
#include "../ImGuiApp.h"

namespace sys_apps
{
	class ImGuiApp_MainWindow : public imgui_rage::ImGuiApp
	{
	public:
		void OnRender() override
		{
			auto ctx = ImGui::GetCurrentContext();
			
			ImGui::Begin("Victor Window");
			ImGui::Text("Hello segfault!");
			ImGui::Text("Delta Time: %f", ctx->IO.DeltaTime);
			ImGui::Button("Test");
			ImGui::Button("Test2");
			ImGui::Button("Test3");
			ImGui::End();
		}
	};
}
