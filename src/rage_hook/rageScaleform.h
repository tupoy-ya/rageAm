#pragma once

namespace rh
{
	class CScaleformMgr
	{
	public:
		CScaleformMgr()
		{
			gm::gmAddress addr = gm::Scan("GameBacktraceConfig::WriteDebugState",
				"48 8B C4 48 89 58 08 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 90 48 81 EC 80");

		/*	gPtr_MovieStore = g_Hook.FindOffset<MovieStore*>("WriteDebugState_ScaleformMovieStore", writeDebugState + 0x1091 + 0x3);
			gPtr_lastScaleformMovie = g_Hook.FindOffset<intptr_t>("WriteDebugState_lastScaleformMovie", writeDebugState + 0x100F + 0x3);
			gPtr_lastActionScriptMethod = g_Hook.FindOffset<intptr_t>("WriteDebugState_lastActionScriptMethod", writeDebugState + 0x1022 + 0x3);
			gPtr_lastActionScriptMethodParams = g_Hook.FindOffset<intptr_t>("WriteDebugState_lastActionScriptMethodParams", writeDebugState + 0x1035 + 0x3);*/

		}
	};
	inline CScaleformMgr g_CScaleformMgr;
}
