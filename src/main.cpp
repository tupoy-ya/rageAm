
#include "main.h"
#include <Windows.h>
#include <fstream>
#include <format>

#include "Memory/Hooking.h"
#include "Logger.h"
#include "../vendor/minhook-1.3.3/include/MinHook.h"
#include "../vendor/scripthook/include/main.h"

#include <iostream>

#include "../vendor/scripthook/include/keyboard.h"
#include "../vendor/scripthook/include/natives.h"
#include "../vendor/scripthook/include/types.h"
#include "../vendor/scripthook/include/enums.h"

#include <d3d11.h>
#include <sstream>

#include "ComponentMgr.h"
//
//struct CExtraContentManager;
//// #include <boost/program_options/option.hpp>
//typedef bool(*SettingMgr__Save)();
//typedef bool(*SettingMgr__BeginSave)(uintptr_t a1);
//
typedef int(*WriteDebugStateToFile)(const WCHAR* fileName);
typedef int(*WriteDebugState)();
//
//typedef __int64 (*UpdateCamFrame)(intptr_t* frame, __int64 a2, __int64 a3);
//
//SettingMgr__Save gimpl_SettingMgr__Save;
WriteDebugStateToFile gimpl_WriteDebugStateToFile;
WriteDebugStateToFile gimpl_WriteDebugState;
//
//uintptr_t save;
//uintptr_t beginSave;
//uintptr_t beginSave_setting_64;
//uintptr_t beginSave_settingDump;
//
uintptr_t writeDebugStateToFile;
uintptr_t writeDebugState;

// Looks like a common structure
uintptr_t numFramesRendered;
uintptr_t isGamePaused;
uintptr_t isDebugPaused;
uintptr_t isPausedUnk1;
uintptr_t isPausedUnk2;

uintptr_t numStreamingRequests;

bool IsGamePaused()
{
	return *(bool*)isDebugPaused ||
		*(bool*)isGamePaused ||
		*(bool*)isPausedUnk1 ||
		*(bool*)isPausedUnk2;
}

enum GameState
{
	SystemInit = 0,
	GameInit = 1,
	GameRunning = 2,
	GameShutdown = 3,
	SystemShutdown = 4,
};

struct CApp
{
	int8_t _gap0[0x10];
	int8_t _gameState;

	GameState GetGameState()
	{
		return (GameState)_gameState;
	}

	std::string GetGameStateStr()
	{
		// TODO: System init is not really possible to get,
		// because game returns it when CGame pointer is set to null,
		// meaning this class instance wont exist
		switch (_gameState)
		{
		case 0: return "System Init";
		case 1: return "Game Init";
		case 2: return "Game Running";
		case 3: return "Game ShutDown";
		case 4: return "System Shutdown";
		}
		return "Unknown";
	}
};

//bool __fastcall aimpl_SettingMgr__BeginSave(uintptr_t a1)
//{
//	g_logger->Log(std::format("SettingMgr::Save({:x})", a1));
//
//	int v1; // edx
//
//	v1 = 203;
//	if (*(unsigned __int16*)(a1 + 64) < 203u) // a1 + 64 seems to be always on 350
//		v1 = *(unsigned __int16*)(a1 + 64);
//	*(int*)beginSave_setting_64 = v1;
//	memmove((void*)beginSave_settingDump, *(const void**)(a1 + 56), 8i64 * v1);
//	return gimpl_SettingMgr__Save();
//}
//
//float cam_x = 0;
//float cam_y = 0;
//float cam_z = 0;
//
//UpdateCamFrame gimpl_UpdateCamFrame = NULL;
//__int64 __fastcall aimpl_UpdateCamFrame(intptr_t* frame, __int64 a2, __int64 a3)
//{
//	if (*(bool*)isDebugPaused)
//	{
//		*(float*)(a2 + 0x40) = cam_x;
//		*(float*)(a2 + 0x44) = cam_y;
//		*(float*)(a2 + 0x48) = cam_z;
//	}
//
//	return gimpl_UpdateCamFrame(frame, a2, a3);
//}
//
//
//struct CPools
//{
//	int64_t qword0;
//	int8_t* pbyte8;
//	int32_t MaxPeds;
//	int int14;
//	int8_t gap18[8];
//	int32_t dword20;
//};
//
//enum PoolType
//{
//	POOL_PED = 128,
//};
//
//struct camFrame
//{
//	float FrontX;
//	float FrontY;
//	float FrontZ;
//	float FrontW;
//	float UpX;
//	float UpY;
//	float UpZ;
//	float UpW;
//	int8_t gap0[32];
//	int32_t PositionX;
//	int32_t PositionY;
//	int32_t PositionZ;
//	int32_t PositionW;
//	int32_t dword50;
//	int32_t dword54;
//	int32_t dword58;
//	int32_t dword5C;
//	int64_t qword60;
//	int64_t qword68;
//	float float70;
//	float float74;
//	int32_t dword78;
//	int32_t dword7C;
//	float float80;
//	int32_t dword84;
//	int32_t dword88;
//	int32_t dword8C;
//	int32_t dword90;
//	int32_t dword94;
//	int32_t dword98;
//	int32_t dword9C;
//	int32_t dwordA0;
//	int32_t dwordA4;
//	int32_t dwordA8;
//	int32_t dwordAC;
//	int32_t dwordB0;
//	int32_t dwordB4;
//	int32_t dwordB8;
//	int32_t dwordBC;
//	int32_t dwordC0;
//	int32_t dwordC4;
//	int32_t dwordC8;
//	int32_t dwordCC;
//	int8_t wordD0;
//};
//
typedef int64_t _QWORD;
//
//typedef __int64 (*GtaThread__RunScript)(
//	__int64 a1,
//	int a2,
//	const void* a3,
//	int a4);
//
//intptr_t gtaThread__RunScript;
//
//GtaThread__RunScript gimpl_GtaThread__RunScript;
//
//__int64 aimpl_GtaThread__RunScript(
//	__int64 a1,
//	int a2,
//	const void* a3,
//	int a4)
//{
//	g_logger->Log(std::format("GtaThread__RunScript: {:x}, {}, {}, {}", a1, a2, a3, a4));
//	return gimpl_GtaThread__RunScript(a1, a2, a3, a4);
//}
//
//BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved);

