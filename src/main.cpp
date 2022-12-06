#include <chrono>
#include <Windows.h>
#include <d3d11.h>
#include "Logger.h"

// Keep it above everything
auto startTime = std::chrono::high_resolution_clock::now();

// We use dynamic initialization for pattern scanning,
// so these may be marked as unused but it's not true.

#include "rage/grmShaderGroup.h"
#include "rage_hook/grcore/rageDX11.h"
#include "rage_hook/grcore/rageGrc.h"
#include "rage_hook/grcore/rageRender.h"
#include "rage_hook/rageGrm.h"
#include "rage_hook/rageFileInterface.h"
#include "rage_hook/rageWin32.h"
#include "rage_hook/rageStreaming.h"
#include "rage_hook/rageFwTimer.h"
#include "rage_hook/rageControls.h"

// #include "rage/fwHelpers.h"

#define IMGUI_USER_CONFIG "imconfig_gta.h"
#include "imgui_rage/ImGuiRage.h"
#include "imgui_rage/ImGuiAppMgr.h"

#include "imgui_rage/sys_apps/ImGuiApp_MainWindow.h"

//typedef int(*WriteDebugStateToFile)(const WCHAR* fileName);
//typedef int(*WriteDebugState)();
//
//WriteDebugStateToFile gimpl_WriteDebugStateToFile;
//WriteDebugStateToFile gimpl_WriteDebugState;
//
//uintptr_t writeDebugStateToFile;
//uintptr_t writeDebugState;
//
//// Looks like a common structure
//uintptr_t numFramesRendered;
//bool* isGamePaused;
//bool* isDebugPaused;
//bool* isPausedUnk1;
//bool* isPausedUnk2;
//
//uintptr_t numStreamingRequests;
//
//bool IsGamePaused()
//{
//	return *isDebugPaused || *isGamePaused || *isPausedUnk1 || *isPausedUnk2;
//}
//
//enum GameState
//{
//	SystemInit = 0,
//	GameInit = 1,
//	GameRunning = 2,
//	GameShutdown = 3,
//	SystemShutdown = 4,
//};
//
//struct CApp
//{
//	int8_t _gap0[0x10];
//	int8_t _gameState;
//
//	GameState GetGameState()
//	{
//		return static_cast<GameState>(_gameState);
//	}
//
//	void SetGameState(GameState state)
//	{
//		_gameState = state;
//	}
//
//	std::string GetGameStateStr()
//	{
//		// TODO: System init is not really possible to get,
//		// because game returns it when CGame pointer is set to null,
//		// meaning this class instance wont exist
//		switch (_gameState)
//		{
//		case 0: return "System Init";
//		case 1: return "Game Init";
//		case 2: return "Game Running";
//		case 3: return "Game ShutDown";
//		case 4: return "System Shutdown";
//		}
//		return "Unknown";
//	}
//};
//
//typedef int64_t _QWORD;
//
//typedef uint32_t uint;
//
//class MovieEntry
//{
//	char pad_01[176];
//	uint _id;
//	char pad_02[4];
//	uintptr_t _fileName;
//	char pad_03[246];
//	uint _state;
//	char pad_04[36];
//
//public:
//	uint GetId()
//	{
//		return _id;
//	}
//
//	const char* GetFileName()
//	{
//		return (const char*)&_fileName;
//	}
//
//	int GetState()
//	{
//		return _state;
//	}
//};
//static_assert(sizeof(MovieEntry) == 480);
//
//class MovieStore
//{
//	MovieEntry* _slots;
//	short _slotCount;
//
//public:
//	MovieEntry* GetSlot(int index)
//	{
//		return &_slots[index];
//	}
//
//	short GetNumSlots()
//	{
//		return _slotCount;
//	}
//
//	bool IsSlotActive(int index)
//	{
//		if (index < 0 || index > 50)
//			return false;
//
//		if (index > GetNumSlots())
//			return false;
//
//		return GetSlot(index)->GetState() == 3;
//	}
//};
//
//#include "imgui.h"
//
//typedef _QWORD(*PresentImage)();
//
//bool logOpen = true;
//bool menuLocked = true;
//PresentImage gimplPresentImage = NULL;
//MovieStore* gPtr_MovieStore;
//
//#include "../vendor/directxtk/include/DDSTextureLoader.h"
//#include "rage/pgDictionary.h"
//
//int textureViewMode = 0;
//
//int dictionary = 0;
//int textureDrawMode = 0;
//char inputName[64] = "";
//char inputModel[64] = "bh1_14_build1";
//
//static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
//
//void DrawDictionary(int index)
//{
//	if (!g_TxdStore->IsSlotActive(index))
//		return;
//
//	const auto value = g_TxdStore->GetSlot(index);
//
//	rage::pgDictionary<rage::grcTexture>* dict = value->GetValue();
//	rage::fwTxdDef def = value->GetKey();
//
//	ImGui::Text("Textures:");
//
//	const char* textureViewModes[] = { "Table", "Grid" };
//	ImGui::Combo("View", &textureViewMode, textureViewModes, IM_ARRAYSIZE(textureViewModes));
//
//	switch (textureViewMode)
//	{
//	case 0:
//		if (ImGui::BeginTable("Textures", 2, tableFlags))
//		{
//			ImGui::TableSetupColumn("Address");
//			ImGui::TableSetupColumn("Name");
//			ImGui::TableHeadersRow();
//		}
//		break;
//	}
//
//	for (int k = 0; k < dict->GetCount(); k++)
//	{
//		rage::grcTexture* texture = dict->GetValue(k);
//
//		const char* name = texture->GetName();
//		auto address = reinterpret_cast<intptr_t>(texture);
//
//		switch (textureViewMode)
//		{
//		case 0:
//		{
//			ImGui::TableNextRow();
//
//			ImGui::TableSetColumnIndex(0);
//			ImGui::Text(std::format("{:X}", address).c_str());
//			ImGui::TableSetColumnIndex(1);
//			ImGui::Text(name);
//			break;
//		}
//		case 1:
//			float windowX = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
//			ImGuiStyle& style = ImGui::GetStyle();
//
//			auto shaderResourceView = texture->GetShaderResourceView();
//			if (shaderResourceView)
//			{
//				int width = 100;
//				float factor = (float)texture->GetWidth() / width;
//
//				float lastImageX = ImGui::GetItemRectMax().x;
//				float nextX = lastImageX + style.ItemSpacing.x + width;
//				if (nextX < windowX && k > 0)
//					ImGui::SameLine();
//
//				ImGui::Image(ImTextureID(shaderResourceView), ImVec2(width, (float)texture->GetHeight() / factor));
//			}
//			break;
//		}
//	}
//
//	switch (textureViewMode)
//	{
//	case 0:
//		ImGui::EndTable();
//		break;
//	}
//}
//
//intptr_t gPtr_lastScaleformMovie;
//intptr_t gPtr_lastActionScriptMethod;
//intptr_t gPtr_lastActionScriptMethodParams;
//
//bool menuOpen = true;
//
//void DisabledInput(const char* text)
//{
//	std::string inputId = "m_messages";
//	inputId.append(text);
//	char input[256];
//	strcpy(input, text);
//	ImGui::PushID(inputId.c_str());
//	ImGui::PushItemWidth(ImGui::GetWindowSize().x);
//	ImGui::InputText("", input, 256, ImGuiInputTextFlags_ReadOnly);
//	ImGui::PopItemWidth();
//	ImGui::PopID();
//}
//
//bool opened = false;
//
//void RenderImGui_Internal()
//{
//	if (g_ImGui.IsInitialized())
//	{
//		if (menuOpen)
//			rh::GameInput::DisableAllControlsThisFrame();
//
//		if (!menuOpen && !menuLocked)
//		{
//			// TODO: Controller still works
//			ImGui::GetIO().MousePos = { 0.0f, 0.0f };
//		}
//
//		//if (IsKeyJustUp(VK_SCROLL))
//		//	menuOpen = !menuOpen;
//
//		g_ImGui.NewFrame();
//
//		ImGui::GetIO().MouseDrawCursor = menuOpen; // Until we have SetCursor hook
//
//		if (menuLocked || menuOpen)
//		{
//			ImGui::Begin("rageAm");
//			ImGui::Checkbox("Locked", &menuLocked);
//			ImGui::Separator();
//
//			//ImGui::Text("Window Handle: %#X", reinterpret_cast<int>(g_rageWnd->GetHwnd()));
//			// ImGui::Checkbox("Debug Pause", reinterpret_cast<bool*>(isDebugPaused));
//			ImGui::Separator();
//
//			if (ImGui::TreeNode("Texture Browser"))
//			{
//				const char* textureDrawModes[] = { "Name", "ID" };
//				ImGui::Combo("Mode", &textureDrawMode, textureDrawModes, IM_ARRAYSIZE(textureDrawModes));
//
//				switch (textureDrawMode)
//				{
//				case 0:
//				{
//					ImGui::InputText("Name", inputName, IM_ARRAYSIZE(inputName));
//
//					int index;
//					g_TxdStore->FindSlotByHashKey(index, fwHelpers::jooat(inputName));
//					if (index != -1)
//					{
//						if (g_TxdStore->IsSlotActive(index))
//						{
//							intptr_t dictPtr = g_TxdStore->GetSlotPtr(index);
//							ImGui::BulletText("Dictionary Address:");
//							ImGui::SameLine(); DisabledInput(std::format("{:X}", dictPtr).c_str());
//						}
//						DrawDictionary(index);
//					}
//					break;
//				}
//				case 1:
//				{
//					if (ImGui::Button("<"))
//						dictionary++;
//
//					ImGui::SameLine();
//					if (ImGui::Button(">"))
//						dictionary++;
//
//					DrawDictionary(dictionary);
//					break;
//				}
//				}
//				ImGui::TreePop();
//			}
//
//			if (ImGui::TreeNode("Drawable Browser"))
//			{
//				ImGui::InputText("Name", inputModel, IM_ARRAYSIZE(inputModel));
//
//				int index;
//				g_DrawableStore->FindSlotByHashKey(index, fwHelpers::jooat(inputModel));
//				if (index != -1 && g_DrawableStore->IsSlotActive(index))
//				{
//					auto slot = g_DrawableStore->GetSlot(index);
//					auto drawable = slot->GetValue();
//
//					ImGui::BulletText("Drawable Address:");
//					ImGui::SameLine(); DisabledInput(std::format("{:X}", (intptr_t)drawable).c_str());
//
//					ImGui::BulletText(std::format("Shader Count: {}", drawable->grmShaderGroup->numMaterials).c_str());
//					for (int i = 0; i < drawable->grmShaderGroup->numMaterials; i++)
//					{
//						auto shaderDef = drawable->grmShaderGroup->materials[i];
//						ImGui::BulletText(std::format("Path: {}", shaderDef->shaderPack->shaderFilePath).c_str());
//					}
//				}
//				ImGui::TreePop();
//			}
//
//			if (ImGui::TreeNode("Shader Group Browser"))
//			{
//				auto grmShaderGroup = reinterpret_cast<rage::grmShaderGroup*>(0x1F8844FFC50);
//
//				ImGui::BulletText(std::format("Material Count: {}", grmShaderGroup->numMaterials).c_str());
//				for (int i = 0; i < grmShaderGroup->numMaterials; i++)
//				{
//					auto material = grmShaderGroup->materials[i];
//					ImGui::Text(std::format("Path: {}", material->shaderPack->shaderFilePath).c_str());
//
//					if (ImGui::BeginTable(material->shaderPack->shaderFilePath, 4, tableFlags))
//					{
//						ImGui::TableSetupColumn("Name");
//						ImGui::TableSetupColumn("Type");
//						ImGui::TableSetupColumn("Value");
//						ImGui::TableSetupColumn("Address");
//						ImGui::TableHeadersRow();
//
//						auto shader = material->shaderPack;
//						for (int k = 0; k < shader->variables.GetSize(); k++)
//						{
//							ImGui::TableNextRow();
//
//							auto shaderVar = shader->variables.GetAt(k);
//
//							int8_t type = *(int8_t*)((intptr_t)shaderVar + 0x0);
//							auto value = material->GetVariableAtIndex(k);
//
//							ImGui::TableSetColumnIndex(0);
//							ImGui::Text(shaderVar->Name);
//							ImGui::TableSetColumnIndex(1);
//							ImGui::Text("%i", type);
//
//							// Control has to have unique ID
//							auto valueId = std::format(
//								"##{}-{}-{}", shaderVar->Name, i, k);
//							// Don't merge in one expression because otherwise
//							// we'll get garbage
//							// https://stackoverflow.com/questions/35980664/why-does-calling-stdstring-c-str-on-a-function-that-returns-a-string-not-wor
//							auto valueIdStr = valueId.c_str();
//
//							ImGui::TableSetColumnIndex(2);
//							if (type == 2) // 4 bytes
//							{
//								//ImGui::Text("%f", value->GetFloat());
//								ImGui::InputFloat(valueIdStr, value->GetFloatPtr());
//							}
//							if (type == 7) // 1 byte
//							{
//								//ImGui::Text(value->GetBool() ? "True" : "False");
//								ImGui::Checkbox(valueIdStr, value->GetBoolPtr());
//							}
//							if (type == 6) // Texture
//							{
//								ImGui::Text(value->GetTexture()->GetName());
//							}
//
//							ImGui::TableSetColumnIndex(3);
//							ImGui::Text(std::format("{:X}", value->pDataValue).c_str());
//						}
//						ImGui::EndTable();
//					}
//				}
//				ImGui::TreePop();
//			}
//
//			if (ImGui::TreeNode("Action Movie"))
//			{
//				ImGui::Text("Last Movie Info:");
//				ImGui::Indent();
//				ImGui::BulletText("Name: %s", (const char*)gPtr_lastScaleformMovie);
//				ImGui::BulletText("Method: %s", (const char*)gPtr_lastActionScriptMethod);
//				ImGui::BulletText("Params:");
//				ImGui::Indent(); ImGui::TextWrapped((const char*)gPtr_lastActionScriptMethodParams); ImGui::Unindent();
//				ImGui::Unindent();
//
//				ImGui::Text("Active Movies:");
//				if (ImGui::BeginTable("ActiveMovies", 2, tableFlags))
//				{
//					ImGui::TableSetupColumn("ID");
//					ImGui::TableSetupColumn("Name");
//					ImGui::TableHeadersRow();
//
//					for (int i = 0; i < gPtr_MovieStore->GetNumSlots(); i++)
//					{
//						if (!gPtr_MovieStore->IsSlotActive(i))
//							continue;
//						ImGui::TableNextRow();
//
//						const auto entry = gPtr_MovieStore->GetSlot(i);
//						const uint movieId = entry->GetId();
//						const auto movieFileName = entry->GetFileName();
//
//						ImGui::TableSetColumnIndex(0);
//						ImGui::Text("%i", movieId);
//						ImGui::TableSetColumnIndex(1);
//						ImGui::Text("%s", movieFileName);
//						//ImGui::Text("%lu - %s", movieId, movieFileName);
//					}
//
//					ImGui::EndTable();
//				}
//				ImGui::TreePop();
//			}
//
//			if (ImGui::TreeNode("Log"))
//			{
//				for (const std::string& entry : g_Log.GetEntries())
//				{
//					ImGui::Text("%s", entry.c_str());
//				}
//				ImGui::TreePop();
//			}
//
//			ImGui::End();
//		}
//
//		g_ImGui.Render();
//	}
//}

