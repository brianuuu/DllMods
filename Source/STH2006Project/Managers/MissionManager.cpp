#include "MissionManager.h"

#include "System/Application.h"
#include "UI/SubtitleUI.h"

#include "Objects/mission/Pele.h"
#include "Objects/mission/SoleannaBoys.h"
#include "Objects/mission/WhosCaptain.h"
#include "Objects/mission/SonicMan.h"

#define NPC_DATA_FILE "Assets\\Database\\npcData.ini"

//---------------------------------------------------
// Mission Complete Result HUD
//---------------------------------------------------
bool m_missionFailed = false;
void MissionManager::setMissionFailed()
{
	m_missionFailed = true;
}

bool MissionManager::m_missionAsStage = false;
HOOK(void, __fastcall, Mission_CMissionManagerAdvance, 0xD10690, uint32_t This, void* Edx, float* a2)
{
	uint32_t* pState = (uint32_t*)(This + 184);
	uint32_t state = *pState;

	if (state == 0 && m_missionFailed)
	{
		m_missionFailed = false;
		struct MsgMissionFailed
		{
			INSERT_PADDING(0x10);
			uint32_t m_type;
		};
		alignas(16) MsgMissionFailed msgMissionFailed {};
		msgMissionFailed.m_type = 3;

		FUNCTION_PTR(void*, __thiscall, processMsgMissionFailed, 0xD10920, void* This, void* message);
		processMsgMissionFailed((void*)This, &msgMissionFailed);

		state = *pState;
	}

	float* pStateTime = (float*)(This + 188);
	static bool dialogShown = false;
	static bool dialogFinished = false;
	if (state == 1 || state == 2)
	{
		if (MissionManager::m_missionAsStage)
		{
			if (Sonic::Player::CPlayerSpeedContext::GetInstance()->m_Grounded && *pStateTime >= 0.6f || *pStateTime >= 1.0f)
			{
				// Change state immediately
				WRITE_NOP(0xD104B0, 6);
				WRITE_NOP(0xD101B0, 6);
			}
			else
			{
				// Prevent changing game state until we show dialog
				WRITE_MEMORY(0xD104B0, uint8_t, 0xE9, 0xCF, 0x01, 0x00, 0x00, 0x90);
				WRITE_MEMORY(0xD101B0, uint8_t, 0xE9, 0xC1, 0x02, 0x00, 0x00, 0x90);
			}
		}
		else
		{
			if (*pStateTime == 0.0f)
			{
				// Prevent changing game state until we show dialog
				WRITE_MEMORY(0xD104B0, uint8_t, 0xE9, 0xCF, 0x01, 0x00, 0x00, 0x90);
				WRITE_MEMORY(0xD101B0, uint8_t, 0xE9, 0xC1, 0x02, 0x00, 0x00, 0x90);
				dialogShown = false;
				dialogFinished = false;
			}
			else if (*pStateTime >= 1.0f)
			{
				if (!dialogShown)
				{
					// Show dialog
					MissionManager::startMissionCompleteDialog(state == 1);
					dialogShown = true;
				}
				else if (!SubtitleUI::isPlayingCaption() && !dialogFinished)
				{
					// Revert code
					WRITE_MEMORY(0xD104B0, uint8_t, 0x0F, 0x82, 0xCE, 0x01, 0x00, 0x00);
					WRITE_MEMORY(0xD101B0, uint8_t, 0x0F, 0x82, 0xC0, 0x02, 0x00, 0x00);

					dialogFinished = true;
				}
			}
		}
	}

	originalMission_CMissionManagerAdvance(This, Edx, a2);

	// Mission success finished
	if (state == 1 && *pState == 3)
	{
		// Use this to fix camera
		FUNCTION_PTR(int, __thiscall, CSonicContext_MsgPlayerGoal, 0xE6C2C0, void* player, int a2);
		CSonicContext_MsgPlayerGoal(Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer, 0);

		// Use normal result screen
		WRITE_MEMORY(0x10951F5, uint8_t, 0xEB);
	}
}

