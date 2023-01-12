#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include "ImGuiRage.h"

#include "rage_hook/grcore/rageD3D.h"
#include "rage_hook/rageWin32.h"
#include "Logger.h"

void imgui_rage::ImGuiRage::SetupFonts()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\Tahoma.ttf)", 14.0f);
	io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\Tahoma.ttf)", 36.0f);

	io.FontDefault = io.Fonts->Fonts[IM_FONT_REGULAR];
}

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

	SetupFonts();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Mix of half life 2 and unity
	auto& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.92f, 1.00f, 0.98f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.57f, 0.57f, 0.57f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.95f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.22f, 0.22f, 0.22f, 0.55f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.17f, 0.17f, 0.17f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.68f, 0.67f, 0.32f, 1.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.28f, 0.31f, 0.31f, 0.38f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.68f, 0.67f, 0.32f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.63f, 0.63f, 0.63f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.91f, 0.88f, 0.88f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.38f, 0.44f, 0.42f, 0.38f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.68f, 0.67f, 0.32f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.85f, 0.84f, 0.36f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.68f, 0.67f, 0.32f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);

	style.WindowRounding = 0;
	style.ChildRounding = 0;
	style.FrameRounding = 0;
	style.PopupRounding = 0;
	style.ScrollbarRounding = 9;
	style.GrabRounding = 0;
	style.TabRounding = 4;

	// Setup Platform/Renderer back-ends
	ImGui_ImplWin32_Init(rh::PlatformWin32Impl::GetHwnd());
	ImGui_ImplDX11_Init(rh::D3D::GetDevice(), rh::D3D::GetContext());

	m_isInitialized = true;
}

void imgui_rage::ImGuiRage::Shutdown()
{
	if (!IsInitialized())
		return;

	g_Log.LogT("ImGuiRage::Shutdown()");

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

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
