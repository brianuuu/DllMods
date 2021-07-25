#pragma once

#define PI 3.141592
#define PI_F 3.141592f

typedef void CSonicContext;
CSonicContext** const PLAYER_CONTEXT = (CSonicContext**)0x1E5E2F0;
CSonicContext** const pModernSonicContext = (CSonicContext**)0x1E5E2F8;
CSonicContext** const pClassicSonicContext = (CSonicContext**)0x1E5E304;
CSonicContext** const pSuperSonicContext = (CSonicContext**)0x1E5E310;

uint32_t const CStringConstructor = 0x6621A0;
uint32_t const CStringDestructor = 0x661550;

struct MsgGetHudPosition
{
    INSERT_PADDING(0x10);
    Eigen::Vector3f m_position;
    INSERT_PADDING(0x4);
    float m_speed; // just a guess?
    uint32_t m_type;
};

struct MsgSetPosition
{
    INSERT_PADDING(0x10);
    Eigen::Vector3f m_position;
    INSERT_PADDING(0x4);
};

struct MsgSetRotation
{
    INSERT_PADDING(0x10);
    Eigen::Quaternionf m_rotation;
};

struct CSonicStateFlags
{
	bool EarthGround;
	bool HeadToVelocity;
	bool HeadUpOnly;
	bool SlowHeadToUpDirection;
	bool OutOfControl;
	bool NoLandOutOfControl;
	bool DisableForwardPathInterpolation;
	bool ChangePath;
	bool ChangePathCameraChange;
	bool WallWalkJump;
	bool SupportWalkOnCeiling;
	bool NotifyWalkOnCeiling;
	bool AlwaysDownForce;
	bool DisableDownForce;
	bool Dead;
	bool Goal;
	bool Boost;
	bool FloatingBoost;
	bool StartingBoost;
	bool EndBoost;
	bool DummyBoost;
	bool EndDummyBoost;
	bool Homing;
	bool EnableHomingAttack;
	bool EnableHomingAttackOnDiving;
	bool EnableHomingAttackOutOfControl;
	bool DisableAirBoost;
	bool EnableAirOnceAction;
	bool DisableStomping;
	bool ForceShoesSliding;
	bool DisableShoesSliding;
	bool OnWater;
	bool OnNoDeadWater;
	bool OnAboveWater;
	bool OnSurfaceWater;
	bool OnShallowWater;
	bool OnWaterSeEnabled;
	bool TerrainCollisionEnable;
	bool AirOutOfControl;
	bool NoPadStopWalk;
	bool DisableAirAutoDec;
	bool DisableWallJumpReady;
	bool GroundDistanceZero;
	bool NoPitchRollHorzOnAir;
	bool DisableMoonsault;
	bool KeepRunning;
	bool KeepRunningEnableBackForce;
	bool KeepRunningOnSpiral;
	bool KeepRunningPause;
	bool SShapeRunning;
	bool MoveOnPath;
	bool IgnoreTerrain;
	bool UpdateYawByVelocity;
	bool NoPitchRoll;
	bool NoUpdateYaw;
	bool NoUpdateYawRef;
	bool UpdateYawOnAir;
	bool NoUpdateAdvancedDirection;
	bool CameraChagedPadCorrection;
	bool EnableCameraChagedPadCorrect;
	bool CameraToCullingBasePosEnabled;
	bool NoDamage;
	bool NoDead;
	bool Damaging;
	bool Paralyzing;
	bool ReactinJumpPathChange;
	bool ForcePitchRollGround;
	bool EnableAttenuateJump;
	bool NoGroundFall;
	bool OnStairs;
	bool OnBeltConveyor;
	bool MoveToPositionAndWait;
	bool StopPositionCount;
	bool OnNoWallWalkGround;
	bool ForceLandForCaught;
	bool Restarting;
	bool SlidingAndSquatPressOnly;
	bool SideViewNoPathMove;
	bool Pushing;
	bool NoChange2DPath;
	bool PrepreRestart;
	bool SetDirectRotation;
	bool IsTakingBreath;
	bool OnSpiralGound;
	bool OnMovableGround;
	bool OnFakeMovableGround;
	bool OnMoonsaltDisableGround;
	bool EnableExceptionalBoost;
	bool AcceptSlidingCollision;
	bool AcceptBuoyancyForce;
	bool AcceptEnvForce;
	bool DisableAdlibTrick;
	bool StandbyAdlibTrick;
	bool StandbyChangeToSpin;
	bool Pressing;
	bool FallEnabledInPressing;
	bool OnIntersectionalWay;
	bool ObserveBoostInExternalControl;
	bool ObserveSpinInExternalControl;
	bool ObserveInputInExternalControl;
	bool EscapeEnabledInExternalControl;
	bool NoDamageInExternalControl;
	bool Slipping;
	bool InvokeFlameBarrier;
	bool InvokeAquaBarrier;
	bool InvokeThunderBarrier;
	bool InvokeBarrier;
	bool InvokePreciousTime;
	bool InvokeHighSpeed;
	bool InvokeTimeBreak;
	bool InvokeSkateBoard;
	bool InvokeSuperSonic;
	bool InvokePtmRocket;
	bool InvokePtmSpike;
	bool InvokeUnbeaten;
	bool DoubleJumping;
	bool InvokeFixedMoveOn3D;
	bool KeepStateAfterChangeDimension;
	bool KeepPostureAfterChangeDimension;
	bool CalibrateFrontDir;
	bool EnableAnimationVelocityDirect;
	bool EnableGravityControl;
	bool EnableAirBoostOnGravityControl;
	bool EnableChaosEnergySetting;
	bool ChaosEnergySubstractDisabled;
	bool EnableAutoAim;
	bool ThroughGoalSignboard;
	bool AutoBoost;
	bool MaxSpeedLimited;
	bool Pause;
	bool SkateBoardSlowDown;
	bool ChangeCollisionLocked;
	bool ReadyGoOpened;
	bool SpikeSpin;
	bool ChangePostureInDeadAirEnabled;
	bool DisableGroundSmoke;
	bool Drifting;
	bool DriftingLowSpeed;
	bool DriftRight;
	bool Squat;
	bool LightSpeedDashReady;
	bool BoostKeep;
	bool DisableCrashWall;
	bool WallCorrection;
	bool Diving;
	bool DivingFloat;
	bool HipSliding;
	bool GrindFullBalance;
	bool GrindSideJump;
	bool Freeze;
	bool DisableBoost;
	bool NoSquatToSliding;
	bool IgnorePadInput;
	bool Rising;
	bool SpinChargeSliding;
	bool CombinationAttacking;
	bool SpinDash;
	bool LightAttack;
	bool AtomicSpin;
	bool Binding;
	bool ChgPlayerDisabled;
	bool EndReflection;
	bool CorrectOnPath;
	bool DebugDrawGroundHitPosition;
	bool DebugDrawVelocity;
	bool DebugDrawPath;
	bool DebugDrawSpikeInfo;
	bool DebugExceptionalMove;
};

