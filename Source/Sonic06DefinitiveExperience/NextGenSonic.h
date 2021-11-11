/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Replicate Sonic 06 Physics for Sonic & Sonic + Elise
/*----------------------------------------------------------*/

#include "NextGenPhysics.h"

#pragma once
class NextGenSonic
{
public:
	static void setAnimationSpeed_Sonic(NextGenAnimation& data);
	static void setAnimationSpeed_Elise(NextGenAnimation& data);

	static bool m_isElise;
	static void applyPatches();

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
	static float m_slidingSpeed;
	static bool __fastcall applySpindashImpulse(void* context);
	static bool __fastcall applySlidingHorizontalTargetVel(void* context);

	// Action
	static float m_bHeldTimer;
	static bool m_enableAutoRunAction;
	static bool bActionHandlerImpl();
	static void getActionButtonStates(bool& bDown, bool& bPressed, bool& bReleased);

	// Elise Shield
	static bool m_isShield;
	static float m_shieldDecRate;
	static float m_shieldRechargeRate;
	static float m_shieldNoChargeTime;
	static float m_shieldNoChargeDelay;
	static Sonic::EKeyState m_shieldButton;
};

