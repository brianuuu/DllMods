/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: 
/*----------------------------------------------------------*/

#pragma once

enum ScoreType : uint32_t
{
	ST_INVALID = 0,

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

	ST_COUNT
};

inline char const* GetScoreTypeName(ScoreType type)
{
	switch (type)
	{
	case ST_ring: 			return "Ring";
	case ST_5ring:			return "5 Ring";
	case ST_10ring:			return "10 Ring";
	case ST_20ring:			return "20 Ring";
	case ST_rainbow:		return "Rainbow Ring";
	case ST_physics:		return "Physics";
	case ST_itembox:		return "Itembox";
	case ST_enemySmall:		return "Enemy (Small)";
	case ST_enemyMedium:	return "Enemy (Medium)";
	case ST_enemyLarge:		return "Enemy (Large)";
	case ST_enemyStealth:	return "Enemy (Stealth)";
	default:				return "";
	}
}

struct MsgSetPinballHud
{
	INSERT_PADDING(0x10);
	uint32_t m_flag;
	uint32_t m_score;
	INSERT_PADDING(0x8);
};

struct CScoreManager
{
	INSERT_PADDING(0xA8);
	uint32_t m_score;

};

class ScoreManager
{
public:
	static void applyPatches(std::string const& modDir);
	static void applyPatches_ScoreGensSystem();
	static void applyPatches_InternalSystem(std::string const& modDir);
	static void applyPostInit(std::string const& modDir);
	static void setExternalIni(std::string const& modDir, bool reset);

	static void __fastcall addScore(ScoreType type, uint32_t* This = nullptr);

	// Members
	static bool m_enabled;
	static bool m_internalSystem;
	static bool m_externalHUD;
	static uint32_t m_scoreLimit;
	static std::string m_scoreFormat;
	static CScoreManager* m_pCScoreManager;
	static bool m_updateScoreHUD;
	static std::unordered_set<uint64_t> m_savedObjects;
};

