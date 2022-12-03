#pragma once
#include "ImGuiGta.h"
#include <cassert>

// TODO: Doesn't work

#define IM_ASSERT(_EXPR) { \
    /* Disable ImGui so it won't spam with messages */ \
	g_imgui->Destroy(); \
	/* assert(_EXPR); */ \
} 
