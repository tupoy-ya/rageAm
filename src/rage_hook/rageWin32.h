#pragma once

// for ImGui_ImplRage_WndProcHandler
#include "../imgui_rage/ImGuiImplRage.h"

#include "../memory/gmScanner.h"
#include "../memory/gmHook.h"
#include "../memory/gmHelper.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplRage_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace rh
{
	class PlatformWin32Impl
	{
		typedef LRESULT(*gDef_WndProc)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
		typedef int (WINAPI* wDef_ShowCursor)(bool);

		static inline gDef_WndProc gImpl_WndProc;

		static LRESULT aImpl_WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
		{
			ImGui_ImplRage_WndProcHandler(hWnd, Msg, wParam, lParam);
			return gImpl_WndProc(hWnd, Msg, wParam, lParam);
		}

		static inline HWND ms_hwnd;

	public:
		PlatformWin32Impl()
		{
			gm::gmAddress addr = g_Scanner.ScanPattern("CreateGameWindowAndGraphics",
				"48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 48");

			ms_hwnd = *addr
				.GetAt(0x46A + 0x3)
				.CastRef<HWND*>();

			gm::ScanAndHook("WndProc",
				"48 8B C4 48 89 58 08 4C 89 48 20 55 56 57 41 54 41 55 41 56 41 57 48 8D 68 A1 48 81 EC F0",
				aImpl_WndProc,
				&gImpl_WndProc);

			g_Log.LogT("::PlatformWin32Impl -> hwnd: {}", reinterpret_cast<int64_t>(ms_hwnd));
		}

		static HWND GetHwnd()
		{
			return ms_hwnd;
		}
	};

	inline PlatformWin32Impl g_PlatformWin32Impl;
}
