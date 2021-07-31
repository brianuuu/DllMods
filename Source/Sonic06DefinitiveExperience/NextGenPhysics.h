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

	static bool lockJumpStandAnimationImpl();

	static bool m_isBrakeFlip;

	static bool m_isStomping;
	static bool m_bounced;
	static bool m_isSquatKick;
	static Eigen::Vector3f m_brakeFlipDir;
	static float m_squatKickSpeed;
	static void bounceBraceletImpl();

	static bool m_isSpindash;
	static bool m_isSliding;
	static float m_slidingTime;
	static bool __fastcall applySpindashImpulse(void* context);
	static bool __fastcall applySlidingHorizontalTargetVel(void* context);

	static void getActionButtonStates(bool& bDown, bool& bPressed, bool& bReleased);

	static float m_bHeldTimer;
	static bool bActionHandlerImpl();

	static void applyCharacterAnimationSpeed();
};

