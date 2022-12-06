#pragma once
#include "../memory/gmHelper.h"

namespace rh
{
	class GameInput
	{
		typedef void(*DisableAllControlActionsCommand)(int padIndex);

		static inline DisableAllControlActionsCommand gImpl_DisableAllControlActionsCommand;
	public:
		GameInput()
		{
			gm::ScanAndSet("DisableAllControlActionsCommand", "40 53 48 83 EC 20 33 DB 85 C9 75 09",
				&gImpl_DisableAllControlActionsCommand);
		}

		static void DisableAllControlsThisFrame()
		{
			gImpl_DisableAllControlActionsCommand(0);
		}
	};

	inline GameInput g_GameInput;
}