class MovieEntry
{
	char pad_01[176];
	uint _id;
	char pad_02[4];
	uintptr_t _fileName;
	char pad_03[246];
	uint _state;
	char pad_04[36];

public:
	uint GetId()
	{
		return _id;
	}

	const char* GetFileName()
	{
		return (const char*)&_fileName;
	}

	int GetState()
	{
		return _state;
	}
};
static_assert(sizeof(MovieEntry) == 480);

class MovieStore
{
	MovieEntry* _slots;
	short _slotCount;

public:
	MovieEntry* GetSlot(int index)
	{
		return &_slots[index];
	}

	short GetNumSlots()
	{
		return _slotCount;
	}

	// TODO: Hook
	bool IsSlotActive(int index)
	{
		if (index < 0 || index > 50)
			return false;

		if (index > GetNumSlots())
			return false;

		return GetSlot(index)->GetState() == 3;
	}
};

//typedef intptr_t(*GetEntityToQueryFromGUID)(int index);
//
//GetEntityToQueryFromGUID gImpl_GetEntityToQueryFromGUID;
//
//__int64 __fastcall GetEntityFromGUID(int index)
//{
//	__int64 v1; // r8
//	__int64 v2; // rax
//
//	if (index == -1)
//		return 0i64;
//	v1 = (unsigned int)index / 256;
//
//	auto u0 = *(_QWORD*)(0x273A3D80A60 + 8);
//	auto u1 = *(int8_t*)(v1 + u0);
//	g_logger->Log(std::format("{}", u0));
//	g_logger->Log(std::format("{}", u1));
//
//	if (u1 == (int8_t)index)
//	{
//		auto unk1 = *(int*)(0x273A3D80A60 + 20); // 16
//		auto unk2 = (unsigned int)(v1 * unk1);
//		g_logger->Log(std::format("{}", unk2));
//		v2 = *(_QWORD*)0x273A3D80A60 + unk2;
//	}
//	else
//	{
//		return 0;
//	}
//	return *(_QWORD*)(v2 + 8);
//}

#include "imgui.h"

typedef _QWORD(*PresentImage)();

void Abort()
{
	g_imgui->Destroy();
	g_hook->UnHookAll();
	MH_Uninitialize();
}
//
//#include "Rage/fwFsm.h"
//#include "Rage/CVehicleFactory.h"
//#include "Rage/Pool.h"

typedef int32_t _DWORD;
typedef int8_t _BYTE;

////class CPed
////{
////public:
////	static CPool* GetPool()
////	{
////		return *reinterpret_cast<CPool**>(0x7FF69F7B12C0);
////	}
////};

//
//void DumpPool()
//{
//	const CPool* pool = CPed::GetPool();
//	for (int i = 0; i < pool->GetSize(); i++)
//	{
//		CPed* pPed = pool->GetSlot<CPed>(i);
//
//		if (!pPed)
//			continue;
//
//
//
//		//g_logger->Log(std::format("CPed at slot [{}] : {:X}", i, reinterpret_cast<intptr_t>(pPed)));
//	}
//}
////
//typedef __int64(*Crap)(__int64 fileName, __int64 fileName)
//{
//	0x7FF69DE56B5C
//}


typedef int _WORD;

struct CExtraContentManager
{
	_QWORD vftable;
	_QWORD qword8;
	_DWORD dword10;
	_BYTE gap14[4];
	_QWORD qword18;
	_DWORD dword20;
	_BYTE gap24[4];
	_QWORD dlcList;
	_DWORD numDlc_andWord32;
	_BYTE gap34[4];
	_QWORD qword38;
	_DWORD dword40;
	_BYTE gap44[4];
	_QWORD qword48;
	_DWORD dword50;
	_BYTE gap54[4];
	_BYTE byte58;
	_BYTE gap59[7];
	_QWORD qword60;
	_DWORD dword68;
	_BYTE gap6C[4];
	_QWORD qword70;
	_BYTE gap78[232];
	_DWORD dword160;
	_BYTE gap164[20];
	_BYTE byte178;
};

typedef intptr_t CBaseModelInfo;
//
//enum eEntryFlags
//{
//	MODEL_UNK0 = 0x0,
//	MODEL_LOADED = 0x1,
//};
//
//struct strStreamingModule
//{
//	intptr_t vftable;
//	uint32_t keysOffset;
//};
//
//struct streamingKeyEntry
//{
//	uint32_t unk0;
//	uint32_t flags;
//};

//bool HasModelLoaded(unsigned int hashName)
//{
//	uint16_t modelIndex;
//	const CBaseModelInfo* modelInfo = GetBaseModelInfoFromNameHash(hashName, &modelIndex);
//
//	if (modelInfo && modelIndex != HASH_INDEX_INVALID)
//	{
//		//if ((*(modelInfo + 157) & 0x1F) == 2)
//		//{
//		//	return true;
//		//}
//
//		//const strStreamingModule* models = GetStreamingModule(STREAMING_MODELS);
//		//const streamingKeyEntry* modelEntry = GetStreamingKeyEntry(models, modelIndex);
//
//		//return (modelEntry->flags & (MODEL_UNK0 | MODEL_LOADED)) == 1;
//	}
//	return false;
//}
//
//bool IsModelExists(unsigned int hashName)
//{
//	uint16_t modelIndex;
//	const CBaseModelInfo* modelInfo = GetBaseModelInfoFromNameHash(hashName, &modelIndex);
//
//	return modelInfo && modelIndex != HASH_INDEX_INVALID;
//}