int MissionManager::getMissionDialog(std::vector<std::string>& captions, std::string const& section, std::string const& name, std::string* speaker)
{
	bool isJapanese = Common::GetUILanguageType() == LT_Japanese;

	const INIReader reader(Application::getModDirString() + NPC_DATA_FILE);
	int dialogCount = 1;
	while (true)
	{
		std::string caption = reader.Get(section, name + (isJapanese ? "JP" : "") + std::to_string(dialogCount), "");
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
		*speaker = reader.Get(section, isJapanese ? "SpeakerJP" : "Speaker", "");
	}

	return dialogCount - 1;
}

void MissionManager::startMissionCompleteDialog(bool success)
{
	std::vector<std::string> captions;

	std::string name = "Mission";
	name += success ? "Success" : "Fail";

	std::string speaker;
	MissionManager::getMissionDialog(captions, std::to_string(Common::GetCurrentStageID()), name, &speaker);
	SubtitleUI::addCaption(captions, speaker);

	// Play voice
	const INIReader reader(Application::getModDirString() + NPC_DATA_FILE);
	static SharedPtrTypeless soundHandle;
	Common::PlaySoundStatic(soundHandle, reader.GetInteger(std::to_string(Common::GetCurrentStageID()), name + "Voice", -1));
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
	const INIReader reader(Application::getModDirString() + "Assets\\Database\\trialData.ini");
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

		// Allow skills in mission
		WRITE_MEMORY(0xD59C89, uint8_t, 0xEB);

		// Enable life HUD
		WRITE_NOP(0x109ADF8, 6);

		// Enable life count change
		WRITE_MEMORY(0xE761ED, uint8_t, 0xEB);
		WRITE_MEMORY(0xD599CE, uint8_t, 0xEB);

		// Don't allow restart at 0 life
		//WRITE_NOP(0x10A0FCA, 6);

		// Reduce "You Succeed!"/"You Failed" time and hide them
		WRITE_MEMORY(0x168E128, float, -1.0f); // success freeze frame
		WRITE_MEMORY(0x168E134, float, -1.0f); // success freeze frame

		// Always use stage result screen
		WRITE_MEMORY(0x10951F5, uint8_t, 0xEB);

		MissionManager::m_missionAsStage = true;
	}
	else
	{
		// Always fail mission if you die (since there's no checkpoint)
		WRITE_MEMORY(0xD10803, uint8_t, 0xE9, 0xFA, 0x00, 0x00, 0x00, 0x90);

		// Disable skills in mission
		WRITE_MEMORY(0xD59C89, uint8_t, 0x77);

		// Revert life codes
		WRITE_MEMORY(0x109ADF8, uint8_t, 0x21, 0xBD, 0x5C, 0x02, 0x00, 0x00);
		WRITE_MEMORY(0xE761ED, uint8_t, 0x77);
		WRITE_MEMORY(0xD599CE, uint8_t, 0x7E);
		//WRITE_MEMORY(0x10A0FCA, uint8_t, 0x0F, 0x85, 0x61, 0x01, 0x00, 0x00);

		// Revert 2s timer
		WRITE_MEMORY(0x168E128, float, 90.0f); // success freeze frame
		WRITE_MEMORY(0x168E134, float, 90.0f); // success freeze frame

		// Revert result screen
		WRITE_MEMORY(0x10951F5, uint8_t, 0x77);

		MissionManager::m_missionAsStage = false;
	}
}

