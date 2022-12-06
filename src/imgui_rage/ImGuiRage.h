#pragma once
#include "../rage_hook/grcore/rageDX11.h"

namespace imgui_rage
{
	class ImGuiRage
	{
		bool m_isInitialized = false;
		bool m_renderPending = false;
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
