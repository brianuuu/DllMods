/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2024
//	Description: Replicate Sonic 06 Physics for Shadow
/*----------------------------------------------------------*/

#include "NextGenPhysics.h"

#pragma once
class NextGenShadow
{
public:
	static void setAnimationSpeed_Shadow(NextGenAnimation& data);
	static void applyPatches();

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

	// X Action
	static float m_xHeldTimer;
	static bool m_enableAutoRunAction;
	static bool bActionHandlerImpl();

	// Chaos Boost
	static uint8_t m_chaosBoostLevel;
	static float m_chaosMaturity;
	static bool ShouldPlayJetEffect();
	static void SetJetEffectVisible(bool visible);
	static void SetChaosBoostModelVisible(bool visible);
	static void SetChaosBoostLevel(uint8_t level, bool notifyHUD);
	static void AddChaosMaturity(float amount);
};

