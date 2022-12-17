#pragma once
#include <d3d11.h>
#include "../../memory/gmScanner.h"
#include "../../memory/gmHook.h"

namespace rh
{
	typedef void(*RenderTask)();

	class D3D
	{
		typedef HRESULT(STDMETHODCALLTYPE* gDef_SwapChain_Present)(IDXGISwapChain* Inst, UINT SyncInterval, UINT Flags);

		static inline ID3D11Device* gPtr_Device = nullptr;
		static inline ID3D11DeviceContext* gPtr_Context = nullptr;
		static inline IDXGISwapChain* gPtr_SwapChain = nullptr;

		static inline gDef_SwapChain_Present gImpl_SwapChain_Present;

		static HRESULT STDMETHODCALLTYPE gHook_SwapChain_Present(
			IDXGISwapChain* Inst,
			UINT SyncInterval,
			UINT Flags)
		{
			for (auto& task : ms_renderTasks)
			{
				task();
			}
			return gImpl_SwapChain_Present(Inst, SyncInterval, Flags);
		}

		static inline std::vector<RenderTask> ms_renderTasks;
	public:
		D3D()
		{
			gm::gmAddress addr = g_Scanner.ScanPattern("CreateGameWindowAndGraphics", "48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 48");

			gPtr_Device = *addr.GetAt(0x5B5 + 0x3).CastRef<ID3D11Device**>();
			gPtr_Context = *addr.GetAt(0x597 + 0x3).CastRef<ID3D11DeviceContext**>();
			gPtr_SwapChain = *addr.GetAt(0x5C3 + 0x3).CastRef<IDXGISwapChain**>();

			g_Log.LogT("D3D::Init() -> D3D11Device: {:X}", reinterpret_cast<uintptr_t>(gPtr_Device));
			g_Log.LogT("D3D::Init() -> D3D11DeviceContext: {:X}", reinterpret_cast<uintptr_t>(gPtr_Context));
			g_Log.LogT("D3D::Init() -> IDXGISwapChain: {:X}", reinterpret_cast<uintptr_t>(gPtr_SwapChain));
			g_Log.LogT("D3D::Init() -> DxFeatureLevel: {:X}", static_cast<int>(GetDevice()->GetFeatureLevel()));

			// IDXGISwapChain::Present is on offset 0x40 (8th function)
			// TODO: Replace this mess with something reasonable
			uintptr_t swapChain_Present = *reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(gPtr_SwapChain) + 0x40);

			g_Hook.SetHook(swapChain_Present,
				gHook_SwapChain_Present,
				&gImpl_SwapChain_Present);
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
			return gPtr_SwapChain;
		}

		template<typename T>
		static void AddRenderTask(T task)
		{
			ms_renderTasks.push_back(reinterpret_cast<RenderTask>(task));
		}
	};

	inline D3D g_D3D;
}
