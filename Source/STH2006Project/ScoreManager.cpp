#include "ScoreManager.h"
#include "Configuration.h"
#include "ChaosEnergy.h"
#include "Application.h"
#include "UIContext.h"

bool ScoreManager::m_enabled = false;
bool ScoreManager::m_internalSystem = true;
std::unordered_set<uint32_t*> ScoreManager::m_savedObjects;

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
float ScoreManager::m_bonusTimer = 0.0f;
float ScoreManager::m_bonusDrawTimer = 0.0f;
PDIRECT3DTEXTURE9 ScoreManager::m_bonusTexture = nullptr;
PDIRECT3DTEXTURE9 ScoreManager::m_bonus_Great = nullptr;
PDIRECT3DTEXTURE9 ScoreManager::m_bonus_Radical = nullptr;

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
			ScoreManager::addScore(ScoreType::ST_enemyBonus, nullptr);
		}

		ScoreManager::m_enemyCount = 0;
	}

	ScoreManager::m_bonusDrawTimer = max(0.0f, ScoreManager::m_bonusDrawTimer - *dt);
	if (ScoreManager::m_bonusDrawTimer == 0.0f)
	{
		ScoreManager::m_bonusTexture = nullptr;
	}

	originalCScoreManager_CHudSonicStageUpdate(This, Edx, dt);
}

