/*----------------------------------------------------------*/
//	Author: brianuuuSonic
//	Year: 2021
//	Description: Play rank quotes at result screen
//				 This is a modified version for Unleashed voice only
//	Requirement: Add 6 rank slap sfx with ID [40000-40005] (S to E rank)
//				 Also add rank quotes with ID [40010-40015]
//			     in the .csb in SonicVoice.ar.00
/*----------------------------------------------------------*/

#pragma once

struct MsgGetAnimationInfo
{
	INSERT_PADDING(0x14);
	char* name;
	float frame;
};

class RankQuote
{
public:
	static uint32_t m_rank;
	static uint32_t m_rankSfxID;
	static bool m_playRankSfx;

	static void applyPatches();
};

