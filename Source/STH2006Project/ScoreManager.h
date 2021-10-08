/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: 
/*----------------------------------------------------------*/

#pragma once

enum ScoreType
{
	ST_INVALID,

	ST_ring,
	ST_5ring,
	ST_10ring,
	ST_20ring,
	ST_rainbow,
	ST_physics,
	ST_itembox,
	ST_enemySmall,
	ST_enemyMedium,
	ST_enemyLarge,
	ST_enemyStealth,
};

class ScoreManager
{
public:
	static void applyPatches();
	static void applyPatches_ScoreGensSystem();
	static void applyPatches_InternalSystem();
	static void overrideScoreTable(std::string const& iniFile);

	static void addScore(ScoreType type);

	// Members
	static bool m_enabled;
	static bool m_internalSystem;
	static bool m_externalHUD;
	static std::string m_scoreFormat;
};