// MsgRestartStage for Sonic
HOOK(int, __fastcall, ScoreManager_MsgRestartStage, 0xE76810, uint32_t* This, void* Edx, void* message)
{
	printf("[ScoreManager] RESTARTED!\n");
	ScoreManager::m_savedObjects.clear();
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

HOOK(int*, __fastcall, ScoreManager_GameplayManagerInit, 0xD00F70, void* This, void* Edx, int* a2)
{
	static char const* scoreHUD = "ui_gameplay_score";
	static char const* defaultHUD = (char*)0x0168E328;

	uint32_t stageID = Common::GetCurrentStageID();
	if 
	(
		stageID <= 17 ||	// All main stages
		stageID == 18 ||	// Casino Night (unused but that's original code)
		stageID == 22 ||	// Silver
		stageID == 278 ||	// Silver Hard Mode
		stageID == 24 ||	// Iblis
		stageID == 280 		// Iblis Hard Mode
	)		
	{
		// Enable CScoreManager
		WRITE_MEMORY(0xD017A7, uint8_t, (uint8_t)stageID);
		WRITE_MEMORY(0x109C1DA, uint8_t, 0x90, 0x90);

		// Use fixed score HUD if not using external
		if (!ScoreManager::m_externalHUD)
		{
			WRITE_MEMORY(0x109D669, char*, scoreHUD);
		}

		// Hook result custom result calculation
		WRITE_JUMP(0xD5A18C, ScoreManager_CalculateResult);
	}
	else
	{
		// Original code
		WRITE_MEMORY(0xD017A7, uint8_t, 0x12);
		WRITE_MEMORY(0x109C1DA, uint8_t, 0x74, 0x78);
		WRITE_MEMORY(0x109D669, char*, defaultHUD);
		WRITE_MEMORY(0xD5A18C, uint8_t, 0xE8, 0x1F, 0x9C, 0x35, 0x00);
	}

	return originalScoreManager_GameplayManagerInit(This, Edx, a2);
}

HOOK(void, __fastcall, ScoreManager_GetItem, 0xFFF810, uint32_t* This, void* Edx, void* message)
{
	switch (This[71])
	{
	default: ScoreManager::addScore(ScoreType::ST_itembox, This); break; // 1up
	case 6:  ScoreManager::addScore(ScoreType::ST_5ring, This); break; // 5 ring
	case 7:  ScoreManager::addScore(ScoreType::ST_10ring, This); break; // 10 ring
	case 8:	 ScoreManager::addScore(ScoreType::ST_20ring, This); break; // 20 ring
	}

	originalScoreManager_GetItem(This, Edx, message);
}

HOOK(int, __fastcall, ScoreManager_GetRing, 0x10534B0, uint32_t* This, void* Edx, void* message)
{
	uint32_t* owner = This;

	// Allow collecting repeated rings in Silver boss
	uint32_t stageID = Common::GetCurrentStageID();
	if (stageID == 22 || stageID == 278)
	{
		owner = nullptr;
	}

	ScoreManager::addScore(ScoreType::ST_ring, owner);
	return originalScoreManager_GetRing(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_GetSuperRing, 0x11F2F10, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_10ring, This);
	originalScoreManager_GetSuperRing(This, Edx, message);
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

	// Check if it's a breakable object
	if (!*(bool*)(This + 0x120))
	{
		if (!a4 || *(uint32_t*)(This + 0x108) == 1)
		{
			std::string name(*(char**)(This + 0x130));
			int fakeEnemyType = ChaosEnergy::getFakeEnemyType(name);
			if (fakeEnemyType)
			{
				ScoreManager::addScore(fakeEnemyType == 1 ? ScoreType::ST_enemySmall : ScoreType::ST_enemyMedium, (uint32_t*)This);
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
					ScoreManager::addScore(ScoreType::ST_physics, (uint32_t*)This);
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

HOOK(void, __fastcall, ScoreManager_EnemyGunner, 0xBAA2F0, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addEnemyChain(This, message);
	ScoreManager::addScore(ScoreType::ST_enemySmall, This);
	originalScoreManager_EnemyGunner(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyStingerLancer, 0xBB01B0, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addEnemyChain(This, message);
	ScoreManager::addScore(This[104] ? ScoreType::ST_enemySmall : ScoreType::ST_enemyMedium, This);
	originalScoreManager_EnemyStingerLancer(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyBuster, 0xB82900, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addEnemyChain(This, message);
	ScoreManager::addScore(ScoreType::ST_enemyMedium, This);
	originalScoreManager_EnemyBuster(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyFlyer, 0xBA6450, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addEnemyChain(This, message);
	ScoreManager::addScore(ScoreType::ST_enemySmall, This);
	originalScoreManager_EnemyFlyer(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemySearcherHunter, 0xBDC110, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addEnemyChain(This, message);
	ScoreManager::addScore(*(bool*)((uint32_t)This + 0x17C) ? ScoreType::ST_enemySmall : ScoreType::ST_enemyMedium, This);
	originalScoreManager_EnemySearcherHunter(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyLiner, 0xBC7440, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addEnemyChain(This, message);
	ScoreManager::addScore(ScoreType::ST_enemySmall, This);
	originalScoreManager_EnemyLiner(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyBomber, 0xBCB9A0, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addEnemyChain(This, message);
	ScoreManager::addScore(ScoreType::ST_enemySmall, This);
	originalScoreManager_EnemyBomber(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyRounder, 0xBCF5E0, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addEnemyChain(This, message);
	ScoreManager::addScore(ScoreType::ST_enemySmall, This);
	originalScoreManager_EnemyRounder(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyTaker, 0xBA3140, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addEnemyChain(This, message);
	ScoreManager::addScore(ScoreType::ST_enemySmall, This);
	originalScoreManager_EnemyTaker(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyBiter, 0xB86850, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addEnemyChain(This, message);
	ScoreManager::addScore(ScoreType::ST_enemySmall, This);
	originalScoreManager_EnemyBiter(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyCrawler, 0xB99B80, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addEnemyChain(This, message);
	ScoreManager::addScore(ScoreType::ST_enemyMedium, This);
	originalScoreManager_EnemyCrawler(This, Edx, message);
}

HOOK(int, __fastcall, ScoreManager_CGameObject3DDestruction, 0xD5D790, uint32_t* This)
{
	ScoreManager::m_savedObjects.erase(This);
	return originalScoreManager_CGameObject3DDestruction(This);
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
		ScoreManager::addScore(ScoreType::ST_boss, pCSilver);
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
	// Must enable Score Generations, whether for internal score system or not
	if (!Common::IsModEnabled("Score Generations", &externIniPath))
	{
		return;
	}

	// Ensure Score Generations is loaded LATER/higher priority than this mod
	if (GetModuleHandle(TEXT("ScoreGenerations.dll")) != nullptr)
	{
		MessageBox(NULL, L"'Score Generations' mod detected, please put it higher priority than 'STH2006 Project'!", L"STH2006 Project", MB_ICONERROR);
		exit(-1);
	}

	printf("[ScoreManager] STH2006 score system enabled\n");
	m_enabled = true;

	// Reset stage
	INSTALL_HOOK(ScoreManager_MsgRestartStage);

	// Remove from list when object is destructed
	INSTALL_HOOK(ScoreManager_CGameObject3DDestruction);

	// Common score hooks
	INSTALL_HOOK(ScoreManager_GetItem);
	INSTALL_HOOK(ScoreManager_GetRing);
	INSTALL_HOOK(ScoreManager_GetSuperRing);
	INSTALL_HOOK(ScoreManager_GetPhysics);
	WRITE_JUMP(0x115A8F6, ScoreManager_GetRainbow);

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

	if (m_internalSystem)
	{
		applyPatches_InternalSystem();
	}
	else
	{
		applyPatches_ScoreGensSystem();
	}

	// Silver boss
	INSTALL_HOOK(ScoreManager_CRivalSilverMsgDamage);
	WRITE_MEMORY(0xC783DE, bool, true); // Don't hide HUD after defeating

	// Iblis boss
	WRITE_JUMP(0xC0FFC5, ScoreManager_CBossPerfectChaosAddScore)
}

void ScoreManager::applyPatches_ScoreGensSystem()
{
	
}

void ScoreManager::applyPatches_InternalSystem()
{
	// Disable extern .dll in mod.ini
	setExternalIni(false);

	std::vector<std::string> modIniList;
	Common::GetModIniList(modIniList);
	for (size_t i = 0; i < modIniList.size(); i++)
	{
		std::string const& config = modIniList[i];
		std::string scoreGenerationsConfig = config.substr(0, config.find_last_of("\\")) + "\\ScoreGenerations.ini";
		if (!scoreGenerationsConfig.empty() && Common::IsFileExist(scoreGenerationsConfig))
		{
			INIReader configReader(scoreGenerationsConfig);
			if (configReader.GetBoolean("Developer", "customXNCP", false))
			{
				m_externalHUD = true;
				m_scoreLimit = configReader.GetInteger("Behaviour", "scoreLimit", 999999);
				m_scoreFormat = configReader.Get("Appearance", "scoreFormat", "%01d");
				break;
			}
		}
	}

	// Set score format
	WRITE_MEMORY(0x1095D7D, char*, m_scoreFormat.c_str());

	// Enable CScoreManager in regular stages
	INSTALL_HOOK(ScoreManager_GameplayManagerInit);

	// CScoreManager score handling
	INSTALL_HOOK(CScoreManager_MsgRestartStage);
	INSTALL_HOOK(CScoreManager_Destructor);
	INSTALL_HOOK(CScoreManager_CHudSonicStageUpdate);
	
	// Hijack next rank time as next rank score
	WRITE_STRING(0x16940F4, "%06d");
	WRITE_JUMP(0x10B5FF6, ScoreManager_NextRankScore);
	WRITE_MEMORY(0x10B6044, uint8_t, 0xC);
	WRITE_MEMORY(0x10B60AF, uint8_t, 0xEB);

	// Disable perfect bonus
	INSTALL_HOOK(ScoreManager_IsPerfectBonus);
}

void ScoreManager::applyPostInit()
{
	if (!m_enabled) return;

	if (m_internalSystem)
	{
		// Reset extern .dll in mod.ini
		setExternalIni(true);

		// We shouldn't have loaded ScoreGenerations.dll
		if (GetModuleHandle(TEXT("ScoreGenerations.dll")) != nullptr)
		{
			MessageBox(NULL, L"An error occured when initializing internal score system, 'Score Generations' MUST be higher priority than 'STH2006 Project'!", L"STH2006 Project", MB_ICONERROR);
			exit(-1);
		}
	}
	else
	{
		std::string iniFile = Application::getModDirString() + "ScoreGenerations.ini";
		if (!Common::IsFileExist(iniFile))
		{
			MessageBox(NULL, L"Failed to parse ScoreGenerations.ini", L"STH2006 Project", MB_ICONERROR);
		}
		else
		{
			printf("[ScoreManager] Forcing STH2006 Project score table...\n");
			ScoreGenerationsAPI::ForceConfiguration(iniFile.c_str());
		}
	}
}

void ScoreManager::setExternalIni(bool reset)
{
	if (Common::IsFileExist(externIniPath))
	{
		std::string content;
		std::ifstream in(externIniPath);
		if (in)
		{
			std::string line;
			while (getline(in, line))
			{
				if (line.find("DLLFile") != std::string::npos)
				{
					if (reset)
					{
						content += "DLLFile=\"ScoreGenerations.dll\"";
					}
					else
					{
						// Absolute path doesn't work, so we have to use relative...
						std::string const modDir = Application::getModDirString();
						content += "DLLFile=\"..\\" + modDir.substr(modDir.find_last_of("\\/", modDir.size() - 2) + 1) + "STH2006ProjectExtra.dll\"";
					}
				}
				else
				{
					content += line;
				}

				content += "\n";
			}
			in.close();
		}

		std::ofstream out(externIniPath);
		if (out)
		{
			out << content;
			out.close();
		}
	}
}

void __fastcall ScoreManager::addScore(ScoreType type, uint32_t* This)
{
	// Prevent same object to add score multiple times
	if (This)
	{
		if (m_savedObjects.count(This))
		{
			printf("[ScoreManager] WARNING: Duplicated score\n");
			return;
		}

		m_savedObjects.insert(This);
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
	default: return;
	}

	// Add to rainbow ring stack bonus and notify draw GUI
	if (type == ST_rainbow)
	{
		// Resets enemy bonus
		if (m_enemyChain > 0)
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
		// Resets rainbow ring bonus
		if (m_rainbowRingChain > 0)
		{
			m_bonus = 0;
			m_rainbowRingChain = 0;
		}

		m_bonus += score;
		notifyDraw(BonusCommentType::BCT_Radical);
	}

	printf("[ScoreManager] Score +%d \tType: %s\n", score, GetScoreTypeName(type));
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

void ScoreManager::addEnemyChain(uint32_t* This, void* message)
{
	// Already counted this object
	if (!This || m_savedObjects.count(This))
	{
		return;
	}

	// Must be MsgNotifyObjectEvent event 12
	uint32_t* msg = (uint32_t*)message;
	if (msg[0] != 0x016A9D98 || msg[4] != 12)
	{
		return;
	}

	// Add to the current enemy chain
	m_enemyCount++;
	m_enemyChainTimer = 0.2f;
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
	uint32_t stageID = Common::GetCurrentStageID();
	switch (stageID)
	{
	case 0:		timeBonusBase = 45000;	break; // Prelude Stage
	case 1:		timeBonusBase = 17000;	break; // Wave Ocean
	case 3:		timeBonusBase = 43000;	break; // TODO: Dusty Desert
	case 5:		timeBonusBase = 50000;	break; // TODO: White Acropolis
	case 7:		timeBonusBase = 27000;	break; // Crisis City
	case 9:		timeBonusBase = 50000;	break; // TODO: Flame Core
	case 11:	timeBonusBase = 45000;	break; // TODO: Radical Train
	case 13:	timeBonusBase = 36000;	break; // Tropical Jungle
	case 15:	timeBonusBase = 24000;	break; // Kingdom Valley
	case 17:	timeBonusBase = 23000;	break; // TODO: Aquatic Base
	case 22:	timeBonusBase = 21000;	break; // Silver
	case 278:	timeBonusBase = 22000;	break; // Silver Hard Mode
	case 24:	timeBonusBase = 19000;	break; // Iblis
	case 280:	timeBonusBase = 20000;	break; // Iblis Hard Mode
	default:	break;
	}

	// Overwrite rank table on special stages (boss stages, missions etc.)
	switch (stageID)
	{
	case 22:	// Silver
	case 278:	// Silver Hard Mode
	case 24:	// Iblis
	case 280:	// Iblis Hard Mode
	{
		scoreTable = ScoreTable{ 30000,27500,25000,5000 };
		timeBonusRate = 80;
		break;
	}
	default: break;
	}

	// Calculate final score
	static ResultData data;
	m_timeBonus = max(timeBonusBase - (int)floorf(ScoreManager::m_currentTime) * timeBonusRate, 0 );
	int ringBonus = *Common::GetPlayerRingCount() * 100;
	data.m_score = m_pCScoreManager->m_score + m_timeBonus + ringBonus;

	// Get current rank and score to next rank (if applicable)
	if (data.m_score > scoreTable.m_scoreS)
	{
		data.m_rank = RankType::RT_S;
		data.m_nextRankScore = 0;
	}
	else if (data.m_score > scoreTable.m_scoreA)
	{
		data.m_rank = RankType::RT_A;
		data.m_nextRankScore = scoreTable.m_scoreS - data.m_score;
	}
	else if (data.m_score > scoreTable.m_scoreB)
	{
		data.m_rank = RankType::RT_B;
		data.m_nextRankScore = scoreTable.m_scoreA - data.m_score;
	}
	else if (data.m_score > scoreTable.m_scoreC)
	{
		data.m_rank = RankType::RT_C;
		data.m_nextRankScore = scoreTable.m_scoreB - data.m_score;
	}
	else
	{
		data.m_rank = RankType::RT_D;
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
		, data.m_rank == RankType::RT_S ? "S" 
		: (data.m_rank == RankType::RT_A ? "A" 
		: (data.m_rank == RankType::RT_B ? "B" 
		: (data.m_rank == RankType::RT_C ? "C" : "D")))
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
	// Always clear texture first
	if (m_bonusTexture)
	{
		m_bonusTexture = nullptr;
	}

	// Only draw comment if there not already one
	if (m_bonusDrawTimer == 0.0f)
	{
		switch (type)
		{
		case BCT_Great:		m_bonusTexture = m_bonus_Great;   break;
		case BCT_Radical:	m_bonusTexture = m_bonus_Radical; break;
		}
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
		m_bonus = 0;
		m_bonusDrawTimer = 0.0f;
		m_rainbowRingChain = 0;
		m_enemyChain = 0;
		return;
	}

	if (m_bonus > 0 && m_bonusDrawTimer > 0)
	{
		static bool visible = true;
		ImGui::Begin("Bonus", &visible, UIContext::m_hudFlags);
		{
			std::string const bonusStr = std::to_string(m_bonus);
			ImVec2 size = ImGui::CalcTextSize(bonusStr.c_str());
			ImGui::SetWindowPos(ImVec2((float)*BACKBUFFER_WIDTH * 0.8f - size.x / 2, (float)*BACKBUFFER_HEIGHT * 0.295f - size.y / 2));
			ImGui::Text(bonusStr.c_str());
		}
		ImGui::End();

		if (m_bonusTexture)
		{
			ImGui::Begin("BonusComment", &visible, UIContext::m_hudFlags);
			{
				float sizeX = *BACKBUFFER_WIDTH * 300.0f / 1280.0f;
				float sizeY = *BACKBUFFER_HEIGHT * 50.0f / 720.0f;
				float posX = 0.645f;
				float posY = 0.347f;
				if (m_bonusDrawTimer > 3.9f)
				{
					posX = 1.0f - (1.0f - posX) * ((4.0f - m_bonusDrawTimer) / 0.1f);
				}

				ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * posX, *BACKBUFFER_HEIGHT * posY));
				ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
				ImGui::Image(m_bonusTexture, ImVec2(sizeX, sizeY));
			}
			ImGui::End();
		}
	}
}
