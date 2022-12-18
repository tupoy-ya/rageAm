/*
 * TODO:
 * Global & Local texture override
 * Swap material shader
 * Developer comments in shader params (like for license plate letters)
 * Sliders in UI
 * Shader & Texture replace
 * Ped, Map Shaders
 * Entity parts highlighting
 * Timecycle editor
 * Trains.xml node editor
 *
 * BUGS:
 * Material editor breaks tuning rendering (related to CCustomShaderEffect)
 */

#include <chrono>
#include <Windows.h>
#include <d3d11.h>
#include "Logger.h"
#include "memory/gmFunc.h"

// Keep it above everything
auto startTime = std::chrono::high_resolution_clock::now();

#define USE_UNHANDLED_CRASH_HANDLER
#ifdef _RELEASE
#define USE_VEH_CRASH_HANDLER // Conflicts with Cheat Engine VEH debugger
#endif
#include "CrashHandler.h"

/* HOOK INCLUDES */

// We use dynamic initialization for pattern scanning,
// so these may be marked as unused but it's not true.
// NOTICE: Order matters here.

#include "rage_hook/grcore/D3D.h"
#include "rage_hook/rageStreaming.h"
#include "rage_hook/grcore/rageGrc.h"
#include "rage_hook/grcore/rageRender.h"
#include "rage_hook/rageGrm.h"
// #include "rage_hook/rageFileInterface.h" // Disabled because not used currently
#include "rage_hook/rageWin32.h"
#include "rage_hook/rageFwTimer.h"
#include "rage_hook/rageControls.h"
#include "rage_hook/rageScaleform.h"

// Global Deluxo Flying Tests
// #include "rage_hook/rageHandlingHacks.h"
// #include "rage_hook/rageHandling.h"

#include "rage/framework/fwRenderThreadInterface.h"

/* HOOK INCLUDES */

/* FILE OBSERVERS */

#include "rage_hook/file_observer/ShaderSwapThreadInterface.h"

/* FILE OBSERVERS */

#include "imgui_rage/ImGuiRage.h"
#include "imgui_rage/ImGuiAppMgr.h"

#include "imgui_rage/sys_apps/ImGuiApp_MainWindow.h"
#include "imgui_rage/sys_apps/ImGuiApp_Toolbar.h"

void Init()
{
	g_Log.LogT("main::Init()");

	g_ImGui.Init();

	g_ImGuiAppMgr.Init();
	g_ImGuiAppMgr.RegisterApp<sapp::ImGuiApp_Toolbar>();

	auto currentTime = std::chrono::high_resolution_clock::now();
	g_Log.LogT("Core systems initialization took {}ms", std::chrono::duration<double, std::milli>(currentTime - startTime).count());

	// Set game level
	// gm::gmFuncFastcall<void, int>(0x7FF7D44F8C20)(2);
}

void Shutdown()
{
	g_Log.LogT("main::Shutdown()");

	// Disable all hooks instantly to prevent any further calls into already unloaded library.
	// We can't rely on dynamic object destructor because it's called too late.
	g_Hook.Shutdown();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	g_RageAmHnd = hModule;

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_Log.LogT("DllMain -> DLL_PROCESS_ATTACH");
		Init();
		break;
	case DLL_PROCESS_DETACH:
		g_Log.LogT("DllMain -> DLL_PROCESS_DETACH");
		Shutdown();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}
