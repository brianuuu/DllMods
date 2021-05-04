/*----------------------------------------------------------*/
//	Author: brianuuuSonic
//	Year: 2021
//	Description: Play rank quotes at result screen
//	Requirement: Add 5 voices with rank sfx with ID [40000-40004]
//			     in the .csb in SonicVoice.ar.00
/*----------------------------------------------------------*/

#pragma once

class RankQuote
{
	static bool m_enabled;

public:
	static int m_rank;
	static int m_rankSfxID;
	static bool m_playRankVoice;

	static void applyPatches();
};