//---------------------------------------------------
// Sonic Spawn Points
//---------------------------------------------------
HOOK(void, __fastcall, Mission_CGameplayFlowPlayableMenu_Spawn, 0xD0A4D0, bool* This)
{
	if (!This[448])
	{
		uint32_t currentStage = Common::GetCurrentStageID();
		if (currentStage == SMT_pam000)
		{
			Eigen::Vector3f* position = (Eigen::Vector3f*)Common::GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0x9FF0 });
			Eigen::Quaternionf* rotation = (Eigen::Quaternionf*)Common::GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0xA2D0 });
			if (std::strcmp(Common::GetCurrentTerrain(), "pam001") == 0)
			{
				// First spawning in distortion world
				if ((*position - Eigen::Vector3f(117.506f, -3.91258f, -118.412f)).norm() <= 10.0f)
				{
					*position = Eigen::Vector3f(-12.3816f, -1.77263f, -172.612f);
					*rotation = Eigen::Quaternionf(0.949235f, 0.0f, 0.314567f, 0.0f);
				}
			}
			else
			{
				// Spawn back from Iblis (distortion world) to New City
				if ((*position - Eigen::Vector3f(-147.926f, -1.12277f, 656.42f)).norm() <= 10.0f)
				{
					*position = Eigen::Vector3f(304.942f, -6.94501f, 14.4114f);
					*rotation = Eigen::Quaternionf(0.0f, 0.0f, 1.0f, 0.0f);
				}
			}
		}
	}

	originalMission_CGameplayFlowPlayableMenu_Spawn(This);
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
		Common::fEventTrigger((void*)This, 1);
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
	if (Common::IsCurrentStageMission())
	{
		if (!MissionManager::m_missionAsStage)
		{
			// Disable goalring sfx
			WRITE_MEMORY(0x1159054, int, -1);
		}
		else
		{
			// Force fixing camera
			FUNCTION_PTR(int, __thiscall, CSonicContext_MsgPlayerGoal, 0xE6C2C0, void* player, int a2);
			CSonicContext_MsgPlayerGoal(Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer, 0);
		}
	}

	originalMission_CObjGoalRing_MsgHitEventCollision(This, Edx, message);

	// Original code
	WRITE_MEMORY(0x1159054, uint32_t, 4001005);
}

//---------------------------------------------------
// HUB NPC Talk
//---------------------------------------------------
bool MissionManager::m_missionAccept = true;
uint32_t MissionManager::m_genericNPCDialog = 0;
void* MissionManager::m_genericNPCObject = nullptr;
HOOK(void, __fastcall, Mission_CHudGateMenuMain_CStateLoadingBegin, 0x107D790, uint32_t** This)
{
	uint32_t gateStageID = This[2][84];
	uint32_t gateMissionID = This[2][85];

	if (gateMissionID > 0 && gateStageID <= SMT_pla200)
	{
		std::string stageID = std::to_string((gateMissionID << 8) + gateStageID);
		MissionManager::startGenericDialog(stageID, gateMissionID <= 5);
	}

	originalMission_CHudGateMenuMain_CStateLoadingBegin(This);
}

class CObjTalkButton : public Sonic::CGameObject
{
	enum TalkState
	{
		TS_Intro,
		TS_Usual,
		TS_Talk,
		TS_Outro,
	} m_state;

	Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectMissionButton;
	boost::shared_ptr<Sonic::CGameObjectCSD> m_spMissionButton;
	Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMissionButton;
	
	bool m_outOfControl;
	uint32_t m_genericNPCDialog;
	Eigen::Vector4f m_genericNPCPos;

public:
	CObjTalkButton(uint32_t inDialogID, Eigen::Vector4f inPos)
		: m_outOfControl(false)
		, m_genericNPCDialog(inDialogID)
		, m_genericNPCPos(inPos)
	{
	}

