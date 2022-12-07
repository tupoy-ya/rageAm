#pragma once

struct CApp;

namespace rh
{
	class fwTimer
	{
		bool* gPtr_bGamePause;
		bool* gPtr_bUserPause;
		bool* gPtr_bScriptPause;
		bool* gPtr_bSystemPause;
	public:
		fwTimer()
		{
			gm::gmAddress addr = g_Scanner.ScanPattern("fwTimer::InitLevel",
				"33 C0 C7 05 ?? ?? ?? ?? ?? ?? ?? ?? C7 05 ?? ?? ?? ?? ?? ?? ?? ?? C7");

			addr = addr.GetAt(0x30 + 0x2).GetRef();

			g_Log.LogD("fwTimer: {:X}", addr.GetAddress());

			gPtr_bGamePause = addr.CastAt<bool*>(0x0);
			gPtr_bUserPause = addr.CastAt<bool*>(0x1);
			gPtr_bScriptPause = addr.CastAt<bool*>(0x2);
			gPtr_bSystemPause = addr.CastAt<bool*>(0x3);
		}
	};

	inline fwTimer g_FwTimer;
}