//class CTxdStore
//{
//public:
//	static CPool* GetPool()
//	{
//		return *reinterpret_cast<CPool**>(0x7FF6A010E428);
//	}
//};

struct txdStoreEntry
{
	intptr_t pgDictionary;
	int64_t unk0x8;
	int64_t unk0x10;
};
static_assert(sizeof(txdStoreEntry) == 0x18);


bool logOpen = true;
bool menuOpen = true;
PresentImage gimplPresentImage = NULL;
MovieStore* gPtr_MovieStore;

bool init = false;
float r = 0;
float g = 0;
float b = 0;

//ID3D11Texture2D* refTex;
//ID3D11Resource* refTex;
ID3D11Texture2D* refTex;
ID3D11RenderTargetView* refRen;
ID3D11ShaderResourceView* refRes;

#include "../vendor/directxtk/include/DDSTextureLoader.h"
#include "Rage/Streaming.h"

#define BOOST_STACKTRACE_USE_BACKTRACE
#include <boost/stacktrace.hpp>
#include <strstream>

void LogStackTrace()
{
	g_logger->Log("- stack trace was disabled.");
	//std::stringstream ss;

	//ss << boost::stacktrace::stacktrace();
	//g_logger->Log(ss.str());
}

void OnException()
{
	g_logger->Log(std::format("Exception occurred. Stack Trace:"));
	LogStackTrace();
}

void InvokeWithExceptionHandler(void* func)
{
	__try
	{
		static_cast<void(*)()>(func)();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		OnException();
	}
}

#include "rage/pgDictionary.h"

//void DrawTextures()
//{
//	const auto txdStore = reinterpret_cast<rage::TxdStore*>(rage::strStreamingModuleMgr::GetStreamingModule(rage::STORE_TXD));
//
//	g_logger->Log("Texture Dictionaries:");
//	int countDict = 0;
//	int countText = 0;
//	for (int i = 0; i < txdStore->GetSize(); i++)
//	{
//		countDict++;
//
//		if (!txdStore->IsSlotActive(i))
//			continue;
//
//		const auto value = txdStore->GetSlot(i);
//
//		rage::pgDictionary<rage::grcTexture>* dict = value->GetValue();
//		rage::fwTxdDef def = value->GetKey();
//
//		auto dictPtr = reinterpret_cast<intptr_t>(dict);
//
//		g_logger->Log(std::format("[{}] at {:X} ({:X}) with {} textures:", i, dictPtr, def.nameHash, dict->GetCount()));
//
//		for (int k = 0; k < dict->GetCount(); k++)
//		{
//			countText++;
//
//			rage::grcTexture* texture = dict->GetValue(k);
//
//			const char* name = texture->GetName();
//			auto texturePtr = reinterpret_cast<intptr_t>(texture);
//
//			g_logger->Log(std::format(" - [{}] at {:X} - {}", k, texturePtr, name));
//
//			auto shaderResourceView = texture->GetShaderResourceView();
//			if(shaderResourceView)
//			{
//				ImGui::Image(ImTextureID(shaderResourceView), ImVec2(texture->GetWidth() / 2, texture->GetHeight() / 2));
//			}
//		}
//
//		break;
//	}
//
//	g_logger->Log(std::format("{} dictionaries total, {} textures.", countDict, countText));
//}
rage::strStreamingModuleMgr* streamingMgr;
rage::TxdStore* txdStore;
void DrawDictionary(int index)
{
	if (!txdStore->IsSlotActive(index))
		return;

	const auto value = txdStore->GetSlot(index);

	rage::pgDictionary<rage::grcTexture>* dict = value->GetValue();
	rage::fwTxdDef def = value->GetKey();

	auto dictPtr = reinterpret_cast<intptr_t>(dict);

	for (int k = 0; k < dict->GetCount(); k++)
	{
		rage::grcTexture* texture = dict->GetValue(k);

		const char* name = texture->GetName();
		auto texturePtr = reinterpret_cast<intptr_t>(texture);

		float windowX = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
		ImGuiStyle& style = ImGui::GetStyle();

		auto shaderResourceView = texture->GetShaderResourceView();
		if (shaderResourceView)
		{
			int width = 100;
			float factor = (float)texture->GetWidth() / width;

			float lastImageX = ImGui::GetItemRectMax().x;
			float nextX = lastImageX + style.ItemSpacing.x + width;
			if (nextX < windowX)
				ImGui::SameLine();

			ImGui::Image(ImTextureID(shaderResourceView), ImVec2(width, (float)texture->GetHeight() / factor));
		}
	}
}

int dictionary = 0;
int textureDrawMode = 0;
char inputName[64] = "";

uint32_t atHash(const char* str) {
	size_t i = 0;
	uint32_t hash = 0;
	while (str[i] != '\0') {
		hash += str[i++];
		hash += hash << 10;
		hash ^= hash >> 6;
	}
	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;
	return hash;
}

intptr_t gPtr_lastScaleformMovie;
intptr_t gPtr_lastActionScriptMethod;
intptr_t gPtr_lastActionScriptMethodParams;

