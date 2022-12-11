#pragma once
#include "../rage/parsable/CBaseSubHandlingData.h"

namespace rh
{
	class CHandling
	{
		typedef rage::par::CHandlingObject* (*gDef_GetSubHandlingByType)(uintptr_t inst, int type);

		static inline gDef_GetSubHandlingByType gImpl_GetSubHandlingByType;

		static rage::par::CHandlingObject* aImpl_GetSubHandlingByType(uintptr_t inst, int type)
		{
			if (type == rage::par::SUB_HANDLING_SPECIAL_FLIGHT)
			{
				return &m_CustomFlightData;
			}
			return gImpl_GetSubHandlingByType(inst, type);
		}

		static inline rage::par::CSpecialFlightHandlingData m_CustomFlightData{};
	public:
		CHandling()
		{
			// From handling.meta
			m_CustomFlightData.mode = 1;
			m_CustomFlightData.fLiftCoefficient = 50.0f;
			m_CustomFlightData.fMinLiftVelocity = 0.0f;
			m_CustomFlightData.fDragCoefficient = 0.0f;
			m_CustomFlightData.fMaxPitchTorque = 500.0f;
			m_CustomFlightData.fMaxSteeringRollTorque = 50.0f;
			m_CustomFlightData.fMaxThrust = 20.0f;
			m_CustomFlightData.fYawTorqueScale = -4.0f;
			m_CustomFlightData.fRollTorqueScale = 7.5f;
			m_CustomFlightData.fTransitionDuration = 1.0f;
			m_CustomFlightData.fPitchTorqueScale = 8.0f;
			m_CustomFlightData.vecAngularDamping = Vec3V(3.0f, 2.0f, 1.2f, 3.0f); // W component repeats X
			m_CustomFlightData.vecLinearDamping = Vec3V(0.9f, 0.1f, 0.7f, 0.9f); // W component repeats X

			// From in game (defaults)
			m_CustomFlightData.fCriticalLiftAngle = 45.0f;
			m_CustomFlightData.fInitialLiftAngle = 1.5f;
			m_CustomFlightData.fMaxLiftAngle = 25.0f;
			m_CustomFlightData.fBrakingDrag = 10.0f;
			m_CustomFlightData.fMaxLiftVelocity = 2000.0f;
			m_CustomFlightData.fRollTorqueScale = 7.5f;
			m_CustomFlightData.fMaxTorqueVelocity = 100.0f;
			m_CustomFlightData.fMinTorqueVelocity = 40000.0f;
			m_CustomFlightData.fYawTorqueScale = -4.0f;
			m_CustomFlightData.fSelfLevelingPitchTorqueScale = -5.0f;
			m_CustomFlightData.fInitalOverheadAssist = -5.0f;
			m_CustomFlightData.fSteeringTorqueScale = 1000.0f;
			m_CustomFlightData.fMaxThrust = 20.0f;
			m_CustomFlightData.fHoverVelocityScale = 1.0f;
			m_CustomFlightData.fStabilityAssist = 10.0f;
			m_CustomFlightData.fMinSpeedForThrustFalloff = 0.0f;
			m_CustomFlightData.fBrakingThrustScale = 0.0f;

			// Temp hack to install game vftable
			*reinterpret_cast<uintptr_t*>(&m_CustomFlightData) = 0x7FF6E0EBAAB8;

			gm::gmAddress addr = g_Scanner.ScanPattern("CHandling::GetSubHandlingByType", "E8 ?? ?? ?? ?? 66 3B 70 48");
			addr = addr.GetAt(0x1).GetRef(); // Function call
			g_Hook.SetHook(addr, aImpl_GetSubHandlingByType, &gImpl_GetSubHandlingByType);

			g_Log.LogT("CHandling -> m_CustomFlightData: {:X}", reinterpret_cast<uintptr_t>(&m_CustomFlightData));
		}

	};

	inline CHandling g_Handling;
}
