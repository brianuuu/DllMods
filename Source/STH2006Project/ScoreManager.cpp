#include "ScoreManager.h"
#include "Configuration.h"
#include "ChaosEnergy.h"

bool ScoreManager::m_enabled = false;
bool ScoreManager::m_internalSystem = false;
bool ScoreManager::m_externalHUD = false;
std::string ScoreManager::m_scoreFormat = "%01d";
std::unordered_set<uint64_t> ScoreManager::m_savedObjects;

HOOK(int, __fastcall, ScoreManager_MsgRestartStage, 0xE76810, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::reset();
	return originalScoreManager_MsgRestartStage(This, Edx, message);
}

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
	default: ScoreManager::addScore(ScoreType::ST_itembox, This); break; // 1up
	case 6:  ScoreManager::addScore(ScoreType::ST_5ring, This); break; // 5 ring
	case 7:  ScoreManager::addScore(ScoreType::ST_10ring, This); break; // 10 ring
	case 8:	 ScoreManager::addScore(ScoreType::ST_20ring, This); break; // 20 ring
	}

	originalScoreManager_GetItem(This, Edx, message);
}

HOOK(int, __fastcall, ScoreManager_GetRing, 0x10534B0, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_ring, This);
	return originalScoreManager_GetRing(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_GetSuperRing, 0x11F2F10, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_10ring, This);
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

uint32_t ScoreManager_GetRainbowReturnAddress = 0x115A8FC;
void __declspec(naked) ScoreManager_GetRainbow()
{
	__asm
	{
		push	esi
		mov		ecx, ST_rainbow
		mov		edx, 0
		call	ScoreManager::addScore
		pop		esi

		mov     eax, [esi + 0BCh]
		jmp		[ScoreManager_GetRainbowReturnAddress]
	}
}

HOOK(void, __fastcall, ScoreManager_EnemyGunner, 0xBAA2F0, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemySmall, This);
	originalScoreManager_EnemyGunner(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyStingerLancer, 0xBB01B0, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(This[104] ? ScoreType::ST_enemySmall : ScoreType::ST_enemyMedium, This);
	originalScoreManager_EnemyStingerLancer(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyBuster, 0xB82900, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemyMedium, This);
	originalScoreManager_EnemyBuster(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyFlyer, 0xBA6450, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemySmall, This);
	originalScoreManager_EnemyFlyer(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemySearcherHunter, 0xBDC110, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(*(bool*)((uint32_t)This + 0x17C) ? ScoreType::ST_enemySmall : ScoreType::ST_enemyMedium, This);
	originalScoreManager_EnemySearcherHunter(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyLiner, 0xBC7440, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemySmall, This);
	originalScoreManager_EnemyLiner(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyBomber, 0xBCB9A0, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemySmall, This);
	originalScoreManager_EnemyBomber(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyRounder, 0xBCF5E0, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemySmall, This);
	originalScoreManager_EnemyRounder(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyTaker, 0xBA3140, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemySmall, This);
	originalScoreManager_EnemyTaker(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyBiter, 0xB86850, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemySmall, This);
	originalScoreManager_EnemyBiter(This, Edx, message);
}

HOOK(void, __fastcall, ScoreManager_EnemyCrawler, 0xB99B80, uint32_t* This, void* Edx, void* message)
{
	ScoreManager::addScore(ScoreType::ST_enemyMedium, This);
	originalScoreManager_EnemyCrawler(This, Edx, message);
}

std::string externIniPath;
void ScoreManager::applyPatches(std::string const& modDir)
{
	// Must enable Score Generations, whether for internal score system or not
	if (!Common::IsModEnabled("Score Generations", &externIniPath))
	{
		return;
	}

	// TODO: Ensure Score Generations is loaded LATER/higher priority than this mod (currently impossible with 06 HUD)
	if (GetModuleHandle(TEXT("ScoreGenerations.dll")) != nullptr)
	{
		
	}

	printf("[ScoreManager] STH2006 score system enabled\n");
	m_enabled = true;

	// Reset stage
	INSTALL_HOOK(ScoreManager_MsgRestartStage);

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
		applyPatches_InternalSystem(modDir);
	}
	else
	{
		applyPatches_ScoreGensSystem();
	}
}

void ScoreManager::applyPatches_ScoreGensSystem()
{
	
}

void ScoreManager::applyPatches_InternalSystem(std::string const& modDir)
{
	// Disable extern .dll in mod.ini
	setExternalIni(modDir, false);

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

	// Enable CScoreManager in regular stages
	INSTALL_HOOK(ScoreManager_GameplayManagerInit);
}

void ScoreManager::applyPostInit(std::string const& modDir)
{
	if (!m_enabled) return;

	if (m_internalSystem)
	{
		// Reset extern .dll in mod.ini
		setExternalIni(modDir, true);

		// We shouldn't have loaded ScoreGenerations.dll
		if (GetModuleHandle(TEXT("ScoreGenerations.dll")) != nullptr)
		{
			MessageBox(NULL, L"An error occured when initializing internal score system!", L"STH2006 Project", MB_ICONERROR);
			exit(-1);
		}
	}
	else
	{
		std::string iniFile = modDir + "ScoreGenerations.ini";
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

void ScoreManager::setExternalIni(std::string const& modDir, bool reset)
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

void ScoreManager::reset()
{
	printf("[ScoreSystem] RESTARTED!\n");
	m_savedObjects.clear();

	if (m_internalSystem)
	{
		// TODO: Set and draw 0 score
	}
}

void ScoreManager::addScore(ScoreType type, uint32_t* This)
{
	// Prevent same object and add score multiple times
	if (This)
	{
		// Combine this ptr with matrix node transform ptr
		uint64_t key = (uint64_t)This + ((uint64_t)This[46] << 32);
		if (m_savedObjects.count(key))
		{
			printf("[ScoreSystem] WARNING: Duplicated score\n");
			return;
		}

		m_savedObjects.insert(key);
	}

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
	default: return;
	}

	printf("[ScoreSystem] Score +%d \tType: %s\n", score, GetScoreTypeName(type));
	if (m_internalSystem)
	{
		// TODO:
	}
	else
	{
		ScoreGenerationsAPI::AddScore(score);
	}
}
