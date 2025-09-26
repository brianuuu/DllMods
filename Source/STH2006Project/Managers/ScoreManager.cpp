#include "ScoreManager.h"

#include "Configuration.h"
#include "Objects/enemy/EnemyLander.h"
#include "Objects/enemy/EnemyMotora.h"
#include "System/ChaosEnergy.h"
#include "System/Application.h"
#include "UI/UIContext.h"

bool ScoreManager::m_internalSystem = true;
std::unordered_set<uint32_t> ScoreManager::m_savedActors;

bool ScoreManager::m_externalHUD = false;
uint32_t ScoreManager::m_scoreLimit = 999999;
std::string ScoreManager::m_scoreFormat = "%06d";
CScoreManager* ScoreManager::m_pCScoreManager = nullptr;
bool ScoreManager::m_updateScoreHUD = false;
float ScoreManager::m_currentTime = 0.0f;

uint32_t ScoreManager::m_rainbowRingChain = 0;
uint32_t ScoreManager::m_enemyChain = 0;
uint32_t ScoreManager::m_enemyCount = 0;
float ScoreManager::m_enemyChainTimer = 0.0f;

uint32_t ScoreManager::m_bonus = 0;
uint32_t ScoreManager::m_bonusToDraw = 0;
float ScoreManager::m_bonusTimer = 0.0f;
float ScoreManager::m_bonusDrawTimer = 0.0f;
IUnknown** ScoreManager::m_bonusTexture = nullptr;
IUnknown** ScoreManager::m_bonusTexturePrev = nullptr;
IUnknown* ScoreManager::m_bonus_Great = nullptr;
IUnknown* ScoreManager::m_bonus_Radical = nullptr;

// MsgRestartStage for CScoreManager
HOOK(void, __fastcall, CScoreManager_MsgRestartStage, 0xCF7F10, uint32_t* This, void* Edx, void* message)
{
	originalCScoreManager_MsgRestartStage(This, Edx, message);

	// Grab ptr here and reset HUD
	ScoreManager::m_pCScoreManager = (CScoreManager*)This;
	ScoreManager::m_updateScoreHUD = true;
}

HOOK(uint32_t*, __fastcall, CScoreManager_Destructor, 0x588AF0, uint32_t* This, void* Edx, bool a2)
{
	ScoreManager::m_pCScoreManager = nullptr;
	return originalCScoreManager_Destructor(This, Edx, a2);
}

HOOK(void, __fastcall, ScoreManager_CHudSonicStageInit, 0x109A8D0, uint32_t This)
{
	originalScoreManager_CHudSonicStageInit(This);

	// Don't change 06 HUD
	if (Configuration::m_using06HUD) return;

	// Fix up Generations' life icon
	if (!ScoreManager::m_externalHUD)
	{
		Chao::CSD::RCPtr<Chao::CSD::CScene>* gensLifeScene = (Chao::CSD::RCPtr<Chao::CSD::CScene>*)(This + 200);
		if (gensLifeScene->Get())
		{
			gensLifeScene->Get()->SetPosition(0.0f, 61.0f);
		}
	}

	// Fix mission count & get effect position
	for (int i = 0; i < 2; i++)
	{
		Chao::CSD::RCPtr<Chao::CSD::CScene>* gensMissionCountScene = (Chao::CSD::RCPtr<Chao::CSD::CScene>*)(This + 304 + i * 8);
		if (gensMissionCountScene->Get())
		{
			gensMissionCountScene->Get()->SetPosition(0.0f, 61.0f);
		}
	}
}

