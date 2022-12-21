#pragma once

// for ImGui_ImplWin32_WndProcHandler
#include <imgui_impl_win32.h>

#include "../memory/gmScanner.h"
#include "../memory/gmHelper.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace rh
{
	class PlatformWin32Impl
	{
		typedef LRESULT(*gDef_WndProc)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
		typedef int (WINAPI* wDef_ShowCursor)(bool);

		static inline gDef_WndProc gImpl_WndProc;

		static LRESULT aImpl_WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
		{
			ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
			return gImpl_WndProc(hWnd, Msg, wParam, lParam);
		}

		static inline HWND* ms_pHwnd;

	public:
		PlatformWin32Impl()
		{
			if (GameVersion::IsGreaterOrEqual(VER_2802, 0))
			{
				gm::gmAddress addr = gm::Scan(
					"CreateGameWindowAndGraphics",
					"48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 38 F7");

				ms_pHwnd = addr
					.GetAt(0x436 + 0x3)
					.CastRef<HWND*>();
			}
			else
			{
				gm::gmAddress addr = gm::Scan("CreateGameWindowAndGraphics",
					"48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 48");

				ms_pHwnd = addr
					.GetAt(0x46A + 0x3)
					.CastRef<HWND*>();
			}

			gm::ScanAndHook("WndProc",
				"48 8B C4 48 89 58 08 4C 89 48 20 55 56 57 41 54 41 55 41 56 41 57 48 8D 68 A1 48 81 EC F0",
				aImpl_WndProc,
				&gImpl_WndProc);

			g_Log.LogT("::PlatformWin32Impl -> hwnd: {}", reinterpret_cast<int64_t>(ms_pHwnd));
		}

		static HWND GetHwnd()
		{
			return *ms_pHwnd;
		}
	};

	inline PlatformWin32Impl g_PlatformWin32Impl;
}
