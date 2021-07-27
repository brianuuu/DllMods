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

	static bool m_isFlipStop;

	static bool m_isStomping;
	static bool m_bounced;
	static bool m_isSquatKick;
	static Eigen::Vector3f m_squatKickDir;
	static Eigen::Vector3f m_squatKickVelocity;
	static void bounceBraceletImpl();

	static bool m_isSpindash;
	static bool m_isSliding2D;
	static float m_slidingTime;
	static bool __fastcall applySpindashImpulse(void* context);
	static bool __fastcall applySlidingHorizontalTargetVel(void* context);

	static void getActionButtonStates(bool& bDown, bool& bPressed, bool& bReleased);

	static float m_bHeldTimer;
	static bool bActionHandlerImpl();
};

