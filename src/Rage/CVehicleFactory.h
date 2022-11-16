#include "../Logger.h"
#include "../Memory/Hooking.h"

namespace rage
{
	// Rage Types
	typedef uint32_t u32;

	// TODO: Hook CreateFactories

	typedef intptr_t CVehicle;
	typedef intptr_t CVehicleFactory;
	typedef intptr_t CVehicleModelInfo;

	typedef CVehicle* (*gDef_CVehicleFactory_Spawn)(
		CVehicleFactory* inst,
		int* a2,
		unsigned int a3,
		unsigned int a4,
		int* a5,
		char a6,
		char a7);

	typedef CVehicleModelInfo* (*gDef_GetModelInfo)(int* hash);

	gDef_CVehicleFactory_Spawn gImpl_CVehicleFactory_Spawn;

	CVehicle* aImpl_CVehicleFactory_Spawn(
		CVehicleFactory* inst,
		int* a2,
		unsigned int a3,
		unsigned int a4,
		int* a5,
		char a6,
		char a7)
	{
		//g_logger->Log(std::format("[CVehicleFactory::Spawn] - {:x} {:x} {} {} {:x} {} {}", (intptr_t)inst, (intptr_t)a2, a3, a4, (intptr_t)a5, a6, a7));

		short v11 = *a2;
		int v12 = *a2;
		int v54 = LOWORD(v11);
		v54 = ((((v54 ^ v12) & 0xFFF0000 ^ v54) & 0xDFFFFFFF ^ v12) & 0x10000000 ^ ((v54 ^ v12) & 0xFFF0000 ^ v54) & 0xDFFFFFFF) & 0x3FFFFFFF;// flags?

		//g_logger->Log(std::format("v11: {:x} v12: {:x} v54: {:x}", v11, v12, 54));

		CVehicleModelInfo* modelInfo = ((gDef_GetModelInfo)(0x7FF69D161E58))(&v54);

		CVehicle* veh = gImpl_CVehicleFactory_Spawn(
			inst,
			a2,
			a3,
			a4,
			a5,
			a6,
			a7);
		return veh;
	}

	intptr_t gPtr_CVehicleFactory;
	intptr_t gPtr_CPedFactory;

	typedef int (*gDef_CreateVehicleCommand)(uint ModelHashKey, float* VecCoors, float fVehicleHeading, bool RegisterAsNetworkObject, bool ScriptHostObject, bool bIgnoreGroundCheck);

	gDef_CreateVehicleCommand gImpl_CreateVehicleCommand;

	int aImpl_CreateVehicleCommand(uint ModelHashKey, float* VecCoors, float fVehicleHeading, bool RegisterAsNetworkObject, bool ScriptHostObject, bool bIgnoreGroundCheck)
	{
		int hVehicle = gImpl_CreateVehicleCommand(ModelHashKey, VecCoors, fVehicleHeading, RegisterAsNetworkObject, ScriptHostObject, bIgnoreGroundCheck);

		g_logger->Log(std::format("[CreateVehicleCommand] - {}, {:X} {} {} {} {} {} {} {}",
			hVehicle, ModelHashKey, VecCoors[0], VecCoors[2], VecCoors[4], fVehicleHeading, 
			RegisterAsNetworkObject, ScriptHostObject, bIgnoreGroundCheck));

		return hVehicle;
	}

	void HookFactories()
	{
		intptr_t gPtrCreateVehicleCommand = g_hook->FindPattern("CreateVehicleCommand", "48 89 5C 24 08 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 50 F3");
		g_hook->SetHook(gPtrCreateVehicleCommand, aImpl_CreateVehicleCommand, &gImpl_CreateVehicleCommand);

		intptr_t vehFactory = g_hook->FindPattern("CreateVehicleFactory", "48 83 EC 28 B9 08 00 00 00 E8 ?? ?? ?? ?? 48 85 C0 74 11 48");
		gPtr_CVehicleFactory = g_hook->FindOffset("CreateVehicleFactory_g_vehicleFactory", vehFactory + 0x1b + 0x3);

		intptr_t pedFactory = g_hook->FindPattern("CreatePedFactory", "48 83 EC 28 B9 10 00 00 00 E8 ?? ?? ?? ?? 48 85 C0 74 11 48 8B");
		gPtr_CPedFactory = g_hook->FindOffset("CreatePedFactory_g_pedFactory", pedFactory + 0x1b + 0x3);

		intptr_t gPtrVehicleFactorySpawn = g_hook->FindPattern("CVehicleFactory::Spawn", "48 8B C4 48 89 58 08 48 89 70 18 48 89 78 20 55 41 54 41 55 41 56 41 57 48 8D 68 B9");
		g_hook->SetHook(gPtrVehicleFactorySpawn, aImpl_CVehicleFactory_Spawn, &gImpl_CVehicleFactory_Spawn);
	}
}