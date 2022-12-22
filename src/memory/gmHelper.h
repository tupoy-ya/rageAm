#pragma once
#include <Psapi.h>

#include "gmHook.h"
#include "gmScanner.h"

namespace gm
{
	template<typename Detour, typename Original>
	void ScanAndHook(const char* name, const char* pattern, Detour detour, Original original)
	{
		gmAddress addr = g_Scanner.ScanPattern(name, pattern);
		g_Hook.SetHook(addr.GetAddress(), detour, original);
	}

	template<typename Detour>
	void ScanAndHook(const char* name, const char* pattern, Detour detour)
	{
		gmAddress addr = g_Scanner.ScanPattern(name, pattern);
		g_Hook.SetHook(addr.GetAddress(), detour);
	}

	template<typename Func>
	void ScanAndSet(const char* name, const char* pattern, Func* func)
	{
		gmAddress addr = g_Scanner.ScanPattern(name, pattern);
		*func = reinterpret_cast<Func>(addr.GetAddress());
	}

	inline gmAddress Scan(const char* name, const char* pattern)
	{
		return g_Scanner.ScanPattern(name, pattern);
	}

	inline void SetToNullsub(const char* name, const char* pattern)
	{
		g_Hook.SetToNullsub(Scan(name, pattern));
	}

	template<typename T>
	T GetGlobal(const char* name)
	{
		MODULEINFO modInfo{};
		GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &modInfo, sizeof(MODULEINFO));
		uint64_t bAddr = reinterpret_cast<uint64_t>(modInfo.lpBaseOfDll);

		gmAddress addr = gmAddress::GM_ADDRESS_INVALID;

		if (strcmp(name, "ms_ShaderResourceViews") == 0)
		{
			// lea rcx, ms_ShaderResourceViews
			addr = Scan("RenderThread::CleanUp",
				"E8 ?? ?? ?? ?? 4C 8D 9C 24 F0 05 00 00 49 8B 5B 10")
				.GetCall()
				.GetAt(0x4 + 0x3)
				.GetRef();

			goto done;
		}

		if (strcmp(name, "ms_TextureBucketArray") == 0)
		{
			// lea r14, ms_TextureBucketArray
			addr = Scan("RenderThread::AddTextureInRenderBucked",
				"48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 20 80 3D ?? ?? ?? ?? ?? 49")
				.GetAt(0x25 + 0x3)
				.GetRef();

			goto done;
		}

		if (strcmp(name, "ms_SamplerIndices") == 0)
		{
			// movzx r8d, rva ms_SamplerIndices[r12+rax*2]
			addr = Scan("grcEffect::BeginPass_FragmentProgram", "E8 ?? ?? ?? ?? 4C 8B 44 24 70 48 8B")
				.GetAt(0x10)
				.GetCall()
				.GetAt(0x6D + 0x5);
			//g_Log.LogT("OFFSET: {:X}", *addr.Cast<uint32_t*>());
			addr = gmAddress(bAddr + *addr.Cast<uint32_t*>());
			goto done;
		}

		if (strcmp(name, "ms_TextureArray") == 0)
		{
			// mov rdx, rva ms_TextureList[r12+rcx*8]
			addr = Scan("grcEffect::BeginPass_FragmentProgram", "E8 ?? ?? ?? ?? 4C 8B 44 24 70 48 8B")
				.GetAt(0x10)
				.GetCall()
				.GetAt(0x76 + 0x4);
			addr = gmAddress(bAddr + *addr.Cast<uint32_t*>());

			goto done;
		}

		if (strcmp(name, "ms_SamplerList") == 0)
		{
			// lea rcx, ms_ShaderResourceViews
			addr = Scan("RenderThread::SetTextureSampler",
				"48 89 5c 24 ?? 57 48 83 ec ?? 48 8b fa 8b d9 66 45 85 c0")
				.GetAt(0x40 + 0x3)
				.GetRef();

			goto done;
		}

		if (strcmp(name, "ms_CurrentFragmentProgram") == 0)
		{
			// mov cs:ms_CurrentFragmentProgram, rdi
			addr = Scan("RenderThread::ResetAll",
				"48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 55 48 8D A8 08 FB")
				.GetAt(0x2FA + 0x3)
				.GetRef();

			goto done;
		}

		if (strcmp(name, "ms_CurrentFragmentProgramFlag") == 0)
		{
			// and cs:ms_CurrentFragmentProgramFlag, 0
			addr = Scan("RenderThread::CleanUp",
				"E8 ?? ?? ?? ?? 4C 8D 9C 24 F0 05 00 00 49 8B 5B 10")
				.GetCall()
				.GetAt(0x47 + 0x2)
				.GetRef()
				.GetAt(0x1);

			goto done;
		}

	done:
		if (addr == gmAddress::GM_ADDRESS_INVALID)
			g_Log.LogE("Resolving global: {} -> failed.", name);
		else
			g_Log.LogT("Resolving global: {} -> {:X}", name, addr.GetAddress());

		return addr.Cast<T>();
	}
}
