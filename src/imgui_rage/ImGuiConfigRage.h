#pragma once

#include "../Logger.h"
#include "ImGuiRage.h"

static void throw_on_assert(bool ok, const char* msg) {
	if (!ok) {
		g_Log.LogE("An error occurred in ImGui: {}", msg);
		g_ImGui.Shutdown();
	}
}

#define IM_ASSERT(_EXPR)  throw_on_assert((_EXPR), #_EXPR)
