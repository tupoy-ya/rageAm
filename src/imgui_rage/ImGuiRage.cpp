#include "imgui.h"
#include "imgui_impl_dx11.h"

#include "ImGuiRage.h"
#include "imgui_impl_win32.h"

#include "../rage_hook/grcore/rageDX11.h"
#include "../rage_hook/rageWin32.h"

#include "../Logger.h"

struct ImGui_ImplDX11_Data;

imgui_rage::ImGuiRage::~ImGuiRage()
{
	Shutdown();
}

void imgui_rage::ImGuiRage::Init()
{
	if (m_isInitialized)
		return;

	g_Log.LogT("ImGuiRage::Init()");

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Game-pad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer back-ends
	ImGui_ImplWin32_Init(rh::PlatformWin32Impl::GetHwnd());
	ImGui_ImplDX11_Init(rh::grcDX11::GetDevice(), rh::grcDX11::GetContext());

	m_isInitialized = true;
}

void imgui_rage::ImGuiRage::Shutdown()
{
	if (!IsInitialized())
		return;

	g_Log.LogT("ImGuiRage::Shutdown()");

	// TODO: Figure out why this crashes game
	//ImGui_ImplDX11_Shutdown();
	//ImGui_ImplRage_Shutdown();
	//ImGui::DestroyContext();

	m_isInitialized = false;
}

void imgui_rage::ImGuiRage::NewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	m_renderPending = true;
}

void imgui_rage::ImGuiRage::Render()
{
	if (!m_renderPending)
		return;

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	ImGui::EndFrame();

	m_renderPending = false;
}

bool imgui_rage::ImGuiRage::IsInitialized() const
{
	return m_isInitialized;
}