HMODULE hModule_rageAm;
PVOID hVectorExceptionHandler;

static HMODULE GetCurrentModule()
{ // NB: XP+ solution!
	HMODULE hModule = nullptr;
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		reinterpret_cast<LPCTSTR>(GetCurrentModule),
		&hModule);

	return hModule;
}

#define BOOST_STACKTRACE_USE_BACKTRACE
#include <boost/stacktrace.hpp>
#include <strstream>

bool m_hasExceptionOccurred = false;
static LONG CALLBACK VectoredExceptionHandler(EXCEPTION_POINTERS* info)
{
	std::stringstream ss;
	ss << boost::stacktrace::stacktrace();

	HMODULE hModule = GetCurrentModule();
	if (hModule == hModule_rageAm)
	{
		if (!m_hasExceptionOccurred)
		{
			g_Log.LogE("An unhandled exception occurred in rageAm: \n{}", ss.str());
			m_hasExceptionOccurred = true;
		}

		// This probably will make every c++ developer in the world angry
		// but at least won't crash whole game and unwind stack properly
		// (hopefully)
		// There's basically 100% chance that it will get in the middle of instruction,
		// causing this handler to be called few more times...
		info->ContextRecord->Rip++;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	g_Log.LogT("An unhandled exception occurred in GTA5: \n{}", ss.str());

	return EXCEPTION_CONTINUE_SEARCH;
}

void Init()
{
	g_Log.LogT("main::Init()");
	hVectorExceptionHandler = AddVectoredExceptionHandler(true, VectoredExceptionHandler);

	g_ImGui.Init();

	g_ImGuiAppMgr.Init();
	g_ImGuiAppMgr.RegisterApp<sys_apps::ImGuiApp_MainWindow>();

	auto currentTime = std::chrono::high_resolution_clock::now();
	g_Log.LogT("Core systems initialization took {}ms", std::chrono::duration<double, std::milli>(currentTime - startTime).count());

	int* p = NULL;
	*p = 10;
	g_Log.LogT("main::Catch()");

	// TODO:
	// mov rax, cs:CApp
	// CApp* game = *(CApp**)g_Hook.FindOffset("writeDebugState_CApp", writeDebugState + 0xAB + 0x3);

	// mov edx, dword ptr cs:numFramesRendered
	// numFramesRendered = g_Hook.FindOffset("writeDebugState_currentFrame", writeDebugState + 0x159 + 0x2);

	// Dump shader list
	// for (int i = 0; i < 346; i++)
	// {
	//	intptr_t address = 0x7FF69FEA8930 + i * sizeof(void*);
	//	uint32_t hash = *(int32_t*)(*(intptr_t*)(address)+0x2C8);

	//	g_Log.Log(std::format("{:X}", hash));
	// }

	//gPtr_MovieStore = g_Hook.FindOffset<MovieStore*>("WriteDebugState_ScaleformMovieStore", writeDebugState + 0x1091 + 0x3);
	//gPtr_lastScaleformMovie = g_Hook.FindOffset<intptr_t>("WriteDebugState_lastScaleformMovie", writeDebugState + 0x100F + 0x3);
	//gPtr_lastActionScriptMethod = g_Hook.FindOffset<intptr_t>("WriteDebugState_lastActionScriptMethod", writeDebugState + 0x1022 + 0x3);
	//gPtr_lastActionScriptMethodParams = g_Hook.FindOffset<intptr_t>("WriteDebugState_lastActionScriptMethodParams", writeDebugState + 0x1035 + 0x3);
}

void Shutdown()
{
	g_Log.LogT("main::Shutdown()");
	RemoveVectoredExceptionHandler(hVectorExceptionHandler);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	hModule_rageAm = hModule;

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		Init();
		break;
	case DLL_PROCESS_DETACH:
		Shutdown();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}