void OnPresentImage()
{
	//if (menuOpen)
	//	PAD::DISABLE_ALL_CONTROL_ACTIONS(0);

	if (IsKeyJustUp(VK_SCROLL))
		menuOpen = !menuOpen;

	if (g_imgui->IsInitialized())
	{
		g_imgui->NewFrame();

		ImGui::GetIO().MouseDrawCursor = menuOpen; // Until we have SetCursor hook

		if (menuOpen)
		{
			ImGui::Begin("rageAm", &menuOpen);

			ImGui::Text("Window Handle: %#X", reinterpret_cast<int>(g_gtaWindow->GetHwnd()));
			ImGui::Checkbox("Debug Pause", reinterpret_cast<bool*>(isDebugPaused));

			if(ImGui::TreeNode("Texture Browser"))
			{
				const char* items[] = { "Name", "ID" };
				ImGui::Combo("Mode", &textureDrawMode, items, IM_ARRAYSIZE(items));

				switch (textureDrawMode)
				{
				case 0:
				{
					ImGui::InputText("Name", inputName, IM_ARRAYSIZE(inputName));

					int index;
					txdStore->FindSlotByHashKey(index, atHash(inputName));
					if (index != -1)
						DrawDictionary(index);
					break;
				}
				case 1:
				{
					if (ImGui::Button("<"))
						dictionary++;

					ImGui::SameLine();
					if (ImGui::Button(">"))
						dictionary++;

					DrawDictionary(dictionary);
					break;
				}
				}
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Action Movie"))
			{
				ImGui::Text("Last Movie Info:");
				ImGui::Text("\tName: %s", (const char*)gPtr_lastScaleformMovie);
				ImGui::Text("\tMethod: %s", (const char*)gPtr_lastActionScriptMethod);
				ImGui::Text("\tParams: %s", (const char*)gPtr_lastActionScriptMethodParams);

				ImGui::Text("Active Movies:");
				for (int i = 0; i < gPtr_MovieStore->GetNumSlots(); i++)
				{
					if (!gPtr_MovieStore->IsSlotActive(i))
						continue;

					const auto entry = gPtr_MovieStore->GetSlot(i);
					const uint movieId = entry->GetId();
					const auto movieFileName = entry->GetFileName();

					ImGui::Text("%lu - %s", movieId, movieFileName);
				}
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Log"))
			{
				for (const std::string& entry : g_logger->GetEntries())
				{
					ImGui::Text("%s", entry.c_str());
				}
				ImGui::TreePop();
			}

			ImGui::End();
		}

		g_imgui->Render();
	}
}

_QWORD aimplPresentImage()
{
	InvokeWithExceptionHandler(OnPresentImage);


	//if (IsKeyJustUp(VK_F9))
	//{
	//	float pos[] = { -74.0f, 0.0f, -820.0f, 0.0f, 327.0f };
	//	rage::aImpl_CreateVehicleCommand(0xB779A091, pos, 0, true, false, false);
	//	g_logger->Log("Spawning vehicle...");
	//}

	//if (IsKeyJustUp(VK_SCROLL) && false)
	//{
	//	//ReadTextureDictionaries();
	//	//InvokeWithExceptionHandler(ReadTxds);

	//	return gimplPresentImage();


	//	const Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED(0), true);

	//	gImpl_GetEntityToQueryFromGUID = (GetEntityToQueryFromGUID)(0x7FF69DB7BFD0);

	//	g_logger->Log(std::format("Current Vehicle: {:X}", gImpl_GetEntityToQueryFromGUID(PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED(0), true))));
	//	//invoke<Void>(0x16C2C89DF3A1E544, vehicle, 1.0f);



	//	CExtraContentManager* contentMgr = *(CExtraContentManager**)(0x7FF69F176D40);

	//	for (int i = 0; i < LOWORD(contentMgr->numDlc_andWord32); i += 1)
	//	{
	//		intptr_t dlcPtr = contentMgr->dlcList + 0xF0 * i;

	//		const char* dlcName = *reinterpret_cast<const char**>(dlcPtr + 0x30);
	//		const char* createDate = *reinterpret_cast<const char**>(dlcPtr + 0x50);
	//		const char* rpfPath = *reinterpret_cast<const char**>(dlcPtr + 0xB8);
	//		uint dlcHash = *reinterpret_cast<uint*>(dlcPtr + 0x60);

	//		//g_logger->Log(std::format("[{}] DLC : {:X} Hash: {:X} Name: {}", i, dlcPtr, dlcHash, dlcName));
	//	}


	//	//uint16_t index = HASH_INDEX_INVALID;

	//	//uint bmx = 0x43779C54;
	//	//uint adder = 0xB779A091;

	//	//g_logger->Log(std::format("GetBaseModelInfoFromNameHash: {:X} , {:X}", (intptr_t)GetBaseModelInfoFromNameHash(bmx, &index), index));
	//	//g_logger->Log(std::format("HasModelLoaded(adder): {}", HasModelLoaded(adder)));
	//	//g_logger->Log(std::format("HasModelLoaded(0x0): {}", HasModelLoaded(0x0)));


	//	const auto outVector = new float[10] {};

	//	intptr_t texture = reinterpret_cast<intptr_t(*)(const char*, const char*)>(0x7FF69DBD1FFC)("vehshare", "plate01");
	//	g_logger->Log(std::format("Texture Ptr: {:X}", texture));

	//	texture = reinterpret_cast<intptr_t(*)(const char*, const char*)>(0x7FF69DBD1FFC)("blimp", "blimp_sign_1");
	//	g_logger->Log(std::format("Texture Ptr: {:X}", texture));

	//	reinterpret_cast<void(*)(float*, const char*, const char*)>(0x7FF69DBC0010)(outVector, "adder", "adder_badges");

	//	float x = outVector[0];
	//	float y = outVector[1];
	//	float z = outVector[2];

	//	delete outVector;

	//	//sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&txd);

	//	auto device = *(ID3D11Device**)(0x7ff69fea1c48);
	//	auto devcon = *(ID3D11DeviceContext**)(0x7ff69fea1c50);

	//	ID3D11Texture2D* txd = *(ID3D11Texture2D**)(texture + 0x38);

	//	D3D11_TEXTURE2D_DESC desc{};
	//	txd->GetDesc(&desc);

	//	if (txd)
	//	{
	//		//g_logger->Log(std::format("{:X}", (intptr_t)txd));

	//		ID3D11RenderTargetView* backbuffer;
	//		//HRESULT hResult = (Id3d) //device->CreateRenderTargetView(txd, NULL, &backbuffer);

	//		//g_logger->Log(std::format("hresult: {:X}", (uint)hResult));





	//		//backbuffer = *(ID3D11RenderTargetView**)(texture + 0x78);

	//		//D3D11_RENDER_TARGET_VIEW_DESC desc2{};
	//		//backbuffer->GetDesc(&desc2);




	//		//auto b = *(ID3D11ShaderResourceView**)(texture + 0x78);
	//		//D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	//		//b->GetDesc(&desc);

	//		//g_logger->Log("CRAP");



	//		//auto devcon = g_gtaDirectX->GetContext();


	//		//// Set the viewport
	//		//D3D11_VIEWPORT viewport;
	//		//ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	//		//viewport.TopLeftX = 0;
	//		//viewport.TopLeftY = 0;
	//		//viewport.Width = 800;
	//		//viewport.Height = 600;

	//		//devcon->RSSetViewports(1, &viewport);

	//		//devcon->OMSetRenderTargets(1, &backbuffer, NULL);




	//		//float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };

	//		//devcon->ClearRenderTargetView(backbuffer, color);




	//		//backbuffer->Release();
	//	}

	//}
	//auto texture = reinterpret_cast<intptr_t(*)(const char*, const char*)>(0x7FF69DBD1FFC)("blimp", "blimp_sign_1");
	//auto device = *(ID3D11Device**)(0x7ff69fea1c48);
	//auto devcon = *(ID3D11DeviceContext**)(0x7ff69fea1c50);

	//if (!init)
	//{
	//	init = true;




	//	D3D11_TEXTURE2D_DESC textureDesc;
	//	ZeroMemory(&textureDesc, sizeof(textureDesc));

	//	textureDesc.Width = 512;
	//	textureDesc.Height = 512;
	//	textureDesc.MipLevels = 1;
	//	textureDesc.ArraySize = 1;
	//	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//	textureDesc.SampleDesc.Count = 1;
	//	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	//	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	//	textureDesc.CPUAccessFlags = 0;
	//	textureDesc.MiscFlags = 0;


	//	device->CreateTexture2D(&textureDesc, NULL, &refTex);





	//	//DirectX::CreateDDSTextureFromFile(device, L"C:/Users/falco/Desktop/gray.dds", &refTex, &refRes);





	//	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	//	renderTargetViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	//	renderTargetViewDesc.Texture2D.MipSlice = 0;

	//	device->CreateRenderTargetView(refTex, &renderTargetViewDesc, &refRen);

	//	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	//	shaderResourceViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	//	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	//	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	//	device->CreateShaderResourceView(refTex, &shaderResourceViewDesc, &refRes);
	//}

	//if (texture)
	//{
	//	// ID3D11Texture2D* txd = *(ID3D11Texture2D**)(texture + 0x38);



	//	r += 0.005f;
	//	g += 0.008f;
	//	b += 0.002f;

	//	if (r > 1.0f)
	//		r = 0.4f;

	//	if (g > 1.0f)
	//		g = 0.4f;

	//	if (b > 1.0f)
	//		b = 0.4f;

	//	devcon->OMSetRenderTargets(1, &refRen, NULL);
	//	float ClearColor[4] = { r, g, b, 1.0f };
	//	devcon->ClearRenderTargetView(refRen, ClearColor);

	//	//* (ID3D11Texture2D**)(texture + 0x38) = (ID3D11Texture2D*)refTex;
	//	//*(ID3D11ShaderResourceView**)(texture + 0x78) = refRes;
	//}


	////DumpPool();

	//reinterpret_cast<void (*)(Vector3, Vector3, int, int, int, int)>(0x7FF69DBBB3D8)(
	//	Vector3(0, 0, 0, 0, 0, 0), Vector3(0, 0, 0, 0, 1000, 0), 255, 255, 255, 255);

	return gimplPresentImage();
}

// TODO: Caching
#include <excpt.h>
//
//int16_t unkHash(int32_t key)
//{
//	int v30 = 0xFFFF;
//	int v31 = 0xFFFFFFF;
//	sub_7FF71FEC4DE8(ModelHashKey, &v30);
//	LOWORD(v31) = v30;
//	v31 = v31 & 0xE000FFFF | 0xFFF0000;
//	if (v30 != 0xFFFF)
//	{
//		LOWORD(v30) = sub_7FF71FEC5278(&v31);
//		v30 = ((v31 ^ ((v31 ^ v30) & 0xFFF0000 ^ v30) & 0xDFFFFFFF) & 0x10000000 ^ ((v31 ^ v30) & 0xFFF0000 ^ v30) & 0xDFFFFFFF) & 0x3FFFFFFF;
//}




typedef uint(*GetHash)(int type, const char* str);
typedef intptr_t(*GetModelInfo)(int hash);

typedef intptr_t GameBacktraceConfig;

typedef bool (*gDef_GameBacktraceConfig_WriteToFile)(GameBacktraceConfig inst, const WCHAR* fileName);

gDef_GameBacktraceConfig_WriteToFile gImpl_GameBacktraceConfig_WriteToFile;

bool aImpl_GameBacktraceConfig_WriteToFile(GameBacktraceConfig inst, const WCHAR* fileName)
{
	g_logger->Log("An unhandled exception occurred in game. Stack trace:");
	LogStackTrace();

	return gImpl_GameBacktraceConfig_WriteToFile(inst, fileName);
}

void Main()
{
	g_logger->Log("Init rageAm", true);

	g_logger->Log(std::format("MH_Initialize: {}", MH_Initialize() == MH_OK));


	g_logger->Log("Scanning patterns...");

	g_componentMgr->RegisterComponents();


	intptr_t gPtr_cModelInfo_RequestAssets = g_hook->FindPattern("CModelInfo::RequestAssets", "8B 01 B9 FF FF 00 00 44 8B C2 23 C1 3B C1 75 07");
	intptr_t gPtr_streaming = g_hook->FindOffset("CModelInfo::RequestAssets_gStreaming", gPtr_cModelInfo_RequestAssets + 0x33 + 0x3);
	//streamingMgr = streaming->GetStreamingModuleMgr();
	//g_logger->Log(std::format("StreamingMgr: {:X}", (intptr_t)streamingMgr));


	streamingMgr = (rage::strStreamingModuleMgr*)(gPtr_streaming + 0x1B8);
	txdStore = reinterpret_cast<rage::TxdStore*>(streamingMgr->GetStreamingModule(rage::STORE_TXD));



	g_imgui->Init(g_gtaWindow->GetHwnd());

	intptr_t gPtr_GameBacktraceConfig_WriteToFile = g_hook->FindPattern("GameBacktraceConfig::WriteToFile", "40 53 48 83 EC 30 48 8B CA E8");
	g_hook->SetHook(gPtr_GameBacktraceConfig_WriteToFile, aImpl_GameBacktraceConfig_WriteToFile, &gImpl_GameBacktraceConfig_WriteToFile);

	//g_logger->Log(std::format("bttf present: {}", reinterpret_cast<bool(*)(int hash)>(0x7FF69DBC1CA0)(0xDE76416E)));
	//g_logger->Log(std::format("tttf present: {}", reinterpret_cast<bool(*)(int hash)>(0x7FF69DBC1CA0)(0x6C0D278C)));




	//save = g_hook->FindPattern("SettingMgr::Save", "48 83 EC 48 48 83 3D");
	//beginSave = g_hook->FindPattern("SettingMgr::BeginSave", "40 53 48 83 EC 20 0F B7 41 40");
	//beginSave_setting_64 = g_hook->FindOffset("SettingMgr::BeginSave_setting64", beginSave + 0x1C);
	//beginSave_settingDump = g_hook->FindOffset("SettingMgr::BeginSave_settingDump", beginSave + 0x27);

	writeDebugStateToFile = g_hook->FindPattern("WriteDebugStateToFile", "48 83 EC 48 48 83 64 24 30 00 83 64 24 28 00 45");
	writeDebugState = g_hook->FindPattern("WriteDebugState", "48 8B C4 48 89 58 08 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 90 48 81 EC 80");

	//gtaThread__RunScript = g_hook->FindPattern("GtaThread::RunScript", "48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 48 81 EC 30 01 00 00 49");
	//g_hook->SetHook((LPVOID)gtaThread__RunScript, aimpl_GtaThread__RunScript, (LPVOID*)&gimpl_GtaThread__RunScript);

	gPtr_MovieStore = g_hook->FindOffset<MovieStore*>("WriteDebugState_ScaleformMovieStore", writeDebugState + 0x1091 + 0x3);
	gPtr_lastScaleformMovie = g_hook->FindOffset<intptr_t>("WriteDebugState_lastScaleformMovie", writeDebugState + 0x100F + 0x3);
	gPtr_lastActionScriptMethod = g_hook->FindOffset<intptr_t>("WriteDebugState_lastActionScriptMethod", writeDebugState + 0x1022 + 0x3);
	gPtr_lastActionScriptMethodParams = g_hook->FindOffset<intptr_t>("WriteDebugState_lastActionScriptMethodParams", writeDebugState + 0x1035 + 0x3);

	//rage::SetHooks();
	//rage::HookFactories();

	auto gPtr_PresentImage = g_hook->FindPattern("PresentImage", "40 55 53 56 57 41 54 41 56 41 57 48 8B EC 48 83 EC 40 48 8B 0D");
	g_hook->SetHook(gPtr_PresentImage, aimplPresentImage, &gimplPresentImage);







	//GetHash getHash = g_hook->FindPattern<GetHash>("GetHash", "48 63 C1 48 8B CA");
	//auto getModel = g_hook->FindPattern<GetModelInfo>("GetModelInfo", "40 53 48 83 EC 20 48 8B D9 E8 ?? ?? ?? ?? 8B 13 66 89 44 24 30 8B 44 24 30 8B CA 33 C8 81 E1 00 00 FF 0F 33 C1 48 8D 4C 24 30 0F BA F0 1D 33 D0 81 E2 00 00 00 10 33 C2 25 FF FF FF 3F 89 44 24 30 E8 ?? ?? ?? ?? 45");


	//g_logger->Log(std::format("Model: {:X}", getModel(getHash(0, "adder"))));




	//g_logger->Log(std::format("Hash0: {:X}", getHash(0, "Prop_Screen_VW_InsideTrack")));
	//g_logger->Log(std::format("Hash1: {:X}", getHash(1, "Prop_Screen_VW_InsideTrack")));
	//g_logger->Log(std::format("Hash2: {:X}", getHash(2, "Prop_Screen_VW_InsideTrack")));
	//g_logger->Log(std::format("Hash3: {:X}", getHash(3, "Prop_Screen_VW_InsideTrack")));

	//while (true)
	//{
	//	if (IsKeyJustUp(VK_F9))
	//	{
	//		VEHICLE::CREATE_VEHICLE(0xB779A091, 0, 0, 100, 0, 0, 0, 0);
	//	}

	//	WAIT(1);
	//}

	//while(true)
	//{
	//	if(menuOpen)
	//	{
	//		//PAD::DISABLE_CONTROL_ACTION(0, 0, true);
	//		PAD::DISABLE_ALL_CONTROL_ACTIONS(0);
	//	}

	//	WAIT(0);
	//}

	//intptr_t movieStore = *(intptr_t*)0x7FF72097CF70;
	//short movieSlots = *(short*)(0x7FF72097CF78);

	//// TODO: Test boost stacktrace

	//const auto movieStore = (MovieStore*)(0x7FF72097CF70);
	//g_logger->Log(std::format("Movie Slots: {}", movieStore->GetNumSlots()));
	//for (int i = 0; i < movieStore->GetNumSlots(); i++)
	//{
	//	if (!movieStore->IsSlotActive(i))
	//		continue;

	//	const auto entry = movieStore->GetSlot(i);
	//	uint movieId = entry->GetId();
	//	auto movieFileName = entry->GetFileName();

	//	g_logger->Log(std::format(" - Id({}), FileName({})", movieId, movieFileName));
	//}



	//int pHandle = PLAYER::GET_PLAYER_PED(0);
	////auto getEntityToQuertyFromGuid = reinterpret_cast<GetEntityToQueryFromGUID>(0x7FF71F307EF0);
	//g_logger->Log(std::format("Player Entity: {:x}", GetEntityFromGUID(pHandle)));



	//while (true)
	//{
	//	if (IsKeyJustUp(VK_F9))
	//	{
	//		/*auto fs = g_logger->Open();
	//		fs << boost::stacktrace::stacktrace();
	//		fs.close();*/

	//		// boost::stacktrace::safe_dump_to("Hello.dump");
	//		//std::cout << boost::stacktrace::stacktrace();
	//		//auto bs = boost::stacktrace::stacktrace().as_vector();
	//		//for (auto b : bs)
	//		//{
	//		//	g_logger->Log(b.name());
	//		//}

	//		//SCRIPT::REQUEST_SCRIPT("emergencycall");
	//		//SYSTEM::START_NEW_SCRIPT("emergencycall", 512);
	//		//invoke<int>(0xB8BA7F44DF1575E1, "victor", 0x1, 12390, 5235);
	//	}

	//	WAIT(0);
	//}

	//auto cam = g_hook->FindPattern("UpdateCamFrame", "48 89 5C 24 08 57 48 83 EC 20 8B 42 40 F3");
	//g_hook->SetHook((LPVOID)cam, aimpl_UpdateCamFrame, (LPVOID*)&gimpl_UpdateCamFrame);

	//gimpl_SettingMgr__Save = (SettingMgr__Save)save;
	////g_hook->SetHook((LPVOID)beginSave, &aimpl_SettingMgr__BeginSave);

	//gimpl_WriteDebugStateToFile = (WriteDebugStateToFile)writeDebugStateToFile;

	// mov rax, cs:CApp
	CApp* game = *(CApp**)g_hook->FindOffset("writeDebugState_CApp", writeDebugState + 0xAB + 0x3);

	//// mov edx, dword ptr cs:numFramesRendered
	//numFramesRendered = g_hook->FindOffset("writeDebugState_currentFrame", writeDebugState + 0x159 + 0x2);

	//// or al, cs:isDebugPaused
	isDebugPaused = g_hook->FindOffset("writeDebugState_isDebugPaused", writeDebugState + 0x179 + 0x2);
	//isGamePaused = isDebugPaused - 0x1;
	//isPausedUnk1 = isDebugPaused + 0x1;
	//isPausedUnk2 = isDebugPaused + 0x2;

	//// mov edx, cs:numStreamingRequests
	//numStreamingRequests = g_hook->FindOffset("writeDebugState_numStreamingRequests", writeDebugState + 0x18F + 0x2);

	//g_logger->Log(std::format("CApp - {:x}", (intptr_t)game));
	//if (game)
	//	g_logger->Log(std::format("Game State: {}", game->GetGameStateStr()));
	//g_logger->Log(std::format("Frame: {} - Paused: {}", *(int*)numFramesRendered, IsGamePaused() ? "yes" : "no"));
	//g_logger->Log(std::format("Streaming Requests: {}", *(int*)numStreamingRequests));

	//// TODO:
	//// GetLocalizedString
	//// ReadGameVersion
	//// GetPlayerPosition
	//// CTheScripts::GetEntityToModifyFromGUID__Ped
	//// DirectX Math
	return;

	auto crap1 = (unsigned int)(*(intptr_t*)(0x7FF66B5112C0 + 0x20));
	auto crap2 = (unsigned int)(4 * crap1);
	auto crap3 = (unsigned int)(crap2 >> 2);
	int numPeds = (unsigned int)((4 * *(intptr_t*)(0x7FF66B5112C0 + 0x20)) >> 2);

	//g_logger->Log(std::format("Crap1: {:x} Crap2: {:x} Crap3: {} NumPeds: {}", crap1, crap2, crap3, numPeds));

	//CPools* pool = *(CPools**)0x7FF66B5112C0;

	//auto maxPeds = pool->MaxPeds;
	//auto ped_missions = 0;
	//auto ped_reused = 0;
	//auto ped_reuse_pool = 0;

	//auto __pedIndex = pool->pbyte8;
	//auto qword0 = pool->qword0;
	//auto pedIterator = (unsigned int)maxPeds;

	//g_logger->Log(std::format("dword20: {}", pool->dword20));
	//g_logger->Log(std::format("numPeds: {}", 4 * pool->dword20));
	//g_logger->Log(std::format("numPeds: {}", (4 * pool->dword20) >> 2));
	//g_logger->Log(std::format("numPeds: {}", 1073741882 >> 2));
	//g_logger->Log(std::format("_entryList: {:x}", qword0));


	// From task_commands.cpp
	/*
	 *  CPed::Pool* pool = CPed::GetPool();
		const int maxPeds = pool->GetSize();
		for(int i = 0; i < maxPeds; i++)
		{
			CPed* pPed = pool->GetSlot(i);
			...
		}
	*/

	//for (int i = 0; i < pedIterator; i++)
	//{
	//	auto v31 = *(__pedIndex + i) & 0b10000000;//& POOL_PED; // flag check?

	//	auto pedPtr = qword0 & ~((v31 | -v31) >> 0x3F);
	//	//g_logger->Log(std::format("{} \t {:x}", v31, pedPtr));
	//	g_logger->Log(std::format("{} {:x}", *(__pedIndex + i), pedPtr));
	//	qword0 += pool->int14;
	//}

	//do                                        // Foreach Ped
	//{
	//	//g_logger->Log(std::format("{}", ((uint8_t)*__pedIndex) & 0b10000000));

	//	auto v31 = *__pedIndex & 0b10000000; // flag check?

	//	auto pedPtr = _entryList & ~((v31 | -v31) >> 0x3F);
	//	//g_logger->Log(std::format("{} \t {:x}", v31, pedPtr));
	//	g_logger->Log(std::format("{:x}",pedPtr));

	//	_entryList += pool->int14;
	//	++__pedIndex;
	//	--pedIterator;
	//	continue;

	//	if (!pedPtr || (*(int8_t*)((_entryList & ~((v31 | -(__int64)(*__pedIndex & 128)) >> 0x3F)) + 0x1091) & 0x10) != 0)
	//		pedPtr = 0i64;
	//	if (pedPtr)
	//	{
	//		ped_missions += (*(int16_t*)(pedPtr + 218) & 0xFu) - 6 <= 1;
	//		auto v33 = *(int32_t*)(pedPtr + 5224);
	//		ped_reused += (v33 >> 5) & 1;
	//		ped_reuse_pool += (v33 >> 4) & 1;
	//	}
	//	_entryList += pool->int14;
	//	++__pedIndex;
	//	--pedIterator;
	//} while (pedIterator);

	//g_logger->Log(std::format("missions: {} reused: {} reuse_pool: {}", ped_missions, ped_reused, ped_reuse_pool));

	//gimpl_WriteDebugStateToFile(L"victor.txt");

	//// Main loop
	//while (true)
	//{
	//	if (IsKeyJustUp(VK_F9))
	//	{
	//		*(bool*)isDebugPaused = !*(bool*)isDebugPaused;
	//		gimpl_WriteDebugStateToFile(L"victor.txt");
	//	}

	//	//auto camFrameP = (camFrame*)0x7FF66AF80750;
	//	//auto camFrameA = (intptr_t)0x7FF66AF80750;

	//	//Vector3 pos = ENTITY::GET_ENTITY_COORDS(PLAYER::GET_PLAYER_PED(0), true);
	//	//ENTITY::SET_ENTITY_ALPHA(PLAYER::GET_PLAYER_PED(0), 100, false);

	//	//// Front
	//	//float f_x = pos.x + camFrameP->FrontX;// *(float*)(camFrameA + 0) * 5;
	//	//float f_y = pos.y + camFrameP->FrontY;// *(float*)(camFrameA + 4) * 5;
	//	//float f_z = pos.z + camFrameP->FrontZ;// *(float*)(camFrameA + 8) * 5;

	//	//// Up
	//	//float u_x = pos.x + camFrameP->UpX;// *(float*)(camFrameA + 16) * 1;
	//	//float u_y = pos.y + camFrameP->UpY;// *(float*)(camFrameA + 20) * 1;
	//	//float u_z = pos.z + camFrameP->UpZ;// *(float*)(camFrameA + 24) * 1;

	//	//GRAPHICS::DRAW_LINE(pos.x, pos.y, pos.z, f_x, f_y, f_z, 255, 255, 255, 255);
	//	//GRAPHICS::DRAW_LINE(pos.x, pos.y, pos.z, u_x, u_y, u_z, 255, 255, 255, 255);

	//	//if (*(bool*)isDebugPaused)
	//	//{
	//	//	float moveVertical = 0.0f;
	//	//	float moveHorizontal = 0.0f;

	//	//	moveVertical += IsKeyDown(0x57) ? 1 : 0;
	//	//	moveVertical -= IsKeyDown(0x53) ? 1 : 0;
	//	//	moveHorizontal += IsKeyDown(0x41) ? 1 : 0;
	//	//	moveHorizontal -= IsKeyDown(0x44) ? 1 : 0;

	//	//	float dt = SYSTEM::TIMESTEP();
	//	//	if (moveHorizontal != 0.0f || moveVertical != 0.0f)
	//	//	{
	//	//		float length = 1 / sqrt(moveVertical * moveVertical + moveHorizontal * moveHorizontal);
	//	//		moveVertical *= length;
	//	//		moveHorizontal *= length;

	//	//		moveVertical *= dt;
	//	//		moveHorizontal *= dt;

	//	//		cam_x += moveHorizontal * 50;
	//	//		cam_y += moveVertical * 50;
	//	//	}

	//	//	cam_z += IsKeyDown(VK_SPACE) ? dt * 50 : 0.0f;
	//	//	cam_z -= IsKeyDown(VK_CONTROL) ? dt * 50 : 0.0f;
	//	//}

	//	//if(IsKeyJustUp(VK_SCROLL))
	//	//{
	//	//	gimpl_WriteDebugStateToFile(L"victor.txt");
	//	//}

	//	WAIT(0);
	//}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		InvokeWithExceptionHandler(Main);
		//scriptRegister(hModule, Main);
		keyboardHandlerRegister(OnKeyboardMessage);
		break;
	case DLL_PROCESS_DETACH:
		InvokeWithExceptionHandler(Abort);
		//scriptUnregister(hModule);
		keyboardHandlerUnregister(OnKeyboardMessage);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:break;
	}

	return TRUE;
}