	~CObjTalkButton()
	{
		if (m_spMissionButton)
		{
			m_spMissionButton->SendMessage(m_spMissionButton->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
			m_spMissionButton = nullptr;
		}

		Chao::CSD::CProject::DestroyScene(m_projectMissionButton.Get(), m_sceneMissionButton);
		m_projectMissionButton = nullptr;
	}

	void AddCallback
	(
		const Hedgehog::Base::THolder<Sonic::CWorld>& worldHolder,
		Sonic::CGameDocument* pGameDocument,
		const boost::shared_ptr<Hedgehog::Database::CDatabase>& spDatabase
	) override
	{
		Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
		pGameDocument->AddUpdateUnit("1", this);
		
		Sonic::CCsdDatabaseWrapper wrapper(m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());
		auto spCsdProject = wrapper.GetCsdProject("ui_hud");
		m_projectMissionButton = spCsdProject->m_rcProject;
		if (m_projectMissionButton)
		{
			m_sceneMissionButton = m_projectMissionButton->CreateScene("btn_guide");
			m_sceneMissionButton->GetNode("btn_word")->SetPatternIndex(0);

			if (SubtitleUI::isPlayingCaption())
			{
				// there's another dialog playing, wait for it to finish
				m_sceneMissionButton->m_MotionDisableFlag = true;
				m_state = TalkState::TS_Talk;
			}
			else
			{
				StateIntro();
			}

			if (!m_spMissionButton)
			{
				m_spMissionButton = boost::make_shared<Sonic::CGameObjectCSD>(m_projectMissionButton, 0.5f, "HUD_B2", false);
				Sonic::CGameDocument::GetInstance()->AddGameObject(m_spMissionButton, "main", this);
			}
		}
	}

	bool ProcessMessage
	(
		Hedgehog::Universe::Message& message,
		bool flag
	) override
	{
		if (flag)
		{
			if (std::strstr(message.GetType(), "MsgRestartStage") != nullptr
			 || std::strstr(message.GetType(), "MsgStageClear") != nullptr)
			{
				Kill();
				return true;
			}
		}

		return Sonic::CGameObject::ProcessMessage(message, flag);
	}

	void UpdateParallel
	(
		const Hedgehog::Universe::SUpdateInfo& updateInfo
	) override
	{
		auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
		if (!context) return;

		bool validState = context->m_Grounded;
		if (validState)
		{
			Hedgehog::Base::CSharedString stateName = context->m_pPlayer->m_StateMachine.GetCurrentState()->GetStateName();
			validState  &= stateName.compare("Walk") == 0 || stateName.compare("Stand") == 0 || stateName.compare("MoveStop") == 0;
		}

		switch (m_state)
		{
		case TS_Intro:
		case TS_Usual:
		{
			// loop animation
			if (m_sceneMissionButton && m_sceneMissionButton->m_MotionDisableFlag)
			{
				PlayMotion("Usual_Anim", true);
				m_state = TalkState::TS_Usual;
			}

			if (!validState || m_genericNPCDialog != MissionManager::m_genericNPCDialog)
			{
				PlayMotion("Outro_Anim");
				m_state = TalkState::TS_Outro;
			}
			else
			{
				Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
				if (padState->IsTapped(Sonic::EKeyState::eKeyState_Y))
				{
					Common::SetPlayerVelocity(Eigen::Vector3f::Zero());
					MissionManager::startGenericDialog("NPC" + std::to_string(MissionManager::m_genericNPCDialog));

					m_outOfControl = true;
					context->StateFlag(eStateFlag_OutOfControl)++;

					PlayMotion("Outro_Anim");
					m_state = TalkState::TS_Talk;
				}
			}
			break;
		}
		case TS_Talk:
		{
			if (!SubtitleUI::isPlayingCaption())
			{
				if (m_outOfControl)
				{
					context->StateFlag(eStateFlag_OutOfControl)--;
					m_outOfControl = false;
				}
				StateIntro();
			}
			break;
		}
		case TS_Outro:
		{
			if (m_genericNPCDialog == MissionManager::m_genericNPCDialog)
			{
				if (validState)
				{
					StateIntro();
				}
			}
			else if (m_sceneMissionButton && m_sceneMissionButton->m_MotionDisableFlag)
			{
				// finished outro, kill
				Kill();
			}
			break;
		}
		}

		// Handle position
		if (m_sceneMissionButton)
		{
			if (m_state == TalkState::TS_Talk && m_sceneMissionButton->m_MotionDisableFlag)
			{
				m_sceneMissionButton->SetPosition(0, -200);
			}
			else
			{
				Eigen::Vector4f screenPosition;
				Common::fGetScreenPosition(m_genericNPCPos, screenPosition);
				m_sceneMissionButton->SetPosition(screenPosition.x(), screenPosition.y());
			}
		}
	}

	void StateIntro()
	{
		PlayMotion("Intro_Anim");
		m_state = TalkState::TS_Intro;
	}

	void PlayMotion(char const* motion, bool loop = false)
	{
		if (!m_sceneMissionButton) return;
		m_sceneMissionButton->SetHideFlag(false);
		m_sceneMissionButton->SetMotion(motion);
		m_sceneMissionButton->m_MotionDisableFlag = false;
		m_sceneMissionButton->m_MotionFrame = 0.0f;
		m_sceneMissionButton->m_MotionSpeed = 1.0f;
		m_sceneMissionButton->m_MotionRepeatType = loop ? Chao::CSD::eMotionRepeatType_Loop : Chao::CSD::eMotionRepeatType_PlayOnce;
		m_sceneMissionButton->Update();
	}

	void Kill()
	{
		auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
		if (context && m_outOfControl)
		{
			context->StateFlag(eStateFlag_OutOfControl)--;
			m_outOfControl = false;
		}

		printf("[TalkButton] Killed\n");
		SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
	}
};

HOOK(void, __fastcall, Mission_MsgNotifyObjectEvent, 0xEA4F50, Sonic::CGameObject* This, void* Edx, Sonic::Message::MsgNotifyObjectEvent& message)
{
	if (message.m_Event == 10000)
	{
		MissionManager::m_genericNPCDialog = 0;
	}
	else if (message.m_Event > 10000)
	{
		Eigen::Vector4f pos = *(Eigen::Vector4f*)(((uint32_t*)This)[46] + 112);
		pos.y() -= 0.5f;

		printf("[TalkButton] NPC ID: %d\n", message.m_Event);
		This->m_pMember->m_pGameDocument->AddGameObject(boost::make_shared<CObjTalkButton>(message.m_Event, pos));

		MissionManager::m_genericNPCDialog = message.m_Event;
		MissionManager::m_genericNPCObject = This;
	}

	originalMission_MsgNotifyObjectEvent(This, Edx, message);
}

void MissionManager::startGenericDialog(std::string const& section, bool hasYesNo)
{
	// If dialog has Yes/No box
	{
		const INIReader reader(Application::getModDirString() + NPC_DATA_FILE);
		if (!reader.Sections().count(section))
		{
			return;
		}

		hasYesNo |= reader.GetBoolean(section, "HasYesNo", false);
	}

	MissionManager::m_missionAccept = true;

	std::vector<std::string> captions;
	std::string speaker;
	MissionManager::getMissionDialog(captions, section, "MissionStart", &speaker);

	// Get accept & reject dialog size
	int acceptSize = -1;
	int rejectSize = -1;
	if (hasYesNo)
	{
		acceptSize = MissionManager::getMissionDialog(captions, section, "MissionAccept");
		rejectSize = MissionManager::getMissionDialog(captions, section, "MissionReject");
	}
	SubtitleUI::addCaption(captions, speaker, acceptSize, rejectSize);

	// Play voice
	{
		const INIReader reader(Application::getModDirString() + NPC_DATA_FILE);
		static SharedPtrTypeless soundHandle;
		Common::PlaySoundStatic(soundHandle, reader.GetInteger(section, "MissionStartVoice", -1));
	}
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
		if (gateMissionID <= 5 && MissionManager::m_missionAccept)
		{
			if (gateStageID % 2 == 0)
			{
				// Force Classic stage
				WRITE_NOP(0xD4024E, 2);
			}
			else
			{
				// Force Modern stage
				WRITE_MEMORY(0xD4024E, uint8_t, 0xEB, 0x13); 
			}

			FUNCTION_PTR(void, __cdecl, Mission_fpEnterStage, 0xD401F0, uint32_t* This, uint32_t nStageID, uint32_t nMissionID);
			Mission_fpEnterStage(This[2], gateStageID, gateMissionID);

			// Revert code
			WRITE_MEMORY(0xD4024E, uint8_t, 0x74, 0x13);
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
	INSTALL_HOOK(Mission_CStateGoalFadeBefore);

	// Load correct mission terrain
	INSTALL_HOOK(Mission_CGameplayFlowStageSetStageInfo);

	// Handle Sonic spawn points in HUB world
	INSTALL_HOOK(Mission_CGameplayFlowPlayableMenu_Spawn);

	// Change press X to interact stage gate
	WRITE_STRING(0x168B3C8, "ui_hud");
	WRITE_MEMORY(0xD315F4, Sonic::EKeyState, Sonic::EKeyState::eKeyState_Y);

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

	// Generic NPC dialog
	INSTALL_HOOK(Mission_MsgNotifyObjectEvent);

	//---------------------------------------------------
	// Individual Mission Manager/AI
	//---------------------------------------------------
    // Pele the Beloved Dog AI
	Pele::applyPatches();

	// Soleanna Boys Challenge AI
	SoleannaBoys::applyPatches();

	// Who's the Captain mission
	WhosCaptain::applyPatches();

	// Sonic Man mission
	SonicMan::applyPatches();
}
