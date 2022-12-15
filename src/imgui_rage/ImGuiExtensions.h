#pragma once
#include <string>
#include <vector>
#include <imgui.h>

namespace ImGui
{
	inline bool ListBox(const char* label, int* selectedIndex, std::vector<std::string> vector)
	{
		return ListBox(label, selectedIndex,
			[](void* vec, int idx, const char** out_text) {
				std::vector<std::string>* vector = static_cast<std::vector<std::string>*>(vec);
				if (idx < 0 || idx >= vector->size())return false;
				*out_text = vector->at(idx).c_str();
				return true;
			}, &vector, vector.size(), vector.size());
	}

	// Helper to display a little (?) mark which shows a tooltip when hovered.
	// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
	static void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
}