#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult) if (a_eResult != tinyxml2::XML_SUCCESS) { printf("XMLParse Error: %i\n", a_eResult); return a_eResult; }
#endif

using SharedPtrTypeless = boost::shared_ptr<void>;
typedef void* __fastcall CSonicSpeedContextPlaySound(void*, void*, SharedPtrTypeless&, uint32_t cueId, uint32_t);

namespace Common
{

inline bool IsStringEndsWith(std::string const& value, std::string const& ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

inline CSonicStateFlags* GetSonicStateFlags()
{
	auto* const context = reinterpret_cast<int*>(*PLAYER_CONTEXT);
	return reinterpret_cast<CSonicStateFlags*>(*reinterpret_cast<int*>(context[0x14D] + 4));
}

inline bool CheckPlayerSuperForm()
{
    void* pSonicContext = *PLAYER_CONTEXT;
    if (pSonicContext)
    {
        uint32_t superSonicAddress = (uint32_t)(pSonicContext)+0x1A0;
        return (*(void**)superSonicAddress ? true : false);
    }

    return false;
}

inline bool CheckCurrentStage(char const* stageID)
{
    char const* currentStageID = (char*)0x01E774D4;
    return strcmp(currentStageID, stageID) == 0;
}

inline bool GetPlayerTransform(Eigen::Vector3f& position, Eigen::Quaternionf& rotation)
{
    if (!*PLAYER_CONTEXT) return false;

    const uint32_t result = *(uint32_t*)((uint32_t) * (void**)((uint32_t)*PLAYER_CONTEXT + 0x110) + 0xAC);
    if (!result) return false;

    float* pPos = (float*)(*(uint32_t*)(result + 0x10) + 0x70);
    position.x() = pPos[0];
    position.y() = pPos[1];
    position.z() = pPos[2];

    float* pRot = (float*)(*(uint32_t*)(result + 0x10) + 0x60);
    rotation.x() = pRot[0];
    rotation.y() = pRot[1];
    rotation.z() = pRot[2];
    rotation.w() = pRot[3];

    return true;
}

inline bool GetPlayerVelocity(Eigen::Vector3f& velocity)
{
    if (!*PLAYER_CONTEXT) return false;

    const uint32_t result = *(uint32_t*)((uint32_t) * (void**)((uint32_t)*PLAYER_CONTEXT + 0x110) + 0xAC);
    if (!result) return false;

    float* pVel = (float*)(result + 0x290);
    velocity.x() = pVel[0];
    velocity.y() = pVel[1];
    velocity.z() = pVel[2];

    return true;
}

inline bool SetPlayerVelocity(Eigen::Vector3f const& velocity)
{
    if (!*PLAYER_CONTEXT) return false;

    const uint32_t result = *(uint32_t*)((uint32_t) * (void**)((uint32_t)*PLAYER_CONTEXT + 0x110) + 0xAC);
    if (!result) return false;

    float* pVel = (float*)(result + 0x290);
    pVel[0] = velocity.x();
    pVel[1] = velocity.y();
    pVel[2] = velocity.z();

    return true;
}

inline bool GetWorldInputDirection(Eigen::Vector3f& direction)
{
	if (!*PLAYER_CONTEXT) return false;

	float* worldDir = (float*)((uint32_t)*PLAYER_CONTEXT + 0x130);
	direction.x() = worldDir[0];
	direction.y() = worldDir[1];
	direction.z() = worldDir[2];

	return true;
}

inline bool GetPlayerWorldDirection(Eigen::Vector3f& direction, bool normalize)
{
    if (!*PLAYER_CONTEXT) return false;

	if (!GetWorldInputDirection(direction)) return false;

    if (direction.isZero())
    {
        Eigen::Vector3f position;
        Eigen::Quaternionf rotation;
        if (GetPlayerTransform(position, rotation))
        {
            direction = rotation * Eigen::Vector3f(0, 0, 1);
            return true;
        }

        return false;
    }
    
    if (normalize)
    {
        direction.normalize();
    }

    return true;
}

inline void SonicContextChangeAnimation(const Hedgehog::Base::CSharedString& name)
{
	void* pSonicContext = *PLAYER_CONTEXT;
	if (!pSonicContext) return;

	FUNCTION_PTR(void, __thiscall, CSonicContextChangeAnimation, 0xE74CC0, CSonicContext* context, const Hedgehog::Base::CSharedString& name);
	CSonicContextChangeAnimation(pSonicContext, name);
}

inline void SonicContextPlaySound(SharedPtrTypeless& soundHandle, uint32_t cueID, uint32_t flag)
{
    // Note: This doesn't work at result screen, use PlaySoundStatic instead
    void* pSonicContext = *PLAYER_CONTEXT;
    if (!pSonicContext) return;

    // Original code by Skyth: https://github.com/blueskythlikesclouds
    CSonicSpeedContextPlaySound* playSoundFunc = *(CSonicSpeedContextPlaySound**)(*(uint32_t*)pSonicContext + 0x74);
    playSoundFunc(pSonicContext, nullptr, soundHandle, cueID, flag);
}

inline void SonicContextPlayVoice(SharedPtrTypeless& soundHandle, uint32_t cueID, uint32_t priority)
{
	// Note: This doesn't work at result screen, use PlaySoundStatic instead
	void* pSonicContext = *PLAYER_CONTEXT;
	if (!pSonicContext) return;

	// Original code by Skyth: https://github.com/blueskythlikesclouds
	CSonicSpeedContextPlaySound* playSoundFunc = *(CSonicSpeedContextPlaySound**)(*(uint32_t*)pSonicContext + 0xA0);
	playSoundFunc(pSonicContext, nullptr, soundHandle, cueID, priority);
}

inline void PlaySoundStatic(SharedPtrTypeless& soundHandle, uint32_t cueID)
{
    uint32_t* syncObject = *(uint32_t**)0x1E79044;
    if (syncObject)
    {
        FUNCTION_PTR(void*, __thiscall, sub_75FA60, 0x75FA60, void* This, SharedPtrTypeless&, uint32_t cueId);
        sub_75FA60((void*)syncObject[8], soundHandle, cueID);
    }
}

inline void applyObjectPhysicsPosition(void* pObject, Eigen::Vector3f const& pos)
{
	FUNCTION_PTR(void*, __thiscall, processObjectMsgSetPosition, 0xEA2130, void* This, void* message);
	alignas(16) MsgSetPosition msgSetPosition;
	msgSetPosition.m_position = pos;
	processObjectMsgSetPosition(pObject, &msgSetPosition);
}

inline void applyObjectPhysicsRotation(void* pObject, Eigen::Quaternionf const& rot)
{
	FUNCTION_PTR(void*, __thiscall, processObjectMsgSetRotation, 0xEA20D0, void* This, void* message);
	alignas(16) MsgSetRotation msgSetRotation;
	msgSetRotation.m_rotation = rot;
	processObjectMsgSetRotation(pObject, &msgSetRotation);
}

} // namespace Common
