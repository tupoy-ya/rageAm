#include "gamehooks.h"

#include "memory/hook.h"

#include "hooks/errordialog.h"
#include "hooks/antidebug.h"
#include "hooks/gameviewport.h"
#include "hooks/input.h"
#include "hooks/streaming.h"

void GameHooks::Init()
{
	Hook::Init();

	//hooks::ErrorDialog::Init();
	//hooks::AntiDebug::Init();
	//hooks::GameViewport::Init();
	//hooks::Input::Init();

	hooks::Streaming::Init();
}

void GameHooks::Shutdown()
{
	// TODO: We can't call it from here
	// hooks::GameViewport::Shutdown();

	Hook::Shutdown();
}

void GameHooks::EnableAll()
{
	Hook::Seal();
}
