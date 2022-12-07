#pragma once

#define BOOST_STACKTRACE_USE_BACKTRACE
#include <boost/stacktrace.hpp>
#include <strstream>

extern inline HMODULE g_RageAmHnd = nullptr;

class CrashHandler
{
	static inline PVOID ms_VectorExceptionHandlerHnd;
	static inline bool ms_hasExceptionOccurred = false;
	static inline LPTOP_LEVEL_EXCEPTION_FILTER ms_PreviousTopLevelUnhandledExceptionHandler;

	static HMODULE GetCurrentModule()
	{ // NB: XP+ solution!
		HMODULE hModule = nullptr;
		GetModuleHandleEx(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
			reinterpret_cast<LPCTSTR>(GetCurrentModule),
			&hModule);

		return hModule;
	}

	static LONG HandleRageException(EXCEPTION_POINTERS* info)
	{
		std::stringstream ss{};
		ss << boost::stacktrace::stacktrace();

		HMODULE hModule = GetCurrentModule();
		if (hModule == g_RageAmHnd)
		{
			if (!ms_hasExceptionOccurred)
			{
				g_Log.LogE("An unhandled exception occurred in rageAm: \n{}", ss.str());
				ms_hasExceptionOccurred = true;
			}

			// This probably will make every c++ developer in the world angry
			// but at least won't crash whole game and unwind stack properly
			// (hopefully)
			// There's basically 100% chance that it will get in the middle of instruction,
			// causing this handler to be called few more times...
			info->ContextRecord->Rip++;
			return EXCEPTION_CONTINUE_EXECUTION;
		}
		g_Log.LogT("An unhandled exception occurred in GTA5: \n{}", ss.str());

		return EXCEPTION_CONTINUE_SEARCH;
	}

	static LONG CALLBACK RageVectoredExceptionHandler(EXCEPTION_POINTERS* info)
	{
		return HandleRageException(info);
	}

	static LONG CALLBACK RageUnhandledExceptionFilter(EXCEPTION_POINTERS* info)
	{
		return HandleRageException(info);
	}

public:
	CrashHandler()
	{
#ifdef USE_GLOBAL_CRASH_HANDLER
		g_Log.LogT("CrashHandler()");

		ms_VectorExceptionHandlerHnd = AddVectoredExceptionHandler(true, RageVectoredExceptionHandler);
		ms_PreviousTopLevelUnhandledExceptionHandler = SetUnhandledExceptionFilter(RageUnhandledExceptionFilter);
#endif
	}

	~CrashHandler()
	{
#ifdef USE_GLOBAL_CRASH_HANDLER
		g_Log.LogT("~CrashHandler()");

		RemoveVectoredExceptionHandler(ms_VectorExceptionHandlerHnd);
		SetUnhandledExceptionFilter(ms_PreviousTopLevelUnhandledExceptionHandler);
#endif
	}
};

inline CrashHandler g_CrashHandler;
