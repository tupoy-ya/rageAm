#pragma once

namespace imgui_rage
{
	class ImGuiApp
	{
	protected:
		const ImGuiTableFlags APP_COMMON_TABLE_FLAGS = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

	public:
		bool IsVisible = false;

	protected:
		virtual void OnRender() = 0;

	public:
		ImGuiApp() = default;
		virtual ~ImGuiApp() = default;

		void Render()
		{
			if (!IsVisible)
				return;

			OnRender();
		}
	};
}
