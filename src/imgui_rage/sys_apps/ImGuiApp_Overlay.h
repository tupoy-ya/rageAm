#pragma once
#include "imgui.h"
#include "../ImGuiApp.h"
#include "backward.hpp"

namespace sapp
{
	class ImGuiApp_Overlay : public imgui_rage::ImGuiApp
	{
	public:
		ImGuiApp_Overlay()
		{
			IsVisible = true;
			bRenderAlways = true;
		}

	protected:
		void OnRender() override
		{
			const char* frameName;
			if (backward::SignalHandling::GetDebugBreak(&frameName))
			{
				ImGui::Begin("##DEBUG_BREAK", nullptr,
					ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground |
					ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
				ImGui::PushFont(GetFont(IM_FONT_LARGE));
				ImGui::TextColored({ 1.0f, 0.15f, 0.15f, 1.0f }, "DEBUG BREAK AT %s", frameName);
				ImGui::PopFont();
				ImGui::End();
			}
		}
	};
}
