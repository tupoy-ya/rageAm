#pragma once
#include <dxgi.h>
#include <d3d11.h>
#include "../Component.h"
#include "ImGui/ImGuiGta.h"
#include "Memory/Hooking.h"

class GtaDirectX : public Component
{
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pImmediateContext;
	IDXGISwapChain* pSwapChain;

public:
	void Init() override
	{
		pDevice = *(ID3D11Device**)0x7FF721601C48;
		pImmediateContext = *(ID3D11DeviceContext**)0x7FF721601C50;
		pSwapChain = *(IDXGISwapChain**)0x7FF7215D0770;

		//g_logger->Log(std::format("DxFeatureLevel: {:x}", (int)pDevice->GetFeatureLevel()));
		//g_hook->SetHook((LPVOID)0x7FF71FBF512C, aimplPresentImage, (LPVOID*)&gimplPresentImage);

		//g_hook->SetHook(0x7FF71FBEB6E4, aImpl_WndProc, &gImpl_WndProc);
	}

	ID3D11Device* GetDevice() const
	{
		return pDevice;
	}

	ID3D11DeviceContext* GetContext() const
	{
		return pImmediateContext;
	}

	IDXGISwapChain* GetSwapChain() const
	{
		return pSwapChain;
	}
};
