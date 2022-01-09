#include "MissionManager.h"
#include "ScoreManager.h"

FUNCTION_PTR(void*, __stdcall, Mission_fpEventTrigger, 0xD5ED00, void* This, int Event);

//---------------------------------------------------
// Mission Gameplay HUD
//---------------------------------------------------
HOOK(int*, __fastcall, Mission_GameplayManagerInit, 0xD00F70, void* This, void* Edx, int* a2)
{
	static char const* scoreHUD = "ui_gameplay_score";
	static char const* defaultHUD = (char*)0x0168E328;

	if (Common::IsCurrentStageMission())
	{
		// Use HUD with moved life icon
		WRITE_MEMORY(0x109D669, char*, scoreHUD);
	}
	else
	{
		// Original code
		WRITE_MEMORY(0x109D669, char*, defaultHUD);
	}

	return originalMission_GameplayManagerInit(This, Edx, a2);
}

//---------------------------------------------------
// Mission Complete Result HUD
//---------------------------------------------------
HOOK(void, __fastcall, Mission_CMissionManagerAdvance, 0xD10690, uint32_t* This, void* Edx, float* a2)
{
	uint32_t state = This[46];
	originalMission_CMissionManagerAdvance(This, Edx, a2);

	// Mission success
	if (This[46] == 3 && state == 1)
	{
		WRITE_MEMORY(0x10951F5, uint8_t, 0xEB);
	}
}

HOOK(void, __fastcall, Mission_MsgGoalResult, 0x10950E0, uint32_t* This, void* Edx, void* message)
{
	originalMission_MsgGoalResult(This, Edx, message);

	// Set original code
	WRITE_MEMORY(0x10951F5, uint8_t, 0x77);
}

HOOK(int, __fastcall, Mission_CStateGoalFadeBefore, 0xCFE080, uint32_t* This)
{
	// Change getScoreTimeTable for missions
	static char* stageName = nullptr;
	if (Common::IsCurrentStageMission())
	{
		uint8_t stageID = Common::GetCurrentStageID() & 0xFF;
		stageName = *(char**)(4 * stageID + 0x1E66B48);
		stageName[5] = '0' + ((Common::GetCurrentStageID() & 0xFF00) >> 8);
	}

	int result = originalMission_CStateGoalFadeBefore(This);
	{
		if (Common::IsCurrentStageMission())
		{
			// Fix result camera
			float* cameraUp = (float*)0x1A48C80;
			if (*cameraUp > 0.0f)
			{
				*cameraUp -= 1.655f;
			}

			// Restore getScoreTimeTable code
			stageName[5] = '0';
		}
	}
	return result;
}

//---------------------------------------------------
// Load terrain
//---------------------------------------------------
HOOK(void, __fastcall, Mission_CGameplayFlowStageSetStageInfo, 0xCFF6A0, void* This)
{
	originalMission_CGameplayFlowStageSetStageInfo(This);

	if (Common::IsCurrentStageMission())
	{
		static std::string stageTerrain = "pam000";

		// TODO: Exceptions, missions that uses MapD instead of MapABC
		uint32_t stageID = Common::GetCurrentStageID();
		switch (stageID)
		{
		default: stageTerrain = "pam000"; break;
		}

		strcpy(Common::GetCurrentTerrain(), stageTerrain.c_str());
	}
}

//---------------------------------------------------
// CObjMsnNumberDashRing
//---------------------------------------------------
HOOK(void, __fastcall, Mission_CObjMsnNumberDashRing_MsgHitEventCollision, 0xEDB560, uint32_t This, void* Edx, void* message)
{
	if (*(uint8_t*)(This + 0x154) == 2)
	{
		static SharedPtrTypeless soundHandle;
		typedef void* __fastcall CObjMsnNumberDashRingPlaySound(uint32_t, void*, SharedPtrTypeless&, uint32_t cueId);
		CObjMsnNumberDashRingPlaySound* playSoundFunc = *(CObjMsnNumberDashRingPlaySound**)(*(uint32_t*)This + 0x40);
		playSoundFunc(This, nullptr, soundHandle, 8002040);

		// Send event
		Mission_fpEventTrigger((void*)This, 1);
	}

	originalMission_CObjMsnNumberDashRing_MsgHitEventCollision(This, Edx, message);
}

