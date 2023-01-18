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
#include "gmhook.h"
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

struct ResourceHeader
{
	u32 Magic;
	u32 Version;
	rage::datResourceInfo Info;
};

struct ResourceFile : ResourceHeader
{
	u8 Data;
};
static_assert(offsetof(ResourceFile, Data) == 0x10);

#include "file/fiCollection.h"
#include "zlib.h"
void ReadResource()
{
	rage::fiPackfile* pack = rage::fiCollection::GetCollection(1);
	rage::fiPackEntry* entry = pack->GetEntry("ranstar.ytd");

	rage::datResourceInfo info = entry->GetResourceInfo();
	rage::datResourceMap map{};
	info.GenerateMap(map);

	// 'Open' file
	u32 offset = entry->GetOffset();
	fiHandle_t file = pack->GetHandle();
	rage::fiDevice* pDevice = pack->GetParent();

	//u32 size = entry->GetOnDiskSize();
	//u32 size2 = pDevice->vftable->Size(pDevice, file);
	u32 size = 99'465; // TODO: How to get actual size?

	char* buffer = new char[size];
	pDevice->vftable->ReadOverlapped(pDevice, file, offset, buffer, size);

	ResourceFile* rscFile = (ResourceFile*)buffer;
	if (rscFile->Magic == 0x37435352 && rscFile->Version == 13) // RSC7, #TD ver. 13
	{
		// Init zlib state
		z_stream st{};
		if (inflateInit2(&st, -15) != Z_OK)
			return;
		st.avail_in = size;
		st.next_in = &rscFile->Data;

		// Allocate map & read chunks from resource file
		for (u8 i = 0; i < map.VirtualChunkCount + map.PhysicalChunkCount; i++)
		{
			rage::datResourceChunk& chunk = map.Chunks[i];
			Bytef* chunkMem = new Bytef[chunk.Size];

			// Decompress chunk
			st.avail_out = chunk.Size;
			st.next_out = chunkMem;
			if (inflate(&st, Z_FULL_FLUSH) < 0)
				return;

			chunk.DestAddr = (u64)chunkMem;
		}
		inflateEnd(&st);

		// Main page is first virtual chunk
		map.MainPage = (rage::pgBase*)map.Chunks[0].DestAddr;

		// Place resource
		rage::datResource rsc(map);
		rage::pgDictionary<rage::grcTexture>* txd =
			new(rsc) rage::pgDictionary<rage::grcTexture>(rsc);

		AM_TRACE("ranstar.ytd:");
		for (u16 i = 0; i < txd->GetCount(); i++)
		{
			const rage::pgKeyPair<rage::grcTexture*>& pair = txd->GetSlot(i);

			AM_TRACEF("{} - [{:X}]: {}", i, pair.Key, pair.Value->GetName());
		}
	}

	// Free map
	for (u8 i = 0; i < map.VirtualChunkCount; i++)
	{
		rage::datResourceChunk& chunk = map.Chunks[i];
		operator delete[]((void*)chunk.DestAddr, chunk.Size);
	}
	delete[] buffer;
}

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

	rage::hooks::RegisterResource();
	rage::hooks::RegisterResourceInfo();
	rh::Rendering::RegisterRender();

	g_ImGui.Init();
	g_ImGuiAppMgr.Init();
	g_ImGuiAppMgr.RegisterApp<sapp::ImGuiApp_Toolbar>();
	g_ImGuiAppMgr.RegisterApp<sapp::ImGuiApp_Overlay>();

	auto currentTime = std::chrono::high_resolution_clock::now();
	g_Log.LogT("Core systems initialization took {}ms", std::chrono::duration<double, std::milli>(currentTime - startTime).count());

	// CrashHandler::ExecuteSafe(ReadResource);
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