FUNCTION_PTR(void, __thiscall, processMsgSetPinballHud, 0x1095D40, void* This, MsgSetPinballHud const& message);
HOOK(void, __fastcall, CScoreManager_CHudSonicStageUpdate, 0x1098A50, void* This, void* Edx, float* dt)
{
	// Update score HUD
	if (ScoreManager::m_pCScoreManager && ScoreManager::m_updateScoreHUD)
	{
		alignas(16) MsgSetPinballHud message{};
		message.m_flag = 1;
		message.m_score = ScoreManager::m_pCScoreManager->m_score;
		processMsgSetPinballHud(This, message);

		ScoreManager::m_updateScoreHUD = false;
	}

	// Get current time
	ScoreManager::m_currentTime = *(float*)Common::GetMultiLevelAddress((uint32_t)This + 0xA4, { 0x0, 0x8, 0x184 });
	
	// Bonus timer
	ScoreManager::m_bonusTimer = max(0.0f, ScoreManager::m_bonusTimer - *dt);
	if (ScoreManager::m_bonusTimer == 0.0f)
	{
		ScoreManager::m_bonus = 0;
		ScoreManager::m_rainbowRingChain = 0;
		ScoreManager::m_enemyChain = 0;
	}

	// Enemy timer for stacking the same bonus
	ScoreManager::m_enemyChainTimer = max(0.0f, ScoreManager::m_enemyChainTimer - *dt);
	if (ScoreManager::m_enemyChainTimer == 0.0f)
	{
		// Award the current bonus
		if (ScoreManager::m_enemyCount >= 3)
		{
			ScoreManager::m_enemyChain++;
			ScoreManager::addScore(ScoreType::ST_enemyBonus);
		}

		ScoreManager::m_enemyCount = 0;
	}

	ScoreManager::m_bonusDrawTimer = max(0.0f, ScoreManager::m_bonusDrawTimer - *dt);
	if (ScoreManager::m_bonusDrawTimer == 0.0f)
	{
		ScoreManager::m_bonusTexture = nullptr;
		ScoreManager::m_bonusTexturePrev = nullptr;
	}

	originalCScoreManager_CHudSonicStageUpdate(This, Edx, dt);
}

// MsgRestartStage for Sonic
HOOK(int, __fastcall, ScoreManager_MsgRestartStage, 0xE76810, uint32_t* This, void* Edx, void* message)
{
	printf("[ScoreManager] RESTARTED!\n");
	ScoreManager::m_savedActors.clear();
	return originalScoreManager_MsgRestartStage(This, Edx, message);
}

void __declspec(naked) ScoreManager_CalculateResult()
{
	static uint32_t returnAddress = 0xD5A191;
	__asm
	{
		call	ScoreManager::calculateResultData
		jmp		[returnAddress]
	}
}

HOOK(int*, __fastcall, ScoreManager_GameplayManagerInit, 0xD00F70, uint32_t This, void* Edx, int* a2)
{
	uint8_t stageID = Common::GetCurrentStageID() & 0xFF;
	if 
	(
		stageID <= SMT_pla200 ||	// All main stages and missions
		stageID == SMT_cnz100 ||	// Casino Night (unused but that's original code)
		(stageID >= STH_BossFirst && stageID <= STH_BossLast) // All STH2006 bosses
	)		
	{
		// Enable CScoreManager
		WRITE_MEMORY(0xD017A7, uint8_t, (uint8_t)stageID);
		WRITE_MEMORY(0x109C1DA, uint8_t, 0x90, 0x90);

		// Hook result custom result calculation
		WRITE_JUMP(0xD5A18C, ScoreManager_CalculateResult);
	}
	else
	{
		// Original code
		WRITE_MEMORY(0xD017A7, uint8_t, 0x12);
		WRITE_MEMORY(0x109C1DA, uint8_t, 0x74, 0x78);
		WRITE_MEMORY(0xD5A18C, uint8_t, 0xE8, 0x1F, 0x9C, 0x35, 0x00);
	}

	return originalScoreManager_GameplayManagerInit(This, Edx, a2);
}

HOOK(void, __fastcall, ScoreManager_GetItem, 0xFFF810, uint32_t* This, void* Edx, void* message)
{
	Sonic::CGameObject* pObj = (Sonic::CGameObject*)This;
	switch (This[71])
	{
	default: ScoreManager::addScore(ScoreType::ST_itembox, pObj->m_ActorID); break; // 1up
	case 6:  ScoreManager::addScore(ScoreType::ST_5ring, pObj->m_ActorID); break; // 5 ring
	case 7:  ScoreManager::addScore(ScoreType::ST_10ring, pObj->m_ActorID); break; // 10 ring
	case 8:	 ScoreManager::addScore(ScoreType::ST_20ring, pObj->m_ActorID); break; // 20 ring
	}

	originalScoreManager_GetItem(This, Edx, message);
}

