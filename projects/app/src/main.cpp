// ReSharper disable CppUnusedIncludeDirective

#ifndef AM_STANDALONE
#define AM_INTEGRATED
#endif

#include "rage/system/new.h"

#include "am/manifest.h"
#include "am/asset/factory.h"
#include "am/asset/types/txd.h"
#include "am/system/system.h"
#include "am/system/cli.h"
#include "helpers/compiler.h"

#ifdef AM_STANDALONE
namespace cli
{
	void Compile(ConstWString projectDir)
	{
		rageam::asset::AssetPtr asset = rageam::asset::AssetFactory::LoadFromPath(projectDir);
		if (!asset)
			return;

		asset->CompileToFile();
	}
}

void ParseAndExecuteArguments(int argc, wchar_t** argv)
{
	enum State
	{
		STATE_LOOP,
		STATE_BUILDING,
	};
	State state = STATE_LOOP;

	rageam::ConsoleArguments args(argv, argc);
	while (args.Next())
	{
		if (args.Current() == L"--help")
		{
			AM_TRACEF("--help");
			AM_TRACEF("-b, --build\t\tCompiles assets passed in the next arguments.");
			continue;
		}

		if (args.Current() == L"--build" || args.Current() == L"-b")
		{
			state = STATE_BUILDING;
			continue;
		}

		if (state == STATE_BUILDING)
		{
			if (args.Current().StartsWith('-'))
			{
				state = STATE_LOOP;
				args.GoBack();
				continue;
			}

			cli::Compile(args.Current());
		}
	}
}

int wmain(int argc, wchar_t** argv)
{
	rageam::System system;
	system.InitCore();

	// We either execute CLI or UI mode depending on if there's any argument was passed
	if (argc > 1) // First argument is always executable path
	{
		system.InitRender(false);
		system.Finalize();

		ParseAndExecuteArguments(argc, argv);
	}
	else
	{
		system.InitRender(true);
		system.InitUI();
		system.Finalize();
		system.EnterWindowUpdateLoop();
	}
}
#else
static rageam::System s_System;
AM_EXPORT void Init()
{
	s_System.InitCore();
	s_System.InitRender(true);
	s_System.InitUI();
	s_System.Finalize();
}

AM_EXPORT void Shutdown()
{
	s_System.Destroy();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	// We don't do anything in here, nothing must be done here.
	// Launcher will call ::Init / ::Shutdown once DLL is loaded.
	
	s_System.InitCore();
	s_System.InitRender(true);
	s_System.InitUI();
	s_System.Finalize();

	return TRUE;
}
#endif
