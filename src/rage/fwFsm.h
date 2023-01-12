#pragma once
#include "Hooking.h"

#include "../Component.h"
#include <format>

namespace rage
{
	struct fwFsm;

	struct fwFsm_members
	{
		float float0;
		float float4;
		__int16 short8;
		__int16 shortA;
		__int16 shortC;
		__int16 shortE;
		__int8 gap10[6];
		__int8 byte16;
	};

	struct fwFsm_vftable
	{
		__int64(__thiscall* unk0)(fwFsm* inst);
		int(__thiscall* unk8)(fwFsm* inst, __int16 a2, __int32 a3, __int32 a4);
	};

	// Finite State Machine
	struct fwFsm
	{
		fwFsm_vftable* vftable;
		fwFsm_members members;
	};

	typedef int(*gDef_fwFsm_Update)(fwFsm* inst);

	int aImpl_fwFsm_Update(fwFsm* inst)
	{
		g_logger->Log(std::format("[fwFsm::Update] - {:X}", (intptr_t)inst));

		unsigned int v2; // ecx
		__int16 short8; // dx
		__int16 shortA; // ax
		unsigned int v5; // eax
		char shortE; // al
		__int64 v7; // rdx
		__int64 v8; // r9
		__int16 v9; // dx
		// 7FF7217AEEFC is near CGameScriptHandler
		// value of it is actually updated by CApp::Update
		// value is 0.067

		//float dword_7FF7217AEEFC = 0.067; // TEMP
		float dword_7FF7217AEEFC = *(float*)0x7FF6A004EEFC;

		inst->members.float4 = dword_7FF7217AEEFC + inst->members.float4;
		inst->members.float0 = dword_7FF7217AEEFC + inst->members.float0;
		v2 = 0;
		while(true)
		{
			short8 = inst->members.short8;
			shortA = inst->members.shortA;

			if (short8 != shortA)
			{
				inst->members.short8 = shortA;
				v2 = (inst->vftable->unk8)(inst, short8, 0xFFFFFFFF, 2i64);// 0xFFFFFFFF is (int32) -1
				inst->members.shortE |= 2u;
				inst->members.float0 = 0.0;
			}
			else if (inst->members.shortC == 65535)
			{
				shortE = inst->members.shortE;
				inst->members.shortE &= ~2u;
				if ((shortE & 2) != 0)
				{
					v7 = inst->members.short8;
					v8 = 0i64;
				}
				else
				{
					v9 = inst->members.short8;
					if (v9 != inst->members.shortA)
						continue;
					v7 = v9;
					v8 = 1i64;
				}
				v2 = (inst->vftable->unk8)(inst, v7, 4294967295i64, v8);
			}
			else
			{
				v2 = (inst->vftable->unk8)(inst, short8, inst->members.shortC, 3i64);
				if (!v2)
					v2 = (inst->vftable->unk8)(inst, 4294967295i64, inst->members.shortC, 3i64);
				inst->members.shortC = 65535;

				g_logger->Log(std::format("[fwFsm::Update_v2] - {}", v2));

				if (v2 != 0)
				{
					return v2;
				}
			}
		}
	}

	void SetHooks()
	{
		//intptr_t gPtr_fwFsm_Update = g_Hook.FindPattern("fwFsm::Update", "10 57 48 83 EC 20 F3 0F 10 05 ?? ?? ?? ?? 48 8B D9 F3 0F 58 41 0C") - 0x9;
		//g_Hook.SetHook((LPVOID)gPtr_fwFsm_Update, aImpl_fwFsm_Update);
	}

	//class hook_fwFsm : public Component
	//{
	//
	//
	//public:
	//	fwFsm()
	//	{
	//		//g_Hook.FindPattern()
	//	}
	//};

	//class fwApp : public fwFsm
	//{
	//private:
	//	int OnStateChanged(int a2, __int64 a3, int a4) override
	//	{
	//		if (a2 >= 0)
	//		{
	//			switch (a2)
	//			{
	//			case 0:
	//				if (a4 == 1)
	//					return InitializeGame(this);
	//				return 0i64;
	//			case 1:
	//				if (a4 == 1)
	//					return _callsInitiaizeGameComponents(this);
	//				return 0i64;
	//			case 2:
	//				if (!a4)
	//					return sub_7FF71E8D0A40(this);
	//				if (a4 == 1)
	//					return sub_7FF71E8D08AC();
	//				return 0i64;
	//			case 3:
	//				if (a4 == 1)
	//					return sub_7FF71E8D599C(this);
	//				return 0i64;
	//			}
	//			if (a2 != 4)
	//				return 2i64;
	//			if (a4 == 1)
	//			{
	//				sub_7FF71F0AA36C();
	//				return 2i64;
	//			}
	//		}
	//		return 0i64;
	//	}
	//};

	//class CApp : public fwApp
	//{
	//private:
	//};
}
