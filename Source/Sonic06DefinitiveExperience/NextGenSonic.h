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
	static void applyPatchesPostInit();

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

	// X Action
	static float m_xHeldTimer;
	static bool m_enableAutoRunAction;
	static bool bActionHandlerImpl();

	// Elise Shield
	static bool m_isShield;
	static float m_shieldDecRate;
	static float m_shieldRechargeRate;
	static float m_shieldNoChargeTime;
	static float m_shieldNoChargeDelay;

	// Sonic's Gems
	static S06HUD_API::SonicGemType m_sonicGemType;
	static Eigen::Vector3f m_gemHoldPosition;
	static bool m_whiteGemEnabled;
	static bool m_purpleGemEnabled;
	static float m_purpleGemBlockTimer;
	static int m_purpleGemJumpCount;
	static bool m_greenGemEnabled;
	static bool m_skyGemEnabled;
	static bool m_skyGemLaunched;
	static bool m_skyGemCancelled;
	static void ChangeGems(S06HUD_API::SonicGemType oldType, S06HUD_API::SonicGemType newType);
	static void DisableGem(S06HUD_API::SonicGemType type);
	static bool GetSkyGemHitLocation
	(
		Eigen::Vector4f& o_pos,
		Hedgehog::Math::CVector position,
		Hedgehog::Math::CVector velocity,
		float const simRate,
		float const maxDist
	);
};

