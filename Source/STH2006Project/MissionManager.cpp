#include "MissionManager.h"
#include "ScoreManager.h"
#include "Application.h"
#include "SubtitleUI.h"

#define NPC_DATA_FILE "Assets\\Textbox\\npcData.ini"
FUNCTION_PTR(void*, __stdcall, Mission_fpEventTrigger, 0xD5ED00, void* This, int Event);

//---------------------------------------------------
// Mission Complete Result HUD
//---------------------------------------------------
bool MissionManager::m_missionAsStage = false;
HOOK(void, __fastcall, Mission_CMissionManagerAdvance, 0xD10690, uint32_t This, void* Edx, float* a2)
{
	uint32_t* pState = (uint32_t*)(This + 184);
	uint32_t state = *pState;

	float* pStateTime = (float*)(This + 188);
	static bool dialogShown = false;
	if (state == 1 || state == 2)
	{
		if (MissionManager::m_missionAsStage)
		{
			// Skip showing "You Succeed!"/"You Failed"
			WRITE_NOP(0xD104B0, 6);
			WRITE_NOP(0xD101B0, 6);
		}
		else if (*pStateTime == 0.0f)
		{
			// Prevent changing game state until we show dialog
			WRITE_MEMORY(0xD104B0, uint8_t, 0xE9, 0xCF, 0x01, 0x00, 0x00, 0x90);
			WRITE_MEMORY(0xD101B0, uint8_t, 0xE9, 0xC1, 0x02, 0x00, 0x00, 0x90);
			dialogShown = false;
		}
		else if (*pStateTime >= 1.0f)
		{
			if (!dialogShown)
			{
				// Show dialog
				MissionManager::startMissionCompleteDialog(state == 1);
				dialogShown = true;
			}
			else if (!SubtitleUI::isPlayingCaption())
			{
				// Revert code
				WRITE_MEMORY(0xD104B0, uint8_t, 0x0F, 0x82, 0xCE, 0x01, 0x00, 0x00);
				WRITE_MEMORY(0xD101B0, uint8_t, 0x0F, 0x82, 0xC0, 0x02, 0x00, 0x00);
			}
		}
	}

	originalMission_CMissionManagerAdvance(This, Edx, a2);

	// Mission success finished
	if (state == 1 && *pState == 3)
	{
		WRITE_MEMORY(0x10951F5, uint8_t, 0xEB);
	}
}

int MissionManager::getMissionDialog(std::vector<std::string>& captions, uint32_t stageID, std::string const& name, std::string* speaker)
{
	bool isJapanese = Common::GetUILanguageType() == LT_Japanese;
	std::string const currentStageStr = std::to_string(stageID);

	const INIReader reader(Application::getModDirString() + NPC_DATA_FILE);
	int dialogCount = 1;
	while (true)
	{
		std::string caption = reader.Get(currentStageStr, name + (isJapanese ? "JP" : "") + std::to_string(dialogCount), "");
		if (caption.empty())
		{
			break;
		}

		captions.push_back(caption);
		dialogCount++;
	}

	// Get speaker
	if (speaker != nullptr)
	{
		*speaker = reader.Get(currentStageStr, isJapanese ? "SpeakerJP" : "Speaker", "");
	}

	return dialogCount - 1;
}