float const NumberDashRingHalfWidth = 0.3f;
float const NumberDashRingRadius = 2.5f;
void __declspec(naked) CObjMsnNumberDashRing_CreatehkpCapsuleShape()
{
	static uint32_t sub_E92B90 = 0xE92B90;
	static uint32_t returnAddress = 0x115B4AE;
	__asm
	{
		sub		esp, 20h
		lea		esi, [esp + 1Ch]

		// radius
		lea		eax, [NumberDashRingRadius]
		fld		dword ptr [eax]
		fstp	[esp + 18h]

		// vertex1 xyz
		lea		eax, [NumberDashRingHalfWidth]
		fld		dword ptr[eax]
		fchs
		fstp	[esp + 14h]
		fldz
		fst		[esp + 10h]
		fstp	[esp + 0Ch]

		// vertex2 xyz
		lea		eax, [NumberDashRingHalfWidth]
		fld		dword ptr[eax]
		fstp	[esp + 08h]
		fldz
		fst		[esp + 04h]
		fstp	[esp + 00h]

		call	[sub_E92B90]
		add     esp, 20h
		mov		eax, [eax]

		jmp		[returnAddress]
	}
}

HOOK(bool, __fastcall, Mission_CObjMsnNumberDashRing_AddEventCollision, 0x115B460, int* This, void* Edx, void* a2)
{
	if (*This == 0x16CCDEC)
	{
		// Change NumberDashRing to use hkpCylinderShape collision
		WRITE_JUMP(0x115B47C, CObjMsnNumberDashRing_CreatehkpCapsuleShape);
	}

	bool result = originalMission_CObjMsnNumberDashRing_AddEventCollision(This, Edx, a2);
	{
		// Restore code
		WRITE_MEMORY(0x115B47C, uint8_t, 0xA1, 0xE0, 0xCB, 0xCF, 0x01);
	}
	return result;
}

//---------------------------------------------------
// Goalring
//---------------------------------------------------
HOOK(void, __fastcall, Mission_CObjGoalRing_MsgHitEventCollision, 0x1159010, uint32_t This, void* Edx, void* message)
{
	if (Common::IsCurrentStageMission())
	{
		// Disable goalring sfx
		WRITE_MEMORY(0x1159054, int, -1);
	}

	originalMission_CObjGoalRing_MsgHitEventCollision(This, Edx, message);

	// Original code
	WRITE_MEMORY(0x1159054, uint32_t, 4001005);
}

void MissionManager::applyPatches()
{
	// Fix Generations HUD
	if (!ScoreManager::m_externalHUD)
	{
		INSTALL_HOOK(Mission_GameplayManagerInit);
	}

	// Use normal stage HUD (with life count)
	WRITE_MEMORY(0x109ADEB, uint8_t, 0xEB);

	// Enable life count change
	WRITE_MEMORY(0xE761ED, uint8_t, 0xEB);
	WRITE_MEMORY(0xD599CE, uint8_t, 0xEB);

	// Don't allow restart at 0 life
	WRITE_NOP(0x10A0FCA, 6);

	// Don't show mission objective at pause
	WRITE_MEMORY(0xD00A46, uint8_t, 0);
	WRITE_MEMORY(0xD07489, uint8_t, 0);

	// Mission Complete use stage result
	WRITE_MEMORY(0xD104C3, uint32_t, 2);
	INSTALL_HOOK(Mission_CMissionManagerAdvance);
	INSTALL_HOOK(Mission_MsgGoalResult);
	INSTALL_HOOK(Mission_CStateGoalFadeBefore);

	// Load correct mission terrain
	INSTALL_HOOK(Mission_CGameplayFlowStageSetStageInfo);

	//---------------------------------------------------
	// CObjMsnNumberDashRing
	//---------------------------------------------------
	// Don't apply impluse
	WRITE_NOP(0xEDB694, 11);

	// Lower transparency
	static float passRingAlpha = 0.3f;
	WRITE_MEMORY(0xEDB3A4, float*, &passRingAlpha);

	// Play sfx
	INSTALL_HOOK(Mission_CObjMsnNumberDashRing_MsgHitEventCollision);

	// Change to cylinder shape collision
	INSTALL_HOOK(Mission_CObjMsnNumberDashRing_AddEventCollision);

	// Use different model
	WRITE_MEMORY(0xEDBD8A, uint8_t, 6);
	static char const* twn_obj_passring = "twn_obj_passring";
	WRITE_MEMORY(0x1A4758C, char*, twn_obj_passring);

	//---------------------------------------------------
	// Goalring
	//---------------------------------------------------
	INSTALL_HOOK(Mission_CObjGoalRing_MsgHitEventCollision);
}
