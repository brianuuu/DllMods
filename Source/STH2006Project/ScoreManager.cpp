#include "ScoreManager.h"
#include "Configuration.h"
#include "ChaosEnergy.h"

bool ScoreManager::m_enabled = false;
bool ScoreManager::m_internalSystem = false;
bool ScoreManager::m_externalHUD = false;
std::string ScoreManager::m_scoreFormat = "%01d";

HOOK(int*, __fastcall, ScoreManager_GameplayManagerInit, 0xD00F70, void* This, void* Edx, int* a2)
{
	static char const* scoreHUD = "ui_gameplay_score";
	static char const* defaultHUD = (char*)0x0168E328;

	uint32_t stageID = Common::GetCurrentStageID();
	if (stageID <= 17 ||	// 0 main stages
		stageID == 18)		// Casino Night (unused but that's original code)
	{
		// Enable CScoreManager
		WRITE_MEMORY(0xD017A7, uint8_t, (uint8_t)stageID);
		WRITE_MEMORY(0x109C1DA, uint8_t, 0x90, 0x90);

		// Use fixed score HUD if not using external
		if (!ScoreManager::m_externalHUD)
		{
			WRITE_MEMORY(0x109D669, char*, scoreHUD);
		}
	}
	else
	{
		// Original code
		WRITE_MEMORY(0xD017A7, uint8_t, 0x12);
		WRITE_MEMORY(0x109C1DA, uint8_t, 0x74, 0x78);
		WRITE_MEMORY(0x109D669, char*, defaultHUD);
	}

	return originalScoreManager_GameplayManagerInit(This, Edx, a2);
}

HOOK(void, __fastcall, ScoreManager_GetItem, 0xFFF810, uint32_t* This, void* Edx, void* message)
{
	switch (This[71])
	{
	default: ScoreManager::addScore(ScoreType::ST_itembox); break; // 1up
	case 6:  ScoreManager::addScore(ScoreType::ST_5ring); break; // 5 ring
	case 7:  ScoreManager::addScore(ScoreType::ST_10ring); break; // 10 ring
	case 8:	 ScoreManager::addScore(ScoreType::ST_20ring); break; // 20 ring
	}

	originalScoreManager_GetItem(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_GetRing, 0x10534B0, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_ring);
	originalScoreManager_GetRing(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_GetSuperRing, 0x11F2F10, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_10ring);
	originalScoreManager_GetSuperRing(This, Edx, message);
}

HOOK(void, __stdcall, ScoreManager_GetPhysics, 0xEA49B0, uint32_t This, int a2, void* a3, bool a4)
{
	static std::vector<std::string> filterList =
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
				ScoreManager::addScore(fakeEnemyType == 1 ? ScoreType::ST_enemySmall : ScoreType::ST_enemyMedium);
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
					ScoreManager::addScore(ScoreType::ST_physics);
				}
			}
		}
	}

	originalScoreManager_GetPhysics(This, a2, a3, a4);
}

uint32_t ScoreManager_GetRainbowReturnAddress = 0x115A8FC;
void __declspec(naked) ScoreManager_GetRainbow()
{
	__asm
	{
		push	esi
		mov		ecx, ST_rainbow
		call	ScoreManager::addScore
		pop		esi

		mov     eax, [esi + 0BCh]
		jmp		[ScoreManager_GetRainbowReturnAddress]
	}
}

void ScoreManager::applyPatches()
{
	if (!Common::IsModEnabled("Score Generations") && !m_internalSystem)
	{
		return;
	}

	printf("[ScoreManager] STH2006 score system enabled\n");
	m_enabled = true;

	// Common score hooks
	INSTALL_HOOK(ScoreManager_GetItem);
	INSTALL_HOOK(ScoreManager_GetRing);
	INSTALL_HOOK(ScoreManager_GetSuperRing);
	INSTALL_HOOK(ScoreManager_GetPhysics);
	WRITE_JUMP(0x115A8F6, ScoreManager_GetRainbow);

	if (m_internalSystem)
	{
		applyPatches_InternalSystem();
	}
	else
	{
		applyPatches_ScoreGensSystem();
	}

}

void ScoreManager::applyPatches_ScoreGensSystem()
{
	
}

void ScoreManager::applyPatches_InternalSystem()
{
	// TODO: disable extern

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
				m_scoreFormat = configReader.Get("Appearance", "scoreFormat", "%01d");
				break;
			}
		}
	}

	// Set score format
	WRITE_MEMORY(0x1095D7D, char*, m_scoreFormat.c_str());

	// TODO: Reset score to 0 at MsgRestartStage

	// Enable CScoreManager in regular stages
	INSTALL_HOOK(ScoreManager_GameplayManagerInit);
}

void ScoreManager::overrideScoreTable(std::string const& iniFile)
{
	if (!m_enabled || m_internalSystem) return;

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

void ScoreManager::addScore(ScoreType type)
{
	int score = 0;
	switch (type)
	{
	case ST_ring:			score = 10;		break;
	case ST_5ring:			score = 50;		break;
	case ST_10ring:			score = 100;	break;
	case ST_20ring:			score = 200;	break;
	case ST_rainbow:		score = 1000;	break;
	case ST_physics:		score = 20;		break;
	case ST_itembox:		score = 200;	break;
	case ST_enemySmall:		score = 100;	break;
	case ST_enemyMedium:	score = 300;	break;
	case ST_enemyLarge:		score = 1000;	break;
	case ST_enemyStealth:	score = 1500;	break;
	default: break;
	}

	if (m_internalSystem)
	{
		// TODO:
	}
	else
	{
		ScoreGenerationsAPI::AddScore(score);
	}
}