void MissionManager::startMissionCompleteDialog(bool success)
{
	std::vector<std::string> captions;

	std::string name = "Mission";
	name += success ? "Success" : "Fail";

	std::string speaker;
	MissionManager::getMissionDialog(captions, Common::GetCurrentStageID(), name, &speaker);
	SubtitleUI::addCaption(captions, speaker);

	// Play voice
	const INIReader reader(Application::getModDirString() + NPC_DATA_FILE);
	static SharedPtrTypeless soundHandle;
	Common::PlaySoundStatic(soundHandle, reader.GetInteger(std::to_string(Common::GetCurrentStageID()), name + "Voice", -1));
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

	// Override terrain
	const INIReader reader(Application::getModDirString() + "Assets\\Title\\trialData.ini");
	uint32_t stageID = Common::GetCurrentStageID();
	std::string const stageStr = std::to_string(stageID);
	std::string stageTerrain = reader.Get(stageStr, "terrainID", "");
	if (!stageTerrain.empty())
	{
		strcpy(Common::GetCurrentTerrain(), stageTerrain.c_str());
	}

	bool isMission = Common::IsStageMission(stageID);
	if (isMission && reader.GetBoolean(stageStr, "missionAsStage", false))
	{
		// Allow dying in mission
		WRITE_MEMORY(0xD10803, uint8_t, 0x0F, 0x84, 0xF9, 0x00, 0x00, 0x00);

		// Enable life HUD
		WRITE_NOP(0x109ADF8, 6);

		// Enable life count change
		WRITE_MEMORY(0xE761ED, uint8_t, 0xEB);
		WRITE_MEMORY(0xD599CE, uint8_t, 0xEB);

		// Don't allow restart at 0 life
		//WRITE_NOP(0x10A0FCA, 6);

		MissionManager::m_missionAsStage = true;
	}
	else
	{
		// Always fail mission if you die (since there's no checkpoint)
		WRITE_MEMORY(0xD10803, uint8_t, 0xE9, 0xFA, 0x00, 0x00, 0x00, 0x90);

		// Revert life codes
		WRITE_MEMORY(0x109ADF8, uint8_t, 0x21, 0xBD, 0x5C, 0x02, 0x00, 0x00);
		WRITE_MEMORY(0xE761ED, uint8_t, 0x77);
		WRITE_MEMORY(0xD599CE, uint8_t, 0x7E);
		//WRITE_MEMORY(0x10A0FCA, uint8_t, 0x0F, 0x85, 0x61, 0x01, 0x00, 0x00);

		MissionManager::m_missionAsStage = false;
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
float const NumberDashRingRadius = 2.3f;
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
	if (Common::IsCurrentStageMission() && !MissionManager::m_missionAsStage)
	{
		// Disable goalring sfx
		WRITE_MEMORY(0x1159054, int, -1);
	}

	originalMission_CObjGoalRing_MsgHitEventCollision(This, Edx, message);

	// Original code
	WRITE_MEMORY(0x1159054, uint32_t, 4001005);
}

//---------------------------------------------------
// HUB NPC Talk
//---------------------------------------------------
bool MissionManager::m_missionAccept = true;
HOOK(void, __fastcall, Mission_CHudGateMenuMain_CStateLoadingBegin, 0x107D790, uint32_t** This)
{
	uint32_t gateStageID = This[2][84];
	uint32_t gateMissionID = This[2][85];

	if (gateMissionID > 0 && gateStageID <= SMT_pla200)
	{
		MissionManager::m_missionAccept = true;

		std::vector<std::string> captions;
		uint32_t stageID = (gateMissionID << 8) + gateStageID;
		std::string speaker;
		MissionManager::getMissionDialog(captions, stageID, "MissionStart", &speaker);

		// Get accept & reject dialog size
		int acceptSize = -1;
		int rejectSize = -1;
		if (gateMissionID <= 5)
		{
			acceptSize = MissionManager::getMissionDialog(captions, stageID, "MissionAccept");
			rejectSize = MissionManager::getMissionDialog(captions, stageID, "MissionReject");
		}
		SubtitleUI::addCaption(captions, speaker, acceptSize, rejectSize);

		// Play voice
		const INIReader reader(Application::getModDirString() + NPC_DATA_FILE);
		static SharedPtrTypeless soundHandle;
		Common::PlaySoundStatic(soundHandle, reader.GetInteger(std::to_string(stageID), "MissionStartVoice", -1));
	}

	originalMission_CHudGateMenuMain_CStateLoadingBegin(This);
}

HOOK(void, __fastcall, Mission_CHudGateMenuMain_CStateLoadingAdvance, 0x10804C0, uint32_t** This)
{
	if (!SubtitleUI::isPlayingCaption())
	{
		originalMission_CHudGateMenuMain_CStateLoadingAdvance(This);
	}
}

HOOK(void, __fastcall, Mission_CHudGateMenuMain_CStateIntroBegin, 0x1080110, uint32_t** This)
{
	uint32_t gateStageID = This[2][84];
	uint32_t gateMissionID = This[2][85];

	if (gateMissionID > 0 && gateStageID <= SMT_pla200)
	{
		if (gateMissionID  <= 5 && MissionManager::m_missionAccept)
		{
			FUNCTION_PTR(void, __cdecl, Mission_fpEnterStage, 0xD401F0, uint32_t* This, uint32_t nStageID, uint32_t nMissionID);
			Mission_fpEnterStage(This[2], gateStageID, gateMissionID);
		}

		// Kill state machine
		*(bool*)((uint32_t)This + 0x1C) = 1;
	}
	else
	{
		originalMission_CHudGateMenuMain_CStateIntroBegin(This);
	}
}

void MissionManager::applyPatches()
{
	// Disable rank display in missions
	WRITE_NOP(0x109ADFE, 0xA);

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

	// Change press X to interact stage gate
	WRITE_MEMORY(0xD315F4, Sonic::EKeyState, Sonic::EKeyState::eKeyState_X);

	// MissionGate will use "Talk" UI popup instead
	WRITE_MEMORY(0xEEAF15, uint32_t, 0);

	// Don't disable item in missions
	WRITE_NOP(0xFFF531, 6);

	// Don't stop camera after mission finishes
	WRITE_JUMP(0xD0FFDF, (void*)0xD1016C); // success
	WRITE_JUMP(0xD0F9A4, (void*)0xD0FB31); // fail

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

	//---------------------------------------------------
	// HUB NPC Talk
	//---------------------------------------------------
	INSTALL_HOOK(Mission_CHudGateMenuMain_CStateLoadingBegin);
	INSTALL_HOOK(Mission_CHudGateMenuMain_CStateLoadingAdvance);
	INSTALL_HOOK(Mission_CHudGateMenuMain_CStateIntroBegin);

	// Disable MissionGate bell
	WRITE_MEMORY(0xEEB4FD, uint8_t, 0xE9, 0x0A, 0x02, 0x00, 0x00, 0x90);
	WRITE_MEMORY(0xEEC5CE, uint8_t, 0xE9, 0xD5, 0x02, 0x00, 0x00, 0x90);

	// Disable MissionGate Icon
	WRITE_JUMP(0xEEBA3C, (void*)0xEEBAFC);
	WRITE_MEMORY(0xEEBCBC, uint8_t, 0xE9, 0x25, 0x01, 0x00, 0x00, 0x90);

	// Don't clamp MissionID param
	WRITE_NOP(0xEEAED4, 0x14);

	// Force invalid MissionID to allow interact
	WRITE_MEMORY(0xEEB17C, uint8_t, 0xB0, 0x01);
	WRITE_MEMORY(0x552145, uint8_t, 0xEB); // ID = 9 is hardcoded for something...?
}
