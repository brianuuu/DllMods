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

	// Common
	static float m_homingDownSpeed;
	static void applyCharacterAnimationSpeed();

	// Stomping
	static bool m_bounced;
	static bool m_isStomping;
	static void bounceBraceletImpl();

	// Squat Kick
	static bool m_isSquatKick;
	static bool m_isBrakeFlip;
	static Eigen::Vector3f m_brakeFlipDir;
	static float m_squatKickSpeed;

	// Spindash/Sliding
	static bool m_isSpindash;
	static bool m_isSliding;
	static float m_slidingTime;
	static bool __fastcall applySpindashImpulse(void* context);
	static bool __fastcall applySlidingHorizontalTargetVel(void* context);

	// Action
	static float m_bHeldTimer;
	static bool bActionHandlerImpl();
	static void getActionButtonStates(bool& bDown, bool& bPressed, bool& bReleased);

};
