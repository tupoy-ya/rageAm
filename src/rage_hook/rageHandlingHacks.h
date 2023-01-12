#pragma once
#include "gmHelper.h"

namespace rh
{
	class rageHandlingHacks
	{
		static void Deluxo_Nullsub()
		{

		}

		typedef void (*gDef_DeluxoSubHandling_Update)(intptr_t vehicle, float a2);

		static inline gDef_DeluxoSubHandling_Update gImpl_DeluxoSubHandling_Update;

		static void aImpl_DeluxoSubHandling_Update(intptr_t vehicle, float a2)
		{
			__try
			{
				gImpl_DeluxoSubHandling_Update(vehicle, a2);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{

			}
		}

	public:
		rageHandlingHacks()
		{
			//gm::ScanAndHook("DeluxoSubHandling::Update", "48 8B C4 F3 0F 11 48 10 55 53 57",
			//	aImpl_DeluxoSubHandling_Update,
			//	&gImpl_DeluxoSubHandling_Update);

			gm::gmAddress addr = gm::Scan("DeluxoSubHandling::UpdateBoneAnimation", "E8 ? ? ? ? EB 30 49 63 0F");
			addr = addr.GetCall();

			g_Hook.SetHook(addr, Deluxo_Nullsub);

			//gm::ScanAndHook("DeluxoSubHandling::UpdateBoneAnimation2",
			//	"48 83 EC 48 83 FA",
			//	Deluxo_Nullsub);

			//// Bugs hover
			//gm::ScanAndHook("DeluxoSubHandling::UpdateSpoilerAnimation",
			//	"48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 56 48 81 EC C0 00 00 00 0F 29 70 E8 0F 29 78 D8 44 0F 29 40 C8 44 0F 29 48 B8 44 0F 29 50 A8 44 0F 29 58 98 44 0F 29 60 88 44 0F 29 6C 24 40 66",
			//	Deluxo_Nullsub);
		}
	};

	inline rageHandlingHacks g_HandlingHacks;
}
