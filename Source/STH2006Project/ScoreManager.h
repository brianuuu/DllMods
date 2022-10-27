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
	ST_enemyBonus,
	ST_boss,
	ST_missionDashRing,
	ST_barricade,

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
	case ST_enemyBonus:		return "Enemy (Bonus)";
	case ST_boss:			return "Boss";
	case ST_missionDashRing:return "Mission Dash Ring";
	case ST_barricade:		return "Barricade";
	default:				return "";
	}
}

enum ResultRankType : uint32_t
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
	ResultRankType m_rank;
	ResultRankType m_perfectRank;
	int m_nextRankScore; // Used to be time in milliseconds, but we repurpose it
	float m_totalProp;	// result progress bar (time prop + ring prop) 
	float m_timeProp;	// result progress bar (time prop)
};

struct CScoreManager
{
	INSERT_PADDING(0xA8);
	uint32_t m_score;
};

enum BonusCommentType : uint32_t
{
	BCT_Great = 0,
	BCT_Radical,
	BCT_NoDraw
};

class ScoreManager
{
public:
	static void applyPatches();
	static void applyPatches_ScoreGensSystem();
	static void applyPatches_InternalSystem();
	static void applyPostInit();
	static void setExternalIni(bool reset);

	static void __fastcall addScore(ScoreType type, uint32_t* This = nullptr);
	static ResultData* calculateResultData();
	static float getScoreProp(ScoreTable const& scoreTable, int score);
	static float getPropBetween(int min, int max, int num);

	// Rainbow ring bonus
	static uint32_t m_rainbowRingChain;
	static uint32_t calculateRainbowRingChainBonus();

	// Enemy bonus
	static uint32_t m_enemyChain;
	static uint32_t m_enemyCount;
	static float m_enemyChainTimer;
	static void addEnemyChain(uint32_t* This, void* message);
	static uint32_t calculateEnemyChainBonus();

	// Bonus GUI
	static uint32_t m_bonus; // interal bonus counter
	static uint32_t m_bonusToDraw; // bonus to draw on screen
	static float m_bonusTimer;
	static float m_bonusDrawTimer;
	static IUnknown** m_bonusTexture;
	static IUnknown** m_bonusTexturePrev;
	static void notifyDraw(BonusCommentType type);
	static void draw();
	static void clearDraw();

	// Textures
	static bool initTextures();
	static IUnknown* m_bonus_Great;
	static IUnknown* m_bonus_Radical;
	~ScoreManager()
	{
		if (m_bonus_Great)   m_bonus_Great->Release();
		if (m_bonus_Radical) m_bonus_Radical->Release();
	}

	// Common members
	static bool m_enabled;
	static bool m_internalSystem;
	static std::unordered_set<uint32_t*> m_savedObjects;

	// Internal system members
	static bool m_externalHUD;
	static uint32_t m_scoreLimit;
	static std::string m_scoreFormat;
	static CScoreManager* m_pCScoreManager;
	static bool m_updateScoreHUD;
	static float m_currentTime;
	static int m_timeBonus;
};

