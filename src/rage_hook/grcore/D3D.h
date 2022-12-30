#pragma once
#include <d3d11.h>
#include "../../memory/gmScanner.h"
#include "../../memory/gmHook.h"
#include "../../memory/gmFunc.h"
#include "../../GameVersion.h"

namespace rh
{
	typedef void(*RenderTask)();

	class ID3D11DeviceWrapper : ID3D11Device
	{
		HRESULT CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D) override
		{
			AM_TRACE("ID3D11DeviceWrapper::CreateTexture2D");
			return ID3D11Device::CreateTexture2D(pDesc, pInitialData, ppTexture2D);
		}
	};

	class D3D
	{
		typedef HRESULT(STDMETHODCALLTYPE* gDef_SwapChain_Present)(IDXGISwapChain* Inst, UINT SyncInterval, UINT Flags);
		typedef HRESULT(STDMETHODCALLTYPE* gDef_ID3D11Device_CreateTexture2D)(ID3D11Device* inst, const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D);

		static inline ID3D11Device* gPtr_Device = nullptr;
		static inline ID3D11DeviceContext* gPtr_Context = nullptr;
		static inline IDXGISwapChain* gPtr_SwapChain = nullptr;

		static inline gDef_SwapChain_Present gImpl_SwapChain_Present;
		static inline gDef_ID3D11Device_CreateTexture2D gImpl_ID3D11Device_CreateTexture2D;

		static HRESULT STDMETHODCALLTYPE aImpl_SwapChain_Present(
			IDXGISwapChain* Inst,
			UINT SyncInterval,
			UINT Flags)
		{
			for (auto& task : sm_renderTasks)
			{
				task();
			}
			return gImpl_SwapChain_Present(Inst, SyncInterval, Flags);
		}

		static HRESULT STDMETHODCALLTYPE aImpl_ID3D11Device_CreateTexture2D(
			ID3D11Device* inst,
			const D3D11_TEXTURE2D_DESC* pDesc,
			const D3D11_SUBRESOURCE_DATA* pInitialData,
			ID3D11Texture2D** ppTexture2D)
		{
			AM_TRACE("aImpl_ID3D11Device_CreateTexture2D()");
			return gImpl_ID3D11Device_CreateTexture2D(inst, pDesc, pInitialData, ppTexture2D);

		}

		static inline std::vector<RenderTask> sm_renderTasks;
	public:
		D3D()
		{
			if (GameVersion::IsGreaterOrEqual(VER_2802, 0))
			{
				gm::gmAddress addr = gm::Scan(
					"CreateGameWindowAndGraphics",
					"48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 38 F7");

				gPtr_Device = *addr.GetAt(0x622 + 0x3).CastRef<ID3D11Device**>();
				gPtr_Context = *addr.GetAt(0x602 + 0x3).CastRef<ID3D11DeviceContext**>();
				gPtr_SwapChain = *addr.GetAt(0x631 + 0x3).CastRef<IDXGISwapChain**>();
			}
			else
			{
				gm::gmAddress addr = g_Scanner.ScanPattern(
					"CreateGameWindowAndGraphics",
					"48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 48");

				gPtr_Device = *addr.GetAt(0x5B5 + 0x3).CastRef<ID3D11Device**>();
				gPtr_Context = *addr.GetAt(0x597 + 0x3).CastRef<ID3D11DeviceContext**>();
				gPtr_SwapChain = *addr.GetAt(0x5C3 + 0x3).CastRef<IDXGISwapChain**>();
			}

			g_Log.LogT("D3D::Init() -> D3D11Device: {:X}", reinterpret_cast<uintptr_t>(gPtr_Device));
			g_Log.LogT("D3D::Init() -> D3D11DeviceContext: {:X}", reinterpret_cast<uintptr_t>(gPtr_Context));
			g_Log.LogT("D3D::Init() -> IDXGISwapChain: {:X}", reinterpret_cast<uintptr_t>(gPtr_SwapChain));
			g_Log.LogT("D3D::Init() -> DxFeatureLevel: {:X}", static_cast<int>(GetDevice()->GetFeatureLevel()));

			// IDXGISwapChain::Present is on offset 0x40 (8th function)
			// TODO: Replace this mess with something reasonable
			uintptr_t swapChain_Present = *reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(gPtr_SwapChain) + 0x40);

			g_Hook.SetHook(swapChain_Present,
				aImpl_SwapChain_Present,
				&gImpl_SwapChain_Present);

			//uintptr_t device_CreateTexture2D = *reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(gPtr_Device) + 0x28);

			//g_Hook.SetHook(device_CreateTexture2D,
			//	aImpl_ID3D11Device_CreateTexture2D,
			//	&gImpl_ID3D11Device_CreateTexture2D);
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
			sm_renderTasks.push_back(reinterpret_cast<RenderTask>(task));
		}
	};

	inline D3D g_D3D;
}
