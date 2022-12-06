#pragma once

namespace imgui_rage
{
	class ImGuiApp
	{
	public:
		virtual ~ImGuiApp() = default;

		virtual void OnRender() = 0;
	};
}