HOOK(int, __fastcall, ScoreManager_GetRing, 0x10534B0, Sonic::CGameObject* This, void* Edx, void* message)
{
	// Allow collecting repeated rings in Silver boss
	uint32_t stageID = Common::GetCurrentStageID();
	if (stageID != 22 && stageID != 278)
	{
		ScoreManager::addScore(ScoreType::ST_ring, This->m_ActorID);
	}

	return originalScoreManager_GetRing(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_GetSuperRing, 0x11F2F10, Sonic::CGameObject* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_10ring, This->m_ActorID);
	originalScoreManager_GetSuperRing(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_GetPhysicsMsgNotifyObjectEvent, 0xEA4F50, Sonic::CGameObject* This, void* Edx, Sonic::Message::MsgNotifyObjectEvent& message)
{
	if (message.m_Event == 12)
	{
		std::string name(*(char**)((uint32_t)This + 0x130));
		int fakeEnemyType = ChaosEnergy::getFakeEnemyType(name);
		if (fakeEnemyType)
		{
			ScoreManager::addEnemyChain(This->m_ActorID);
		}
	}

	originalScoreManager_GetPhysicsMsgNotifyObjectEvent(This, Edx, message);
}

HOOK(void, __stdcall, ScoreManager_GetPhysics, 0xEA49B0, uint32_t This, int a2, void* a3, bool a4)
{
	static std::vector<std::string> const filterList =
	{
		"sfx",
		"cmn_cage",
		"cmn_switch",
		"cmn_itembox",
		"cmn_bombbox_big",
		"lock_dummy",
		"goalpost",

		"en_e", // this will prevent fake enemy _col to be counted
		"enm_eggchaser",

		"wvo_obj_tn1_bridge",
		"wvo_obj_cliffB",
		"wvo_obj_net",
		"wvo_door",

		"wap_alert",

		"rct_obj_door",
		"rct_obj_horn",

		"kdv_mapA_bridge01",
		"kdv_obj_Tower",
		"kdv_obj_brickwall", // this doesn't give score in 06 lol
		"kdv_obj_stairs",
		"kdv_obj_scaffold",
		"kdv_door",

		"aqa_obj_magnet",
		"aqa_obj_sphere",
	};

	uint32_t actorID = ((Sonic::CGameObject*)This)->m_ActorID;

	// Check if it's a breakable object
	if (!*(bool*)(This + 0x120))
	{
		if (!a4 || *(uint32_t*)(This + 0x108) == 1)
		{
			std::string name(*(char**)(This + 0x130));
			int fakeEnemyType = ChaosEnergy::getFakeEnemyType(name);
			if (fakeEnemyType)
			{
				ScoreManager::addScore(fakeEnemyType == 1 ? ScoreType::ST_enemySmall : ScoreType::ST_enemyMedium, actorID);
			}
			else if (name == "twn_obj_barricade")
			{
				ScoreManager::addScore(ScoreType::ST_barricade, actorID);
			}
			else
			{
				bool filtered = false;
				for (std::string const& filter : filterList)
				{
					if (name.find(filter) != std::string::npos)
					{
						filtered = true;
						break;
					}
				}

				if (!filtered)
				{
					ScoreManager::addScore(ScoreType::ST_physics, actorID);
				}
			}
		}
	}

	originalScoreManager_GetPhysics(This, a2, a3, a4);
}

void __declspec(naked) ScoreManager_GetRainbow()
{
	static uint32_t returnAddress = 0x115A8FC;
	__asm
	{
		push	esi
		mov		ecx, ST_rainbow
		mov		edx, 0
		add		ScoreManager::m_rainbowRingChain, 1
		call	ScoreManager::addScore
		pop		esi

		mov     eax, [esi + 0BCh]
		jmp		[returnAddress]
	}
}

HOOK(void, __fastcall, ScoreManager_GetMissionDashRing, 0xEDB560, uint32_t This, void* Edx, void* message)
{
	if (*(uint8_t*)(This + 0x154) == 2)
	{
		ScoreManager::addScore(ScoreType::ST_missionDashRing, ((Sonic::CGameObject*)This)->m_ActorID);
	}

	originalScoreManager_GetMissionDashRing(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyGunner, 0xBAA2F0, Sonic::CGameObject* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemySmall, This->m_ActorID);
	originalScoreManager_EnemyGunner(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyStingerLancer, 0xBB01B0, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(This[104] ? ScoreType::ST_enemySmall : ScoreType::ST_enemyMedium, ((Sonic::CGameObject*)This)->m_ActorID);
	originalScoreManager_EnemyStingerLancer(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyBuster, 0xB82900, Sonic::CGameObject* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemyMedium, This->m_ActorID);
	originalScoreManager_EnemyBuster(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyFlyer, 0xBA6450, Sonic::CGameObject* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemySmall, This->m_ActorID);
	originalScoreManager_EnemyFlyer(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemySearcherHunter, 0xBDC110, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(*(bool*)((uint32_t)This + 0x17C) ? ScoreType::ST_enemySmall : ScoreType::ST_enemyMedium, ((Sonic::CGameObject*)This)->m_ActorID);
	originalScoreManager_EnemySearcherHunter(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyLiner, 0xBC7440, EnemyMotora* This, void* Edx, void* message)
{
	ScoreManager::addScore(This->m_isChaser ? ScoreType::ST_enemyMedium : ScoreType::ST_enemySmall, This->m_ActorID);
	originalScoreManager_EnemyLiner(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyBomber, 0xBCB9A0, Sonic::CGameObject* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemySmall, This->m_ActorID);
	originalScoreManager_EnemyBomber(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyRounder, 0xBCF5E0, EnemyLander* This, void* Edx, void* message)
{
	ScoreManager::addScore(This->m_isCommander ? ScoreType::ST_enemyMedium : ScoreType::ST_enemySmall, This->m_ActorID);
	originalScoreManager_EnemyRounder(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyTaker, 0xBA3140, Sonic::CGameObject* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemySmall, This->m_ActorID);
	originalScoreManager_EnemyTaker(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyBiter, 0xB86850, Sonic::CGameObject* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemySmall, This->m_ActorID);
	originalScoreManager_EnemyBiter(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyCrawler, 0xB99B80, Sonic::CGameObject* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemyMedium, This->m_ActorID);
	originalScoreManager_EnemyCrawler(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemySpinner, 0xBBD990, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(*(bool*)((uint32_t)This + 372) ? ScoreType::ST_enemyMedium : ScoreType::ST_enemySmall, ((Sonic::CGameObject*)This)->m_ActorID);
	originalScoreManager_EnemySpinner(This, Edx, message);
}

void __declspec(naked) ScoreManager_NextRankScore()
{
	static uint32_t returnAddress = 0x10B6037;
	__asm
	{
		// Don't display for S rank
		mov     eax, [edi + 208h]
		cmp		eax, RT_S
		jne		jump
		retn	4

		jump:
		sub		esp, 14h
		push	esi
		push	ecx
		lea		ecx, [esp + 14h - 8h]
		jmp		[returnAddress]
	}
}

HOOK(bool, __cdecl, ScoreManager_IsPerfectBonus, 0x10B8A90)
{
	if (ScoreManager::m_pCScoreManager)
	{
		return false;
	}

	return originalScoreManager_IsPerfectBonus();
}

HOOK(bool, __fastcall, ScoreManager_CRivalSilverMsgDamage, 0xC89080, uint32_t* This, void* Edx, void* message)
{
	// Add score for Silver's final hit
	uint32_t* pCSilver = (uint32_t*)This[104];
	if ((pCSilver[81] & 0x100000) != 0)
	{
		ScoreManager::addScore(ScoreType::ST_boss, ((Sonic::CGameObject*)This)->m_ActorID);
	}

	return originalScoreManager_CRivalSilverMsgDamage(This, Edx, message);
}

void __declspec(naked) ScoreManager_CBossPerfectChaosAddScore()
{
	static uint32_t returnAddress = 0xC0FFCA;
	__asm
	{
		push	esi
		mov		edx, esi
		mov		ecx, ST_boss
		call	ScoreManager::addScore
		pop		esi

		// original function
		push	[0x01587D58]
		jmp		[returnAddress]
	}
}

std::string externIniPath;
void ScoreManager::applyPatches()
{
	if (!Configuration::m_using06ScoreSystem)
	{
		return;
	}

	// Check if we are using external HUD
	std::vector<std::string> modIniList;
	Common::GetModIniList(modIniList);
	for (int i = modIniList.size() - 1; i >= 0; i--)
	{
		std::string const& config = modIniList[i];
		std::string scoreGenerationsConfig = config.substr(0, config.find_last_of("\\")) + "\\ScoreGenerations.ini";
		if (!scoreGenerationsConfig.empty() && Common::IsFileExist(scoreGenerationsConfig))
		{
			INIReader configReader(scoreGenerationsConfig);
			m_scoreLimit = configReader.GetInteger("Behaviour", "scoreLimit", 999999);
			m_scoreFormat = configReader.Get("Appearance", "scoreFormat", "%01d");
			if (configReader.GetBoolean("Developer", "customXNCP", false))
			{
				m_externalHUD = true;
				break;
			}
		}
	}

	printf("[ScoreManager] STH2006 score system enabled\n");

	// Reset stage
	INSTALL_HOOK(ScoreManager_MsgRestartStage);

	// Common score hooks
	INSTALL_HOOK(ScoreManager_GetItem);
	INSTALL_HOOK(ScoreManager_GetRing);
	INSTALL_HOOK(ScoreManager_GetSuperRing);
	INSTALL_HOOK(ScoreManager_GetPhysicsMsgNotifyObjectEvent);
	INSTALL_HOOK(ScoreManager_GetPhysics);
	WRITE_JUMP(0x115A8F6, ScoreManager_GetRainbow);
	INSTALL_HOOK(ScoreManager_GetMissionDashRing);

	// Enemy score hooks
	INSTALL_HOOK(ScoreManager_EnemyGunner);
	INSTALL_HOOK(ScoreManager_EnemyStingerLancer);
	INSTALL_HOOK(ScoreManager_EnemyBuster);
	INSTALL_HOOK(ScoreManager_EnemyFlyer);
	INSTALL_HOOK(ScoreManager_EnemySearcherHunter);
	INSTALL_HOOK(ScoreManager_EnemyLiner);
	INSTALL_HOOK(ScoreManager_EnemyBomber);
	INSTALL_HOOK(ScoreManager_EnemyRounder);
	INSTALL_HOOK(ScoreManager_EnemyTaker);
	INSTALL_HOOK(ScoreManager_EnemyBiter);
	INSTALL_HOOK(ScoreManager_EnemyCrawler);
	INSTALL_HOOK(ScoreManager_EnemySpinner);

	// Set score format
	WRITE_MEMORY(0x1095D7D, char*, m_scoreFormat.c_str());

	// Enable CScoreManager in regular stages
	INSTALL_HOOK(ScoreManager_GameplayManagerInit);

	// CScoreManager score handling
	INSTALL_HOOK(CScoreManager_MsgRestartStage);
	INSTALL_HOOK(CScoreManager_Destructor);
	INSTALL_HOOK(ScoreManager_CHudSonicStageInit);
	INSTALL_HOOK(CScoreManager_CHudSonicStageUpdate);

	// Hijack next rank time as next rank score
	WRITE_STRING(0x16940F4, "%06d");
	WRITE_JUMP(0x10B5FF6, ScoreManager_NextRankScore);
	WRITE_MEMORY(0x10B6044, uint8_t, 0xC);
	WRITE_MEMORY(0x10B60AF, uint8_t, 0xEB);

	// Disable perfect bonus
	INSTALL_HOOK(ScoreManager_IsPerfectBonus);

	// Silver boss
	INSTALL_HOOK(ScoreManager_CRivalSilverMsgDamage);
	WRITE_MEMORY(0xC783DE, bool, true); // Don't hide HUD after defeating

	// Iblis boss
	WRITE_JUMP(0xC0FFC5, ScoreManager_CBossPerfectChaosAddScore);
}

void __fastcall ScoreManager::addScore(ScoreType type, uint32_t actorID)
{
	// Prevent same object to add score multiple times
	if (actorID)
	{
		if (m_savedActors.count(actorID))
		{
			printf("[ScoreManager] Actor %u score already added\n", actorID);
			return;
		}

		m_savedActors.insert(actorID);
	}

	int score = 0;
	switch (type)
	{
	case ST_ring:			score = 10;		break;
	case ST_5ring:			score = 50;		break;
	case ST_10ring:			score = 100;	break;
	case ST_20ring:			score = 200;	break;
	case ST_rainbow:		score = calculateRainbowRingChainBonus();	break;
	case ST_physics:		score = 20;		break;
	case ST_itembox:		score = 200;	break;
	case ST_enemySmall:		score = 100;	break;
	case ST_enemyMedium:	score = 300;	break;
	case ST_enemyLarge:		score = 1000;	break;
	case ST_enemyStealth:	score = 1500;	break;
	case ST_enemyBonus:		score = calculateEnemyChainBonus();	break;
	case ST_boss:			score = 10000;	break;
	case ST_missionDashRing:score = 100;	break;
	case ST_barricade:		score = 500;	break;
	default: return;
	}
	
	bool resetBonus = false;

	// Add to rainbow ring stack bonus and notify draw GUI
	if (type == ST_rainbow)
	{
		// Chain from enemy +600
		if (m_enemyChain == 1)
		{
			score += 600;
			resetBonus = true;
		}
		else if (m_enemyChain > 1)
		{
			m_bonus = 0;
			m_enemyChain = 0;
		}

		m_bonus += score;
		notifyDraw(BonusCommentType::BCT_Great);
	}

	// Add to enemy stack bonus and notify draw GUI
	if (type == ST_enemyBonus)
	{
		// Chain from rainbow ring +600
		if (m_rainbowRingChain == 1)
		{
			score += 600;
			resetBonus = true;
		}
		else if (m_rainbowRingChain > 1)
		{
			m_bonus = 0;
			m_rainbowRingChain = 0;
		}

		m_bonus += score;
		notifyDraw(BonusCommentType::BCT_Radical);
	}

	printf("[ScoreManager] Actor %u \tScore +%d \tType: %s\n", actorID, score, GetScoreTypeName(type));
	if (m_internalSystem)
	{
		if (m_pCScoreManager)
		{
			m_pCScoreManager->m_score = min(m_pCScoreManager->m_score + score, 999999);
			m_updateScoreHUD = true;
		}
	}
	else
	{
		ScoreGenerationsAPI::AddScore(score);
	}

	// Switching chain add the first time, but discontinue afterwards
	// Ask Sonic Team why that is
	if (resetBonus)
	{
		m_bonus = 0;
		m_enemyChain = 0;
		m_rainbowRingChain = 0;
	}
}

uint32_t ScoreManager::calculateRainbowRingChainBonus()
{
	switch (m_rainbowRingChain)
	{
	case 1: return 1000;
	case 2: return 1600;
	case 3: return 2000;
	case 4: return 3400;
	default: return 4000 + (m_rainbowRingChain - 5) * 600;
	}
}

void ScoreManager::addEnemyChain(uint32_t actorID)
{
	// Already counted this object
	if (!actorID || m_savedActors.count(actorID))
	{
		return;
	}

	// Add to the current enemy chain
	m_enemyCount++;
	m_enemyChainTimer = 0.2f;
	printf("[ScoreManager] Enemy Chain: %d\n", m_enemyCount);
}

uint32_t ScoreManager::calculateEnemyChainBonus()
{
	if (m_enemyCount < 3) return 0;

	uint32_t baseScore = m_enemyCount * 500;
	switch (m_enemyChain)
	{
	case 1: return baseScore - 900;
	case 2: return baseScore - 300;
	case 3: return baseScore + 100;
	default: return baseScore + 1500 + (m_enemyChain - 4) * 600;
	}
}

int ScoreManager::m_timeBonus = 0;
ResultData* ScoreManager::calculateResultData()
{
	// Default score table for regular stages
	ScoreTable scoreTable{ 50000, 45000, 25000, 5000 };

	int timeBonusBase = 0;
	int timeBonusRate = 40;

	// Get base score and timebonus rate
	uint32_t const stageID = Common::GetCurrentStageID();
	switch (stageID)
	{
	case SMT_ghz100:	timeBonusBase = 45000;	break; // Prelude Stage
	case SMT_ghz200:	timeBonusBase = 17000;	break; // Wave Ocean
	case SMT_cpz200:	timeBonusBase = 43000;	break; // TODO: Dusty Desert
	case SMT_ssz200:	timeBonusBase = 50000;	break; // TODO: White Acropolis
	case SMT_sph200:	timeBonusBase = 27000;	break; // Crisis City
	case SMT_cte200:	timeBonusBase = 50000;	break; // TODO: Flame Core
	case SMT_ssh200:	timeBonusBase = 45000;	break; // TODO: Radical Train
	case SMT_csc200:	timeBonusBase = 36000;	break; // Tropical Jungle
	case SMT_euc200:	timeBonusBase = 24000;	break; // Kingdom Valley
	case SMT_pla200:	timeBonusBase = 23000;	break; // TODO: Aquatic Base
	case (SMT_ghz100 | SMT_Mission1):	timeBonusBase = 41000;	break; // Wave Ocean: Blaze
	case (SMT_ghz100 | SMT_Mission2):	timeBonusBase = 26000;	break; // Gems Training Ground
	case SMT_bsl:						timeBonusBase = 21000;	break; // Silver
	case (SMT_bsl | SMT_BossHard):		timeBonusBase = 22000;	break; // Silver Hard Mode
	case SMT_bpc:						timeBonusBase = 19000;	break; // Iblis
	case (SMT_bpc | SMT_BossHard):		timeBonusBase = 20000;	break; // Iblis Hard Mode
	default:	break;
	}

	// Overwrite rank table on special stages (boss stages, missions etc.)
	switch (stageID)
	{
	case SMT_bsl:					// Silver
	case (SMT_bsl | SMT_BossHard):	// Silver Hard Mode
	case SMT_bpc:					// Iblis
	case (SMT_bpc | SMT_BossHard):	// Iblis Hard Mode
	{
		scoreTable = ScoreTable{ 30000,27500,25000,5000 };
		timeBonusRate = 80;
		break;
	}
	case (SMT_ghz200 | SMT_Mission1): // Sonic Mission 1
	{
		scoreTable = ScoreTable{ 30000,27500,25000,22500 };
		timeBonusBase = 40000;
		timeBonusRate = 500;
		break;
	}
	case (SMT_ghz200 | SMT_Mission2): // Sonic Mission 2
	{
		scoreTable = ScoreTable{ 30000,27500,25000,22500 };
		timeBonusBase = 30000;
		timeBonusRate = 100;
		break;
	}
	case (SMT_ghz200 | SMT_Mission3): // Sonic Mission 3
	case (SMT_ghz200 | SMT_Mission5): // Sonic Mission 5
	{
		scoreTable = ScoreTable{ 30000,27500,25000,22500 };
		timeBonusBase = 36000;
		timeBonusRate = 100;
		break;
	}
	case (SMT_ghz200 | SMT_Mission4): // Sonic Mission 4
	{
		scoreTable = ScoreTable{ 30000,27500,25000,22500 };
		timeBonusBase = 40500;
		timeBonusRate = 100;
		break;
	}
	case (SMT_ghz100 | SMT_Mission3): // Sonic Mission 14
	{
		scoreTable = ScoreTable{ 30000,27500,25000,22500 };
		timeBonusBase = 59000;
		timeBonusRate = 1000;
		break;
	}
	default: break;
	}

	// Read score data from INI
	const INIReader reader(Application::getModDirString() + "Assets\\Database\\scoreData.ini");
	if (reader.ParseError() == 0)
	{
		std::string const currentStageStr = std::to_string(stageID);
		scoreTable.m_scoreS = reader.GetInteger(currentStageStr, "S", scoreTable.m_scoreS);
		scoreTable.m_scoreA = reader.GetInteger(currentStageStr, "A", scoreTable.m_scoreA);
		scoreTable.m_scoreB = reader.GetInteger(currentStageStr, "B", scoreTable.m_scoreB);
		scoreTable.m_scoreC = reader.GetInteger(currentStageStr, "C", scoreTable.m_scoreC);
		timeBonusBase = reader.GetInteger(currentStageStr, "timeBonusBase", timeBonusBase);
		timeBonusRate = reader.GetInteger(currentStageStr, "timeBonusRate", timeBonusRate);
	}

	// Calculate final score
	static ResultData data{};
	m_timeBonus = max(timeBonusBase - (int)floorf(ScoreManager::m_currentTime) * timeBonusRate, 0 );
	int ringBonus = *Common::GetPlayerRingCount() * 100;
	data.m_score = m_pCScoreManager->m_score + m_timeBonus + ringBonus;

	// Get current rank and score to next rank (if applicable)
	if (data.m_score >= scoreTable.m_scoreS)
	{
		data.m_rank = ResultRankType::RT_S;
		data.m_nextRankScore = 0;
	}
	else if (data.m_score >= scoreTable.m_scoreA)
	{
		data.m_rank = ResultRankType::RT_A;
		data.m_nextRankScore = scoreTable.m_scoreS - data.m_score;
	}
	else if (data.m_score >= scoreTable.m_scoreB)
	{
		data.m_rank = ResultRankType::RT_B;
		data.m_nextRankScore = scoreTable.m_scoreA - data.m_score;
	}
	else if (data.m_score >= scoreTable.m_scoreC)
	{
		data.m_rank = ResultRankType::RT_C;
		data.m_nextRankScore = scoreTable.m_scoreB - data.m_score;
	}
	else
	{
		data.m_rank = ResultRankType::RT_D;
		data.m_nextRankScore = scoreTable.m_scoreC - data.m_score;
	}
	data.m_perfectRank = data.m_rank;

	// Get the prop of "base score + time bonus" and "total score" (since we only have 2 bars)
	data.m_timeProp = getScoreProp(scoreTable, m_pCScoreManager->m_score + m_timeBonus);
	data.m_totalProp = getScoreProp(scoreTable, data.m_score);

	printf
	(
		"[ScoreManager] Calculating result data:\n"
		"Time: %02d:%02d.%03d (%.3f)\n"
		"Rings: %d\n"
		"Rank: %s\n"
		"Base Score = \t%d\n"
		"Time Bonus = \t%d (%d - %d * %d)\n"
		"Ring Bonus = \t%d (%d * 100)\n"
		"Final Score = \t%d\n"
		"Time Prop = \t%.4f\n"
		"Total Prop = \t%.4f\n"
		, (int)m_currentTime / 60
		, (int)m_currentTime % 60
		, (int)(m_currentTime * 1000.0f) % 1000
		, m_currentTime
		, *Common::GetPlayerRingCount()
		, data.m_rank == ResultRankType::RT_S ? "S"
		: (data.m_rank == ResultRankType::RT_A ? "A"
		: (data.m_rank == ResultRankType::RT_B ? "B"
		: (data.m_rank == ResultRankType::RT_C ? "C" : "D")))
		, m_pCScoreManager->m_score
		, m_timeBonus
		, timeBonusBase
		, (int)floorf(m_currentTime)
		, timeBonusRate
		, ringBonus
		, *Common::GetPlayerRingCount()
		, data.m_score
		, data.m_timeProp
		, data.m_totalProp
	);
	
	return &data;
}

float ScoreManager::getScoreProp(ScoreTable const& scoreTable, int score)
{
	if (score > scoreTable.m_scoreA)
	{
		return 1.0f + getPropBetween(scoreTable.m_scoreA, scoreTable.m_scoreS, score) * 0.3333f;
	}
	else if (score > scoreTable.m_scoreB)
	{
		return 0.6667f + getPropBetween(scoreTable.m_scoreB, scoreTable.m_scoreA, score) * 0.3333f;
	}
	else if (score > scoreTable.m_scoreC)
	{
		return 0.3333f + getPropBetween(scoreTable.m_scoreC, scoreTable.m_scoreB, score) * 0.3333f;
	}
	else
	{
		return getPropBetween(0, scoreTable.m_scoreC, score) * 0.3333f;
	}
}

float ScoreManager::getPropBetween(int min, int max, int num)
{
	float minF = (float)min;
	float maxF = (float)max;
	float numF = (float)num;
	return (numF - minF) / (maxF - minF);
}

void ScoreManager::notifyDraw(BonusCommentType type)
{
	// Update bonus to draw
	m_bonusToDraw = m_bonus;

	// Always clear texture first
	if (m_bonusTexture)
	{
		m_bonusTexture = nullptr;
	}

	switch (type)
	{
		case BCT_Great:		m_bonusTexture = &m_bonus_Great;   break;
		case BCT_Radical:	m_bonusTexture = &m_bonus_Radical; break;
	}

	// Only draw comment if there not already one or has switched
	if (m_bonusTexture == m_bonusTexturePrev && m_bonusDrawTimer > 0.0f)
	{
		m_bonusTexture = nullptr;
	}
	else
	{
		m_bonusTexturePrev = m_bonusTexture;
	}

	m_bonusTimer = 10.0f;
	m_bonusDrawTimer = 4.0f;
}

bool ScoreManager::initTextures()
{
	std::wstring const dir = Application::getModDirWString();
	bool success = true;
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Bonus\\Bonus_Great.dds").c_str(), &m_bonus_Great);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Bonus\\Bonus_Radical.dds").c_str(), &m_bonus_Radical);
	
	if (!success)
	{
		MessageBox(nullptr, TEXT("Failed to load assets for score bonus texts, they may not display correctly"), TEXT("STH2006 Project"), MB_ICONWARNING);
	}

	return success;
}

void ScoreManager::draw()
{
	// At loading screen, clear all
	if (Common::IsAtLoadingScreen())
	{
		clearDraw();
		return;
	}

	if (m_bonusToDraw > 0 && m_bonusDrawTimer > 0)
	{
		static bool visible = true;
		ImGui::Begin("Bonus", &visible, UIContext::m_hudFlags);
		{
			std::string const bonusStr = std::to_string(m_bonusToDraw);
			ImVec2 size = ImGui::CalcTextSize(bonusStr.c_str());
			ImGui::SetWindowPos(ImVec2(0, 0));
			ImGui::SetCursorPos(ImVec2((float)*BACKBUFFER_WIDTH * 0.8f - size.x / 2, (float)*BACKBUFFER_HEIGHT * 0.295f - size.y / 2));
			ImGui::Text(bonusStr.c_str());
		}
		ImGui::End();

		if (m_bonusTexture)
		{
			ImGui::Begin("BonusComment", &visible, UIContext::m_hudFlags);
			{
				float sizeX = *BACKBUFFER_WIDTH * 300.0f / 1280.0f;
				float sizeY = *BACKBUFFER_HEIGHT * 50.0f / 720.0f;
				float posX = 0.64083f;
				float posY = 0.34052f;
				if (m_bonusDrawTimer > 3.9f)
				{
					posX = 1.0f - (1.0f - posX) * ((4.0f - m_bonusDrawTimer) / 0.1f);
				}

				ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * posX, *BACKBUFFER_HEIGHT * posY));
				ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
				ImGui::Image(*m_bonusTexture, ImVec2(sizeX, sizeY));
			}
			ImGui::End();
		}
	}
}

void ScoreManager::clearDraw()
{
	m_bonus = 0;
	m_bonusDrawTimer = 0.0f;
	m_rainbowRingChain = 0;
	m_enemyChain = 0;
}
