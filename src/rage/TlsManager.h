#pragma once
#include <windows.h>
#include <d3d11.h>

namespace rage
{
	static constexpr uint32_t TLS_INDEX_DATRESOURCE = 0x190;
	static constexpr uint32_t TLS_INDEX_NUM_DATRESOURCES = 0x18C;
	static constexpr uint32_t TLS_INDEX_D3DCONTEXT = 0x1B0;

	struct datResource;

	class TlsManager
	{
		static constexpr uint32_t TLS_SLOT = 0; // Never changes, always zero.

	public:
		static uint64_t GetPrimarySlotPtr()
		{
			// NtCurrentTeb() + 0x58 = ThreadLocalStoragePointer
			uint64_t pLts = *reinterpret_cast<uint64_t*>(reinterpret_cast<int64_t>(NtCurrentTeb()) + 0x58);
			uint64_t pSlot = *reinterpret_cast<uint64_t*>(pLts + sizeof(uint64_t) * TLS_SLOT);

			return pSlot;
		}

		template<typename T>
		static T Get(uint64_t offset)
		{
			return *reinterpret_cast<T*>(GetPrimarySlotPtr() + offset);
		}

		template<typename T>
		static void Set(uint64_t offset, T value)
		{
			*reinterpret_cast<T*>(GetPrimarySlotPtr() + offset) = value;
		}

		static ID3D11DeviceContext* GetD3D11Context()
		{
			return Get<ID3D11DeviceContext*>(TLS_INDEX_D3DCONTEXT);
		}

		static datResource* GetResource()
		{
			return Get<datResource*>(TLS_INDEX_DATRESOURCE);
		}

		static void SetResource(datResource* value)
		{
			Set<datResource*>(TLS_INDEX_DATRESOURCE, value);
		}
	};
}
