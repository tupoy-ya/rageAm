#pragma once
#include "../rage/grmShaderGroup.h"
#include "../memory/gmHelper.h"

namespace rh
{
	class grmMaterial
	{
		// grmShaderVariable array is in fact grmMaterial->shaderPack->variables
		typedef void(*gDef_UpdateAndBindFragmentProgram)(rage::grmMaterial*, rage::grcFragmentProgram*, rage::grmShaderVariable**);

		static inline gDef_UpdateAndBindFragmentProgram gImpl_UpdateAndBindFragmentProgram;

		static void aImpl_UpdateAndBindFragmentProgram(rage::grmMaterial* inst, rage::grcFragmentProgram* ps, rage::grmShaderVariable** variables)
		{
			gImpl_UpdateAndBindFragmentProgram(inst, ps, variables);
		}

	public:
		grmMaterial()
		{
			// TODO: This signature is hilarious
			gm::ScanAndHook("grmMaterial::UpdateAndBindFragmentProgram",
				"48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 54 41 56 41 57 48 83 EC 30 0F B7 42 18 45 33 FF 4D 8B F0 48 8B FA 48 8B E9 41 8B DF 8B F0 85 C0 0F 8E B9 00 00 00 4C 8D 25 F7 38 CC FE",
				aImpl_UpdateAndBindFragmentProgram, &gImpl_UpdateAndBindFragmentProgram);
		}
	};

	inline grmMaterial g_grmMaterial;
}
