#include "MissionManager.h"

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
	uint8_t stageID = Common::GetCurrentStageID() & 0xFF;
	bool fixMission = stageID <= 0x11 && Common::IsCurrentStageMission();
	if (fixMission)
	{
		stageName = *(char**)(4 * stageID + 0x1E66B48);
		stageName[5] = '0' + ((Common::GetCurrentStageID() & 0xFF00) >> 8);
	}

	int result = originalMission_CStateGoalFadeBefore(This);
	{
		if (fixMission)
		{
			// Fix result camera
			*(float*)0x1A48C80 -= 1.655f;

			// Restore getScoreTimeTable code
			stageName[5] = '0';
		}
	}
	return result;
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
	}

	originalMission_CObjMsnNumberDashRing_MsgHitEventCollision(This, Edx, message);
}

void MissionManager::applyPatches()
{
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

	// Don't apply impluse on CObjMsnNumberDashRing
	WRITE_NOP(0xEDB694, 11);
	INSTALL_HOOK(Mission_CObjMsnNumberDashRing_MsgHitEventCollision);
}
