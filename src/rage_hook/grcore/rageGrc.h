#pragma once
#include "gmHelper.h"
#include "D3D.h"

namespace rh
{
	class grcFragmentProgram
	{
		// Updates constant buffers on GPU (Map & Unmap) and calls PSSetConstantBuffers
		typedef bool (*gDef_SetConstantBuffers)(int64_t grcProgram);

		static inline gDef_SetConstantBuffers gImpl_SetConstantBuffers;

		static bool aImpl_SetConstantBuffers(int64_t inst)
		{
			return gImpl_SetConstantBuffers(inst);
		}

	public:
		grcFragmentProgram()
		{
			gm::ScanAndHook("grcFragmentProgram::SetConstantBuffers",
				"40 53 48 83 EC 20 8B 05 ?? ?? ?? ?? 48 8B D9 F7 D0 23 05 ?? ?? ?? ?? A8 02",
				aImpl_SetConstantBuffers,
				&gImpl_SetConstantBuffers);
		}
	};

	inline grcFragmentProgram g_grcFragmentProgram;
}
