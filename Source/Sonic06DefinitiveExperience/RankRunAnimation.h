/*----------------------------------------------------------*/
//	Author: brianuuuSonic
//	Year: 2021
//	Description: Play custom animation at the beginning of result screen
//  Requirement: sn_result_run.anm.hkx and sn_result_run_loop.anm.hkx
//				 exist in Sonic.ar.00
/*----------------------------------------------------------*/

#pragma once

class RankRunAnimation
{
	static bool m_enabled;

public:
	static void applyPatches();
};

