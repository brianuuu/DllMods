/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Apply stage specific codes
/*----------------------------------------------------------*/

#pragma once

class StageManager
{
public:
	static void applyPatches();

	static bool m_wallJumpStart;
	static float m_wallJumpTime;
	static void __fastcall getIsWallJumpImpl(float* outOfControl);

	static bool m_waterRunning;
};

