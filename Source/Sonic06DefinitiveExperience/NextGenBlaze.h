/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2022
//	Description: Replicate Sonic 06 Physics for Blaze
/*----------------------------------------------------------*/

#include "NextGenPhysics.h"

#pragma once
class NextGenBlaze
{
public:
	static void setAnimationSpeed_Blaze(NextGenAnimation& data);
	static void applyPatches();

	// Double Jump
	static bool m_doubleJumpEnabled;
	static bool __fastcall doubleJumpImpl(bool pressedJump);

	// X Action
	static float m_xHeldTimer;
	static float m_homingVSpeed;
	static bool m_dummyHoming;
	static bool bActionHandlerImpl();

	// Spinning Claw
	static float m_spinningClawSpeed;
	static float m_spinningClawTime;
};

