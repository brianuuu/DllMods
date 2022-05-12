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
	static void applyPatchesPostInit();
	static void applyNoTrickPatches();

	// Common 06 Physics
	static float m_homingDownSpeed;
	static NextGenAnimation m_animationData;
	static void applyCharacterAnimationSpeed();

	static float const c_funcMaxTurnRate;
	static float const c_funcTurnRateMultiplier;
	static void applyCSonicRotationAdvance
	(
		void* This, 
		float* targetDir, 
		float turnRate1 = c_funcMaxTurnRate,
		float turnRateMultiplier = c_funcTurnRateMultiplier,
		bool noLockDirection = true, 
		float turnRate2 = c_funcMaxTurnRate
	);

	static void getActionButtonStates(bool& bDown, bool& bPressed, bool& bReleased);
	static bool checkUseLightSpeedDash();
	static void keepConstantHorizontalVelocity(float hSpeed);

	static void fixGenerationsLayout(char* pData);
};

