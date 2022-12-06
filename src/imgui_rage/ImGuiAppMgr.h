#pragma once
#include "ImGuiApp.h"
#include <vector>
#include <memory>

namespace imgui_rage
{
	class ImGuiAppMgr;
}
extern imgui_rage::ImGuiAppMgr g_ImGuiAppMgr;

namespace imgui_rage
{
	class ImGuiAppMgr
	{
		static inline std::vector<std::unique_ptr<ImGuiApp>> ms_apps;

		bool ms_isBackground = false;
		bool ms_isVisible = true;

		void UpdateAll() const
		{
			if (!g_ImGui.IsInitialized())
				return;

			if (!ms_isVisible)
				return;

			if (ms_apps.empty())
				return;

			if (ms_isBackground)
				rh::GameInput::DisableAllControlsThisFrame();

			g_ImGui.NewFrame();

			for (auto& app : ms_apps)
			{
				app->OnRender();
			}

			g_ImGui.Render();
		}

		static void OnRender()
		{
			g_ImGuiAppMgr.UpdateAll();
		}
	public:
		void Init() const
		{
			rh::RenderThread::AddRenderTask(OnRender);
		}

		template<typename T>
		void RegisterApp()
		{
			ms_apps.push_back(std::make_unique<T>());
		}
	};
}

inline imgui_rage::ImGuiAppMgr g_ImGuiAppMgr;
