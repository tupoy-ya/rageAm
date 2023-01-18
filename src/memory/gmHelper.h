#pragma once
#include <functional>
#include <Psapi.h>

#include "gmHook.h"
#include "gmScanner.h"
#include "unionCast.h"

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

	inline gmAddress Scan(uintptr_t onScan())
	{
		return onScan();
	}

	inline void SetToNullsub(const char* name, const char* pattern)
	{
		g_Hook.SetToNullsub(Scan(name, pattern));
	}

	template<typename TOriginal>
	LPVOID Hook(const gmAddress& addr, TOriginal detour)
	{
		LPVOID original = nullptr;
		g_Hook.SetHook(addr, gm::CastAny(detour), &original);
		return original;
	}

	template<typename TOriginal>
	LPVOID Hook(const char* name, const char* pattern, TOriginal detour)
	{
		return Hook(Scan(name, pattern), detour);
	}

	// OBSOLETE
	// TODO: Move these to gmScannable
	template<typename T>
	T GetGlobal(const char* name)
	{
		MODULEINFO modInfo{};
		GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &modInfo, sizeof(MODULEINFO));
		uint64_t bAddr = reinterpret_cast<uint64_t>(modInfo.lpBaseOfDll);

		gmAddress addr = gmAddress::GM_ADDRESS_INVALID;

		if (strcmp(name, "sm_ShaderResourceViews") == 0)
		{
			// lea rcx, sm_ShaderResourceViews
			addr = Scan("RenderThread::CleanUp",
				"E8 ?? ?? ?? ?? 4C 8D 9C 24 F0 05 00 00 49 8B 5B 10")
				.GetCall()
				.GetAt(0x4 + 0x3)
				.GetRef();

			goto done;
		}

		if (strcmp(name, "sm_TextureBucketArray") == 0)
		{
			// lea r14, sm_TextureBucketArray
			addr = Scan("RenderThread::AddTextureInRenderBucked",
				"48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 20 80 3D ?? ?? ?? ?? ?? 49")
				.GetAt(0x25 + 0x3)
				.GetRef();

			goto done;
		}

		if (strcmp(name, "sm_SamplerIndices") == 0)
		{
			// movzx r8d, rva sm_SamplerIndices[r12+rax*2]
			addr = Scan("grcEffect::BeginPass_FragmentProgram", "E8 ?? ?? ?? ?? 4C 8B 44 24 70 48 8B")
				.GetAt(0x10)
				.GetCall()
				.GetAt(0x6D + 0x5);
			//g_Log.LogT("OFFSET: {:X}", *addr.Cast<uint32_t*>());
			addr = gmAddress(bAddr + *addr.Cast<uint32_t*>());
			goto done;
		}

		if (strcmp(name, "sm_TextureArray") == 0)
		{
			// mov rdx, rva sm_TextureList[r12+rcx*8]
			addr = Scan("grcEffect::BeginPass_FragmentProgram", "E8 ?? ?? ?? ?? 4C 8B 44 24 70 48 8B")
				.GetAt(0x10)
				.GetCall()
				.GetAt(0x76 + 0x4);
			addr = gmAddress(bAddr + *addr.Cast<uint32_t*>());

			goto done;
		}

		if (strcmp(name, "sm_SamplerList") == 0)
		{
			// lea rcx, sm_ShaderResourceViews
			addr = Scan("RenderThread::SetTextureSampler",
				"48 89 5c 24 ?? 57 48 83 ec ?? 48 8b fa 8b d9 66 45 85 c0")
				.GetAt(0x40 + 0x3)
				.GetRef();

			goto done;
		}

		if (strcmp(name, "sm_CurrentFragmentProgram") == 0)
		{
			// mov cs:sm_CurrentFragmentProgram, rdi
			addr = Scan("RenderThread::ResetAll",
				"48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 55 48 8D A8 08 FB")
				.GetAt(0x2FA + 0x3)
				.GetRef();

			goto done;
		}

		if (strcmp(name, "sm_CurrentFragmentProgramFlag") == 0)
		{
			// and cs:sm_CurrentFragmentProgramFlag, 0
			addr = Scan("RenderThread::CleanUp",
				"E8 ?? ?? ?? ?? 4C 8D 9C 24 F0 05 00 00 49 8B 5B 10")
				.GetCall()
				.GetAt(0x47 + 0x2)
				.GetRef()
				.GetAt(0x1);

			goto done;
		}

		if (strcmp(name, "sm_Streams") == 0)
		{
			// lea pStreams, sm_Streams
			addr = Scan("fiStream::AllocStream",
				"48 89 5C 24 08 57 48 83 EC 20 48 8D 0D ?? ?? ?? ?? 49")
				.GetAt(0x32 + 0x3)
				.GetRef();

			goto done;
		}

		if (strcmp(name, "sm_Buffers") == 0)
		{
			// lea r9, sm_Buffers
			addr = Scan("fiStream::AllocStream",
				"48 89 5C 24 08 57 48 83 EC 20 48 8D 0D ?? ?? ?? ?? 49")
				.GetAt(0x51 + 0x3)
				.GetRef();

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
