#pragma once
#include <string>
#include <vector>

namespace ImGui
{
	inline void ListBox(const char* label, int* selectedIndex, std::vector<std::string> vector)
	{
		ImGui::ListBox(label, selectedIndex,
			[](void* vec, int idx, const char** out_text) {
				std::vector<std::string>* vector = static_cast<std::vector<std::string>*>(vec);
				if (idx < 0 || idx >= vector->size())return false;
				*out_text = vector->at(idx).c_str();
				return true;
			}, &vector, vector.size(), vector.size());
	}
}
