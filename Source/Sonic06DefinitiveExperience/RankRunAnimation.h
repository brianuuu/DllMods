/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Play custom animation at the beginning of result screen
//  Requirement: sn_result_run.anm.hkx and sn_result_run_loop.anm.hkx
//				 exist in Sonic.ar.00
//				 sn_result_run.anm.hkx should have ~38 extra frames at start (60fps)
/*----------------------------------------------------------*/

#pragma once

class RankRunAnimation
{
public:
	static void applyPatches();
};

