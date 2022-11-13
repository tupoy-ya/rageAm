#pragma once
#include "../Component.h"
#include "../ComponentMgr.h"
#include "ImGui/ImGuiGta.h"
#include "Memory/Hooking.h"
#include "ImGui/Imgui_impl_gta.h"

typedef LRESULT(* gDef_WndProc)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

extern IMGUI_IMPL_API LRESULT ImGui_ImplGta_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class GtaWindow : public Component
{
	// TODO: Hook create window

	inline static bool _hWndSet = false;
	inline static HWND _gtaWnd = nullptr;
	inline static gDef_WndProc gImpl_WndProc = nullptr;

	static LRESULT aImpl_WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		_gtaWnd = hWnd;

		if(!_hWndSet)
		{
			_hWndSet = true;

			g_logger->Log(std::format("GTA hWnd: {:x}", (int)_gtaWnd));

			g_imgui->Init(_gtaWnd);
		}

		ImGui_ImplGta_WndProcHandler(hWnd, Msg, wParam, lParam);
		return gImpl_WndProc(hWnd, Msg, wParam, lParam);
	}

public:
	void Init() override
	{
		g_hook->SetHook(0x7FF71FBEB6E4, &aImpl_WndProc, &gImpl_WndProc);
	}

	static HANDLE GetHwnd()
	{
		return _gtaWnd;
	}
};
