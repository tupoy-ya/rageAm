#pragma once
#include <dxgi.h>
#include <d3d11.h>
#include <format>
#include "../Memory/Hooking.h"
#include "../Component.h"
#include "../Logger.h"
#include "../ComponentMgr.h"
#include "../GtaCommon.h"

extern std::shared_ptr<GtaCommon> g_gtaCommon;

class GtaDirectX : public Component
{
	typedef IDXGISwapChain* (*gDef_GetSwapChain)();

	ID3D11Device* gPtr_Device;
	ID3D11DeviceContext* gPtr_Context;
	gDef_GetSwapChain gImpl_GetSwapChain;

public:
	GtaDirectX()
	{
		// lea rax, ppImmediateContext
		// ..
		// lea rax, ppDevice
		// ..
		// call cs:D3D11CreateDeviceAndSwapChain
		
		const intptr_t createWndAndGfx = g_gtaCommon->gPtr_CreateGameWindowAndGraphics;

		gPtr_Device = *g_hook->FindOffset<ID3D11Device**>("CreateGameWindowAndGraphics_ID3D11Device", createWndAndGfx + 0x5B5 + 0x3);
		gPtr_Context = *g_hook->FindOffset<ID3D11DeviceContext**>("CreateGameWindowAndGraphics_ID3D11DeviceContext", createWndAndGfx + 0x597 + 0x3);

		// Both device and context are stored in RageDirect3DDevice11 + 0x8 and RageDirect3DDeviceContext11 + 0x8
		// but swap chain is in global variable for some reason
		gImpl_GetSwapChain = g_hook->FindPattern<gDef_GetSwapChain>("GetSwapChain", "E8 ?? ?? ?? ?? 48 8D 55 67 4C 8B 08");

		g_logger->Log(std::format("DxFeatureLevel: {:x}", static_cast<int>(GetDevice()->GetFeatureLevel())));
	}

	ID3D11Device* GetDevice() const
	{
		return gPtr_Device;
	}

	ID3D11DeviceContext* GetContext() const
	{
		return gPtr_Context;
	}

	IDXGISwapChain* GetSwapChain() const
	{
		return gImpl_GetSwapChain();
	}
};
