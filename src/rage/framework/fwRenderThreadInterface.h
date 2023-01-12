#pragma once
#include "gmFunc.h"
#include "unionCast.h"

namespace rage
{
	//class fwRenderThreadInterface;

	//namespace hooks
	//{
	//	static inline gm::gmFuncScan<void, fwRenderThreadInterface*> gImpl_DefaultRenderFunction(
	//		[]() -> auto
	//		{
	//			return gm::Scan("fwRenderThreadInterface::DefaultRenderFunction",
	//				"E8 ?? ?? ?? ?? EB 6D 48 8B 49 48").GetCall();
	//		});
	//}

	//class fwRenderThreadInterface
	//{
	//public:
	//	void DoRenderFunction()
	//	{
	//		hooks::gImpl_DefaultRenderFunction(this);
	//	}
	//};

	//namespace hooks
	//{
	//	static inline gm::gmFuncSwap<void, fwRenderThreadInterface*> gSwap_DoRenderFunction(
	//		"fwRenderThreadInterface::DoRenderFunction",
	//		"48 89 5C 24 08 57 48 83 EC 20 48 8D 99 B0 00 00 00 48 8B F9 48 83",
	//		gm::CastAny(&fwRenderThreadInterface::DoRenderFunction));
	//}
}
