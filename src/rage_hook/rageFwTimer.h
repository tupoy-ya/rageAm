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
			gm::gmAddress addr = g_Scanner.ScanPattern("GameBacktraceConfig::WriteCrashContextToFile",
				"48 83 EC 48 48 83 64 24 30 00 83 64 24 28 00 45");

			addr = addr.GetAt(0x179 + 0x2).GetRef();

			gPtr_bGamePause = addr.CastAt<bool*>(-0x1);
			gPtr_bUserPause = addr.CastAt<bool*>(0x0);
			gPtr_bScriptPause = addr.CastAt<bool*>(0x1);
			gPtr_bSystemPause = addr.CastAt<bool*>(0x2);
		}
	};

	inline fwTimer g_FwTimer;
}
