#pragma once
#include <d3d11.h>
#include "../../memory/gmScanner.h"

namespace rh
{
	class grcDX11
	{
		typedef IDXGISwapChain* (*gDef_GetSwapChain)();

		static inline ID3D11Device* gPtr_Device = nullptr;
		static inline ID3D11DeviceContext* gPtr_Context = nullptr;
		static inline gDef_GetSwapChain gImpl_GetSwapChain = nullptr;

	public:
		grcDX11()
		{
			gm::gmAddress addr = g_Scanner.ScanPattern("CreateGameWindowAndGraphics", "48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 48");

			gPtr_Device = *addr.GetAt(0x5B5 + 0x3).CastRef<ID3D11Device**>();
			gPtr_Context = *addr.GetAt(0x597 + 0x3).CastRef<ID3D11DeviceContext**>();

			// Both device and context are stored in RageDirect3DDevice11 + 0x8 and RageDirect3DDeviceContext11 + 0x8
			// but swap chain is in global variable for some reason (why use function anyway? offset would more consistent) TODO:
			gImpl_GetSwapChain = g_Scanner.ScanPattern("GetSwapChain", "E8 ?? ?? ?? ?? 48 8D 55 67 4C 8B 08").Cast<gDef_GetSwapChain>();

			g_Log.LogT("grcDX11::Init() -> D3D11Device: {:X}", reinterpret_cast<uintptr_t>(gPtr_Device));
			g_Log.LogT("grcDX11::Init() -> D3D11DeviceContext: {:X}", reinterpret_cast<uintptr_t>(gPtr_Context));
			g_Log.LogT("grcDX11::Init() -> DxFeatureLevel: {:X}", static_cast<int>(GetDevice()->GetFeatureLevel()));
		}

		static ID3D11Device* GetDevice()
		{
			return gPtr_Device;
		}

		static ID3D11DeviceContext* GetContext()
		{
			return gPtr_Context;
		}

		static IDXGISwapChain* GetSwapChain()
		{
			return gImpl_GetSwapChain();
		}
	};

	inline grcDX11 g_grcDX11;
}
