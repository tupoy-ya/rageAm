#include "ImGuiGta.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "../ComponentMgr.h"

struct ImGui_ImplDX11_Data;

void ImGuiGta::Init(HANDLE hWnd)
{
	if (_isInitialized)
		return;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;// Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	
	// Setup Platform/Renderer backends
	ImGui_ImplGta_Init(hWnd);
	ImGui_ImplDX11_Init(g_gtaDirectX->GetDevice(), g_gtaDirectX->GetContext());

	_isInitialized = true;

	g_logger->Log("ImGuiGta::Init");
}

void ImGuiGta::Destroy()
{
	if (!IsInitialized())
		return;

	// This is bad but otherwise game crashes...
	// I think its related to thread safety
	//ImGui_ImplDX11_Shutdown();
	//ImGui_ImplGta_Shutdown();
	//ImGui::DestroyContext();

	_isInitialized = false;

	g_logger->Log("ImGuiGta::Destroy");
}

void ImGuiGta::NewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplGta_NewFrame();
	ImGui::NewFrame();

	_renderPending = true;

	//g_logger->Log("ImGuiGta::NewFrame");
}

void ImGuiGta::Render()
{
	if (!_renderPending)
		return;

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	ImGui::EndFrame();

	_renderPending = false;

	//g_logger->Log("ImGuiGta::Render");
}

bool ImGuiGta::IsInitialized() const
{
	return _isInitialized;
}

ImGuiGta* g_imgui = ImGuiGta::GetInstance();
