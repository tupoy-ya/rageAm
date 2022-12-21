#pragma once

namespace gm
{
	/**
	 * \brief Swaps object's vftable pointer.
	 */
	class VftSwap
	{
		/**
		 * \brief Pointer on object that vftable was swapped.
		 */
		void*** m_ToSwap;

		/**
		 * \brief Pointer on original object vftable.
		 */
		void** m_Backup;
	public:
		VftSwap(LPVOID toSwap, LPVOID toUse)
		{
			g_Log.LogT("VftSwap() -> {:X}, {:X}", (uintptr_t)toSwap, (uintptr_t)toUse);

			m_ToSwap = (void***)toSwap;
			m_Backup = *(void***)toSwap;
			*(void***)toSwap = *(void***)toUse;
		}

		~VftSwap()
		{
			g_Log.LogT("~VftSwap()");

			*m_ToSwap = m_Backup;
		}
	};
}
