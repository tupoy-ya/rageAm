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

#define AM_EXPORT extern "C" __declspec(dllexport)

 // For TaskDialog
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <chrono>

 // Keep it above everything
auto startTime = std::chrono::high_resolution_clock::now();

// We use dynamic initialization with global variable instance to initialize these singletons

/* COMMON */
#include "Logger.h"
#include "CrashHandler.h"
#include "GameVersion.h"
#include "gmScanner.h"
/* COMMON */

/* HOOK INCLUDES */
#include "rage_hook/rageStreaming.h"
#include "rage_hook/rageGrm.h"
#include "rage_hook/rageFileInterface.h"
#include "rage_hook/rageWin32.h"
#include "rage_hook/rageFwTimer.h"
#include "rage_hook/rageControls.h"
#include "rage_hook/rageScaleform.h"
#include "rage/paging/datResource.h"
#include "rage/paging/datResourceInfo.h"
#include "rage/fiStream.h"
#include "rage/framework/fwRenderThreadInterface.h"
#include "rage_hook/grcore/rageD3D.h"
#include "rage_hook/grcore/rageGrc.h"
#include "rage_hook/grcore/rageRender.h"
/* HOOK INCLUDES */

/* FILE OBSERVERS */
#include "rage_hook/file_observer/ShaderStoreThreadInterface.h"
#include "rage_hook/file_observer/TextureStoreThreadInterface.h"
/* FILE OBSERVERS */

#include "imgui_rage/ImGuiRage.h"
#include "imgui_rage/ImGuiAppMgr.h"
#include "imgui_rage/sys_apps/ImGuiApp_Toolbar.h"
#include "imgui_rage/sys_apps/ImGuiApp_Overlay.h"

AM_EXPORT void Shutdown()
{
	g_Log.LogT("main::Shutdown()");

	g_GlobalTextureSwapThreadInterface.Shutdown();
	g_LocalTextureSwapThreadInterface.Shutdown();
	g_ShaderSwapThreadInterface.Shutdown();
}

AM_EXPORT void Init()
{
	g_Log.LogT("main::Init()");

	g_ImGui.Init();
	g_ImGuiAppMgr.Init();
	g_ImGuiAppMgr.RegisterApp<sapp::ImGuiApp_Toolbar>();
	g_ImGuiAppMgr.RegisterApp<sapp::ImGuiApp_Overlay>();

	auto currentTime = std::chrono::high_resolution_clock::now();
	g_Log.LogT("Core systems initialization took {}ms", std::chrono::duration<double, std::milli>(currentTime - startTime).count());
}

void OnAttach()
{
	g_Log.LogT("DllMain -> DLL_PROCESS_ATTACH");
}

void OnDetach()
{
	g_Log.LogT("DllMain -> DLL_PROCESS_DETACH");

	// Disable all hooks instantly to prevent any further calls into already unloaded library.
	// We can't rely on dynamic object destructor because it's called too late.
	g_Hook.Shutdown();
	g_ImGui.Shutdown();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		OnAttach();
		break;
	case DLL_PROCESS_DETACH:
		OnDetach();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}
