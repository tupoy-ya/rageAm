#pragma once
#include "../rage/grmShaderGroup.h"
#include "gmHelper.h"

namespace rh
{
	class grmMaterial
	{
		//// grcEffectVar array is in fact grcInstanceData->effect->variables
		//typedef void(*gDef_UpdateAndBindFragmentProgram)(rage::grcInstanceData*, rage::grcFragmentProgram*, rage::grcEffectVar**);

		//static inline gDef_UpdateAndBindFragmentProgram gImpl_UpdateAndBindFragmentProgram;

		//static void aImpl_UpdateAndBindFragmentProgram(rage::grcInstanceData* inst, rage::grcFragmentProgram* ps, rage::grcEffectVar** variables)
		//{
		//	gImpl_UpdateAndBindFragmentProgram(inst, ps, variables);
		//}

	public:
		grmMaterial()
		{
			//// TODO: This signature is hilarious
			//gm::ScanAndHook("grcInstanceData::UpdateAndBindFragmentProgram",
			//	"48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 54 41 56 41 57 48 83 EC 30 0F B7 42 18 45 33 FF 4D 8B F0 48 8B FA 48 8B E9 41 8B DF 8B F0 85 C0 0F 8E B9 00 00 00 4C 8D 25 F7 38 CC FE",
			//	aImpl_UpdateAndBindFragmentProgram, &gImpl_UpdateAndBindFragmentProgram);
		}
	};

	inline grmMaterial g_grmMaterial;
}
