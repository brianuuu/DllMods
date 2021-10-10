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
	ST_rainbow2,
	ST_rainbow3,
	ST_rainbow4,
	ST_rainbow5,
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
	case ST_rainbow2:		return "Rainbow Ring Lv2";
	case ST_rainbow3:		return "Rainbow Ring Lv3";
	case ST_rainbow4:		return "Rainbow Ring Lv4";
	case ST_rainbow5:		return "Rainbow Ring Lv5";
	case ST_physics:		return "Physics";
	case ST_itembox:		return "Itembox";
	case ST_enemySmall:		return "Enemy (Small)";
	case ST_enemyMedium:	return "Enemy (Medium)";
	case ST_enemyLarge:		return "Enemy (Large)";
	case ST_enemyStealth:	return "Enemy (Stealth)";
	default:				return "";
	}
}

enum RankType : uint32_t
{
	RT_D,
	RT_C,
	RT_B,
	RT_A,
	RT_S,
};

struct ScoreTable
{
	int m_scoreS;
	int m_scoreA;
	int m_scoreB;
	int m_scoreC;
};

struct ResultData
{
	int m_score;
	RankType m_rank;
	RankType m_perfectRank;
	int m_nextRankScore; // Used to be time in milliseconds, but we repurpose it
	float m_totalProp;	// result progress bar (time prop + ring prop) 
	float m_timeProp;	// result progress bar (time prop)
};

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
	static ResultData* calculateResultData();
	static float getScoreProp(ScoreTable const& scoreTable, int score);
	static float getPropBetween(int min, int max, int num);

	// Common members
	static bool m_enabled;
	static bool m_internalSystem;
	static uint32_t m_rainbowRingChain;
	static std::unordered_set<uint32_t*> m_savedObjects;

	// Internal system members
	static bool m_externalHUD;
	static uint32_t m_scoreLimit;
	static std::string m_scoreFormat;
	static CScoreManager* m_pCScoreManager;
	static bool m_updateScoreHUD;
	static float m_currentTime;
};

