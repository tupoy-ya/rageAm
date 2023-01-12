#pragma once
#include "rage/fwTypes.h"
#include "ImGuiRage.h"
#include <cstdarg>

namespace imgui_rage
{
	class ImGuiApp
	{
		static constexpr u16 TEXT_BUFFER_SIZE = 256;

		bool m_Started = false;
		char m_TextBuffer[TEXT_BUFFER_SIZE];

		static void InternalUpdate(ImGuiApp* inst)
		{
			inst->OnUpdate();
			if (!inst->IsVisible)
				return;
			inst->OnRender();
		}
	protected:
		const ImGuiTableFlags APP_COMMON_TABLE_FLAGS = ImGuiTableFlags_RowBg;
		static inline bool sm_DisplayDebugInfo = false;

	public:
		bool IsVisible = false;
		bool bRenderAlways = false;

	protected:
		const char* Format(const char* fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			vsprintf_s(m_TextBuffer, TEXT_BUFFER_SIZE, fmt, args);
			va_end(args);
			return m_TextBuffer;
		}

		ImFont* GetFont(eImFont font)
		{
			return ImGui::GetIO().Fonts->Fonts[font];
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

			CrashHandler::ExecuteSafe(InternalUpdate, this);
		}
	};
}
