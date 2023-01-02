#pragma once
#include <string>
#include <vector>
#include <imgui.h>

namespace ImGui
{
	template<typename T>
	bool ListBox(const char* label, int* selectedIndex, T t, int size, const char* (getAt)(T, int))
	{
		typedef const char* (*ListBoxParam_Delegate)(T, int);
		struct ListBoxParam
		{
			T t;
			int Size;
			ListBoxParam_Delegate GetAt;
		};
		ListBoxParam param{ t, size, getAt };

		return ListBox(label, selectedIndex,
			[](void* par, int idx, const char** out_text) {
				const ListBoxParam* para = (ListBoxParam*)(par);
				if (idx < 0 || idx >= para->Size)return false;
				*out_text = para->GetAt(para->t, idx);
				return true;
			}, &param, size, size);
	}

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

	inline bool ListBox(const char* label, int* selectedItem, int count, const char* (getItem)(int))
	{
		ImGuiContext& g = *GImGui;

		int height_in_items = count;
		float height_in_items_f = height_in_items + 0.25f;
		ImVec2 size(0.0f, ImFloor(GetTextLineHeightWithSpacing() * height_in_items_f + g.Style.FramePadding.y * 2.0f));

		if (!BeginListBox(label, size))
			return false;

		// Assume all items have even height (= 1 line of text). If you need items of different height,
		// you can create a custom version of ListBox() in your code without using the clipper.
		bool value_changed = false;
		ImGuiListClipper clipper;
		clipper.Begin(count, GetTextLineHeightWithSpacing()); // We know exactly our line height here so we pass it as a minor optimization, but generally you don't need to.
		while (clipper.Step())
		{
			int actualIndex = clipper.DisplayStart;
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++, actualIndex++)
			{
				const char* name = nullptr;
				while (actualIndex < count && !((name = getItem(actualIndex))))
					actualIndex++;

				if (!name) break;

				PushID(actualIndex);
				const bool item_selected = (actualIndex == *selectedItem);
				if (Selectable(name, item_selected))
				{
					*selectedItem = actualIndex;
					value_changed = true;
				}
				if (item_selected)
					SetItemDefaultFocus();
				PopID();
			}
		}
		EndListBox();

		if (value_changed)
			MarkItemEdited(g.LastItemData.ID);

		return value_changed;
	}

	// Helper to display a little (?) mark which shows a tooltip when hovered.
	// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
	static void HelpMarker(const char* desc)
	{
		TextDisabled("(?)");
		if (IsItemHovered(ImGuiHoveredFlags_DelayShort))
		{
			BeginTooltip();
			PushTextWrapPos(GetFontSize() * 35.0f);
			TextUnformatted(desc);
			PopTextWrapPos();
			EndTooltip();
		}
	}
}
