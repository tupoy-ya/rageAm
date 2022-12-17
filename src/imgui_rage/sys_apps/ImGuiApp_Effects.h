#pragma once
#include "imgui_internal.h"
#include "../ImGuiApp.h"
#include "ImGuiApp_Toolbar.h"

namespace sapp
{
	class ImGuiApp_Effects : public imgui_rage::ImGuiApp
	{
	protected:
		void OnRender() override
		{
			ImGui::Begin("Effect Viewer", &IsVisible);

			rage::grcEffect** effectArray = (rage::grcEffect**)0x7FF6E2158930;
			int effectArrayIndex = *(int*)0x7FF6E2159930;

			for (int i = 0; i < effectArrayIndex; i++)
			{
				rage::grcEffect* effect = effectArray[i];

				ImGui::Text("%s", effect->GetFilePath());
			}

			ImGui::End(); // Effect Viewer
		}
	};
}
