#pragma once
#include "../memory/gmScanner.h"
#include "../rage/Streaming.h"

inline rage::strStreamingModuleMgr* g_StreamingMgr;
inline rage::TxdStore* g_TxdStore;
inline rage::DrawableStore* g_DrawableStore;
inline rage::FragmentStore* g_FragmentStore;

namespace rh
{
	class strStreamingModuleMgr
	{
	public:
		strStreamingModuleMgr()
		{
			gm::gmAddress requestAssets = g_Scanner.ScanPattern("CModelInfo::RequestAssets",
				"8B 01 B9 FF FF 00 00 44 8B C2 23 C1 3B C1 75 07");
			g_StreamingMgr = requestAssets
				.GetAt(0x33 + 0x3)
				.GetRef()
				.GetAt(0x1B8)
				.Cast<rage::strStreamingModuleMgr*>();

			g_TxdStore = g_StreamingMgr->GetStreamingModule<rage::TxdStore>(rage::STORE_TXD);
			g_DrawableStore = g_StreamingMgr->GetStreamingModule<rage::DrawableStore>(rage::STORE_DRAWABLE);
			g_FragmentStore = g_StreamingMgr->GetStreamingModule<rage::FragmentStore>(rage::STORE_FRAGMENTS);

			g_Log.LogT("-- STREAMING MODULES --");
			g_Log.LogT("g_strStreamingModuleMgr found at: {:X}", reinterpret_cast<uintptr_t>(g_StreamingMgr));
			g_Log.LogT("g_TxdStore found at: {:X}", reinterpret_cast<uintptr_t>(g_TxdStore));
			g_Log.LogT("g_DrawableStore found at: {:X}", reinterpret_cast<uintptr_t>(g_DrawableStore));
			g_Log.LogT("g_FragmentStore found at: {:X}", reinterpret_cast<uintptr_t>(g_FragmentStore));
		}
	};

	inline strStreamingModuleMgr g_strStreamingModuleMgr;
}
