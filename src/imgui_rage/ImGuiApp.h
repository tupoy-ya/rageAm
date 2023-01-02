#pragma once

namespace imgui_rage
{
	class ImGuiApp
	{
		bool m_Started = false;

		void InternalUpdate()
		{
			OnUpdate();
			if (!IsVisible)
				return;
			OnRender();
		}

		void ReportException()
		{
			// TODO: Not really good to duplicate stack trace code here
			std::stringstream ss{};
			ss << boost::stacktrace::stacktrace();
			AM_ERRF("An error occured while executing app: {}\n{}", typeid(this).name(), ss.str());
		}
	protected:
		const ImGuiTableFlags APP_COMMON_TABLE_FLAGS = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

	public:
		bool IsVisible = false;

	protected:
		virtual void OnStart() {}
		virtual void OnRender() = 0;
		virtual void OnUpdate() {}

	public:
		ImGuiApp() = default;
		virtual ~ImGuiApp() = default;

		void Update()
		{
			if (!m_Started)
			{
				OnStart();
				m_Started = true;
			}

			// Stupid exceptions happen quite often so don't let it crash GTA
			__try
			{
				InternalUpdate();
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				ReportException();
			}
		}
	};
}
