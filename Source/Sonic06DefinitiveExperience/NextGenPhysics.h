/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Replicate Sonic 06 Physics
/*----------------------------------------------------------*/

#pragma once
class NextGenPhysics
{
public:
	static void applyPatches();

	static bool m_isStomping;
	static bool m_bounced;
	static bool m_isSquatKick;
	static void bounceBraceletImpl();

	static bool m_isSpindash;
	static float m_slidingTime;
	static bool __fastcall applySpindashImpulse(void* context);
	static bool __fastcall applySlidingHorizontalTargetVel(void* context, bool is2D);
};

