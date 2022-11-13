#pragma once
#include "../Template/atSingleton.h"
#include "../Graphics/GtaDirectX.h"

class ImGuiGta : public atSingleton<ImGuiGta>
{
	bool _isInitialized = false;
	bool _renderPending = false;
public:
	// TODO: Events
	void Init(HANDLE hWnd);
	void Destroy();
	void NewFrame();
	void Render();

	bool IsInitialized();
};

extern ImGuiGta* g_imgui;
