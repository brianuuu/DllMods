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
	static int m_tripleKickCount;
	static bool m_tripleKickBuffered;
	static bool m_tripleKickShockWaveSpawned;
	static void NextTripleKick(Sonic::Player::CPlayerSpeedContext* context);

	// Spindash/Sliding
	static bool m_isSpindash;
	static bool m_isSliding;
	static float m_slidingTime;
	static float m_slidingSpeed;
	static bool __fastcall applySpindashImpulse(void* context);
	static bool __fastcall applySlidingHorizontalTargetVel(void* context);

	// Chaos Attack
	static Eigen::Vector3f m_holdPosition;
	static int m_chaosAttackCount;
	static bool m_chaosAttackBuffered;
	static bool m_chaosSnapActivated;
	static bool m_chaosSnapNoDamage;

	// X Action
	static float m_xHeldTimer;
	static bool m_enableAutoRunAction;
	static bool bActionHandlerImpl();

	// Chaos Boost
	static uint8_t m_chaosBoostLevel;
	static float m_chaosMaturity;
	static bool ShouldPlayJetEffect();
	static void SetJetEffectVisible(bool visible, hh::mr::CSingleElement* pModel, bool isSuper);
	static bool IsModelVisible();
	static void SetChaosBoostModelVisible(bool visible, bool allInvisible = false);
	static void SetChaosBoostLevel(uint8_t level, bool notifyHUD);
	static bool CheckChaosBoost();
	static bool CheckChaosControl();
	static bool CheckChaosSnapTarget();
	static void AddChaosMaturity(float amount);
	static bool AirActionCheck();

	// Chaos Spear
	struct TargetData
	{
		uint32_t m_actorID;
		float m_dist;
		uint32_t m_priority;
	};
	static std::vector<TargetData> m_targetData;
	static void AddTargetData(uint32_t actorID, float dist, uint32_t priority);

	static enum class OverrideType
	{
		SH_None,
		SH_SpearWait,
		SH_SpearShot,
		SH_ChaosBoost,
		SH_ChaosBlastWait,
		SH_ChaosBlast,
	} m_overrideType;
};

