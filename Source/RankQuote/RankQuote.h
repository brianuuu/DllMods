/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Play rank quotes at result screen
//	Requirement: Add 6 voices (S-E rank) with rank sfx with ID [40000-40005]
//			     and rank slam sfx with ID [1000041-1000046]
//			     in the .csb in SonicVoice.ar.00
/*----------------------------------------------------------*/

#pragma once

enum ResultRankType : int
{
	D,
	C,
	B,
	A,
	S,
	E
};

struct ResultData
{
	int m_score;
	ResultRankType m_rank;
	ResultRankType m_perfectRank;
	int m_nextRankTime;
	float m_totalProp;	// result progress bar (time prop + ring prop) 
	float m_timeProp;	// result progress bar (time prop)
};

class RankQuote
{
public:
	static void applyPatches();
};

