#pragma once

namespace imgui_rage
{
	class ImGuiApp
	{
		static constexpr u16 TEXT_BUFFER_SIZE = 256;

		bool m_Started = false;
		char m_TextBuffer[TEXT_BUFFER_SIZE];

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
		const ImGuiTableFlags APP_COMMON_TABLE_FLAGS = ImGuiTableFlags_RowBg;
		static inline bool sm_DisplayDebugInfo = false;

	public:
		bool IsVisible = false;

	protected:
		const char* Format(const char* fmt, ...)
		{
			va_list argptr;
			va_start(argptr, fmt);
			sprintf_s(m_TextBuffer, TEXT_BUFFER_SIZE, fmt, argptr);
			va_end(argptr);
			return m_TextBuffer;
		}
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
