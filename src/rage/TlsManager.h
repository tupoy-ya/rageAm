#pragma once

namespace rage
{
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
			return Get<ID3D11DeviceContext*>(0x1B0);
		}
	};
}
