#pragma once
#include <set>

#include "../logger.h"
#include "../../vendor/minhook-1.3.3/include/MinHook.h"

namespace gm
{
	class gmHook
	{
		std::set<LPVOID> m_hookedFuncs;

		void SetHook_Internal(LPVOID func, LPVOID detour, LPVOID* orig);
		static const char* GetMHStatusStr(MH_STATUS e);

		bool m_IsInitialized;

		static void Nullsub() {}
	public:
		gmHook();
		~gmHook();

		void Shutdown();

		void UnHook(LPVOID addr);

		template<typename Original>
		void SetHook(LPVOID func, LPVOID detour, Original orig)
		{
			SetHook_Internal(func, detour, reinterpret_cast<LPVOID*>(orig));
		}

		template<typename Detour, typename Original>
		void SetHook(uintptr_t addr, Detour detour, Original orig)
		{
			SetHook_Internal(reinterpret_cast<LPVOID>(addr),
				reinterpret_cast<LPVOID>(detour), reinterpret_cast<LPVOID*>(orig));
		}

		template<typename Detour>
		void SetHook(uintptr_t func, Detour detour)
		{
			SetHook_Internal(reinterpret_cast<LPVOID>(func), reinterpret_cast<LPVOID>(detour), nullptr);
		}

		void SetToNullsub(uintptr_t func)
		{
			SetHook_Internal(reinterpret_cast<LPVOID>(func), reinterpret_cast<LPVOID>(Nullsub), nullptr);
		}
	};
}

inline gm::gmHook g_Hook;
