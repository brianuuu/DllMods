/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Replicate Sonic 06 Physics
/*----------------------------------------------------------*/

#pragma once

struct NextGenAnimation
{
	// Note: All animations have 81 frames [0-80]
	// playbackSpeed: how fast animation plays
	// speedFactor: how much distance to play one loop, -1.0 to use constant playbackSpeed

	float walk_playbackSpeed = 1.0f;
	float walk_speedFactor = 1.35f;
	float walkFast_playbackSpeed = 1.0f;
	float walkFast_speedFactor = 2.0f;

	float jog_playbackSpeed = 1.0f;
	float jog_speedFactor = 3.3f;

	float run_playbackSpeed = 1.0f;
	float run_speedFactor = 9.0f;

	float dash_playbackSpeed = 1.0f;
	float dash_speedFactor = 12.0f;

	float jet_playbackSpeed = 1.0f;
	float jet_speedFactor = 13.0f;
	float jetWall_playbackSpeed = 1.0f;
	float jetWall_speedFactor = 20.0f;

	float boost_playbackSpeed = 1.0f;
	float boost_speedFactor = 13.0f;
	float boostWall_playbackSpeed = 1.0f;
	float boostWall_speedFactor = 20.0f;
};

class NextGenPhysics
{
public:
	static void applyPatches();

	// Common
	static float m_homingDownSpeed;
	static NextGenAnimation m_animationData;
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
	static float m_slidingSpeed;
	static bool __fastcall applySpindashImpulse(void* context);
	static bool __fastcall applySlidingHorizontalTargetVel(void* context);

	// Action
	static float m_bHeldTimer;
	static bool bActionHandlerImpl();
	static void getActionButtonStates(bool& bDown, bool& bPressed, bool& bReleased);

};

