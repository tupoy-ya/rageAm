#pragma once
#include "parStructure.h"
#include "../Vec3V.h"

namespace rage::par
{
	struct CHandlingObject
	{
		CHandlingObject() = default;
		virtual ~CHandlingObject() = default;

		virtual parStructure* GetParser()
		{
			return nullptr;
		}
	};

	enum eSubHandlingType
	{
		SUB_HANDLING_BIKE = 0,
		SUB_HANDLING_BOAT = 3,
		SUB_HANDLING_SEA_PLANE = 4,
		SUB_HANDLING_SUBMARINE = 5,
		SUB_HANDLING_CAR = 8,
		SUB_HANDLING_VEHICLE_WEAPON = 9,
		SUB_HANDLING_SPECIAL_FLIGHT = 10
	};

	struct CBaseSubHandlingData : CHandlingObject
	{
		virtual int GetTypeID() { return 0; }
		virtual void Function0x18() { return; }

		// TODO: Not fully documented (CFlyingHandlingData, TrailerHandlingData are weird)
		eSubHandlingType GetType()
		{
			return static_cast<eSubHandlingType>(GetTypeID());
		}
	};

	struct CSpecialFlightHandlingData : CBaseSubHandlingData
	{
		uint8_t pad0[8];
		Vec3V vecAngularDamping;
		Vec3V vecAngularDampingMin;
		Vec3V vecLinearDamping;
		Vec3V vecLinearDampingMin;
		float fLiftCoefficient;
		float fCriticalLiftAngle;
		float fInitialLiftAngle;
		float fMaxLiftAngle;
		float fDragCoefficient;
		float fBrakingDrag;
		float fMaxLiftVelocity;
		float fMinLiftVelocity;
		float fRollTorqueScale;
		float fMaxTorqueVelocity;
		float fMinTorqueVelocity;
		float fYawTorqueScale;
		float fSelfLevelingPitchTorqueScale;
		float fInitalOverheadAssist;
		float fMaxPitchTorque;
		float fMaxSteeringRollTorque;
		float fPitchTorqueScale;
		float fSteeringTorqueScale;
		float fMaxThrust;
		float fTransitionDuration;
		float fHoverVelocityScale;
		float fStabilityAssist;
		float fMinSpeedForThrustFalloff;
		float fBrakingThrustScale;
		int mode;
		int8_t strFlags[12]; // TODO: Actual AtFinalHashString

		CSpecialFlightHandlingData() = default;
	};
	static_assert(sizeof(CSpecialFlightHandlingData) == 0xC0);
}
