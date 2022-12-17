#pragma once
#include "imgui_internal.h"
#include "../ImGuiApp.h"

namespace sapp
{
	class ImGuiApp_Log : public imgui_rage::ImGuiApp
	{
		bool m_ScrollToBottom = false;
		int m_LastSize = 0;

		ImVec4 GetColorForLevel(eLoggerLevel level) const
		{
			switch (level)
			{
			case LOG_ERROR: return ImVec4(1.0f, 0.15f, 0.15f, 1.0f);
			case LOG_TRACE:
			case LOG_DEBUG:
			case LOG_DEFAULT:
			default: return ImVec4(1, 1, 1, 1);
			}
		}
	protected:
		void OnRender() override
		{
			ImGui::Begin("Log Viewer", &IsVisible);

			g_Log.GetMutex()->lock_shared();

			int logCount = g_Log.GetLogCount();
			for (int i = 0; i < logCount; i++)
			{
				LogEntry* entry = g_Log.GetLogEntryAt(i);
				std::string msg = g_Log.GetLogMessageAt(i);

				ImGui::PushStyleColor(ImGuiCol_Text, GetColorForLevel(entry->Level));
				ImGui::TextWrapped("%s", msg.c_str());
				ImGui::PopStyleColor();
			}

			g_Log.GetMutex()->unlock_shared();

			if (logCount != m_LastSize)
			{
				ImGui::SetScrollHereY(1.0f);
				m_LastSize = logCount;
			}

			ImGui::End(); // Log Viewer
		}
	};
}
