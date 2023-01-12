#pragma once

#define NOINLINE __declspec(noinline)
#include "ImGuiRage.h"

class ImGuiCrashHandler
{
public:
	NOINLINE static void ImGuiHandleAssert(const char* msg);
};

NOINLINE static void throw_on_assert(bool ok, const char* msg) {  // NOLINT(clang-diagnostic-unused-function)
	if (ok)
		return;

	g_ImGui.Shutdown();
	ImGuiCrashHandler::ImGuiHandleAssert(msg);

	// We override abort behaviour meaning we still have to stop execution somehow.
	// All ImGui code is executed in try catch (ImGuiAppMgr.h), simply throwing exception and ignoring it should work.
	throw;
}

#define IM_ASSERT(_EXPR)  throw_on_assert((_EXPR), #_EXPR)  // NOLINT(clang-diagnostic-unused-macros)
