#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <format>
#include "Memory/Hooking.h"
#include "Component.h"
#include "Logger.h"

class GtaCommon : public Component
{
public:
	intptr_t gPtr_CreateGameWindowAndGraphics;

	GtaCommon()
	{
		gPtr_CreateGameWindowAndGraphics = g_hook->FindPattern("CreateGameWindowAndGraphics", "48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 48");
	}
};
