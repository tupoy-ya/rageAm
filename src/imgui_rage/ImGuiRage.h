#pragma once
#include "imgui.h"

enum eImFont // Keep in sync with ImGuiRage::SetupFonts
{
	IM_FONT_REGULAR = 0,
	IM_FONT_LARGE = 1,
};

namespace imgui_rage
{
	class ImGuiRage
	{
		bool m_isInitialized = false;
		bool m_renderPending = false;

		void SetupFonts();
	public:
		ImGuiRage() = default;
		~ImGuiRage();

		void Init();
		void Shutdown();
		void NewFrame();
		void Render();

		bool IsInitialized() const;
	};
}

inline imgui_rage::ImGuiRage g_ImGui;
