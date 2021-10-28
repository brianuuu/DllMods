#pragma once

#define PI 3.141592
#define PI_F 3.141592f
#define DEG_TO_RAD PI_F / 180.0f
#define RAD_TO_DEG 180.0f / PI_F

typedef void CSonicContext;
CSonicContext** const PLAYER_CONTEXT = (CSonicContext**)0x1E5E2F0;
CSonicContext** const pModernSonicContext = (CSonicContext**)0x1E5E2F8;
CSonicContext** const pClassicSonicContext = (CSonicContext**)0x1E5E304;
CSonicContext** const pSuperSonicContext = (CSonicContext**)0x1E5E310;

uint32_t* const BACKBUFFER_WIDTH = (uint32_t*)0x1DFDDDC;
uint32_t* const BACKBUFFER_HEIGHT = (uint32_t*)0x1DFDDE0;

uint32_t const CStringConstructor = 0x6621A0;
uint32_t const CStringDestructor = 0x661550;

static void* const pCGlitterCreate = (void*)0xE73890;
static void* const pCGlitterEnd = (void*)0xE72650;
static void* const pCGlitterKill = (void*)0xE72570;

enum SonicCollision : uint32_t
{
	TypeNoAttack			= 0x1E61B5C,
	TypeRagdoll				= 0x1E61B60,
	TypeSonicSpinCharge		= 0x1E61B64,
	TypeSonicSpin			= 0x1E61B68,
	TypeSonicUnbeaten		= 0x1E61B6C,
	TypeSuperSonic			= 0x1E61B70,
	TypeSonicSliding		= 0x1E61B74,
	TypeSonicHoming			= 0x1E61B78,
	TypeSonicSelectJump		= 0x1E61B7C,
	TypeSonicDrift			= 0x1E61B80,
	TypeSonicBoost			= 0x1E61B84,
	TypeSonicStomping		= 0x1E61B88,
	TypeSonicTrickAttack	= 0x1E61B8C,
	TypeSonicSquatKick		= 0x1E61B90,
	TypeSonicClassicSpin	= 0x1E61B94,
	TypeExplosion			= 0x1E61B98,
	TypeBossAttack			= 0x1E61B9C,
	TypeGunTruckAttack		= 0x1E61BA0,
	TypeRagdollEnemyAttack	= 0x1E61BA4,
};

struct MatrixNodeSingleElementNode
{
	INSERT_PADDING(0x60);
	Eigen::Matrix4f local;
	Eigen::Matrix4f world;
	INSERT_PADDING(0x60);
};

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

struct MsgGetAnimationInfo
{
	INSERT_PADDING(0x14);
	char* m_name;
	float m_frame;

	bool IsAnimation(char const* anim)
	{
		return strstr(m_name, anim);
	}
};

enum ImpulseType : uint32_t
{
	None,
	DashPanel,
	UnknowCase_0x2,
	UnknowCase_0x3,
	UnknowCase_0x4,
	UnknowCase_0x5,
	JumpBoard,
	JumpBoardSpecial,
	DashRing,
	DashRingR,
	LookBack,
	HomingAttackAfter,
	BoardJumpBoard,
	UnknowCase_0xD,
	BoardJumpAdlibTrickA,
	BoardJumpAdlibTrickB,
	BoardJumpAdlibTrickC
};

struct MsgApplyImpulse
{
	INSERT_PADDING(0x10);
	Eigen::Vector3f m_position;
	INSERT_PADDING(0x4);
	Eigen::Vector3f m_impulse;
	INSERT_PADDING(0x4);
	float m_outOfControl;
	INSERT_PADDING(0x4);
	ImpulseType m_impulseType;
	float m_keepVelocityTime;
	bool m_notRelative; // if false, add impulse direction relative to Sonic
	bool m_snapPosition; // snap Sonic to m_position
	INSERT_PADDING(0x3);
	bool m_pathInterpolate; // linked to 80
	INSERT_PADDING(0xA);
	Eigen::Vector3f m_unknown80; // related to position interpolate?
	INSERT_PADDING(0x4);
	float m_alwaysMinusOne; // seems to be always -1.0f
	INSERT_PADDING(0xC);
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

inline float WrapFloat(float number, float bounds)
{
    if (number > bounds) number -= bounds;
    if (number < 0) number += bounds;
    return number;
}

inline void ClampFloat(float& number, float min, float max)
{
    if (number < min) number = min;
    if (number > max) number = max;
}

inline bool IsStringEndsWith(std::string const& value, std::string const& ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

inline uint32_t GetMultiLevelAddress(uint32_t initAddress, std::vector<uint32_t> offsets)
{
	uint32_t address = *(uint32_t*)initAddress;
	for (uint32_t i = 0; i < offsets.size(); i++)
	{
		uint32_t const& offset = offsets[i];
		address += offset;

		if (i < offsets.size() - 1)
		{
			address = *(uint32_t*)address;
		}
	}
	return address;
}

static void* fCGlitterCreate
(
	void* pContext,
	SharedPtrTypeless& handle,
	void* pMatrixTransformNode,
	Hedgehog::Base::CSharedString const& name,
	uint32_t flag
)
{
	__asm
	{
		push    flag
		push    name
		push    pMatrixTransformNode
		mov     eax, pContext
		mov     esi, handle
		call	[pCGlitterCreate]
	}
}

static void fCGlitterEnd
(
	void* pContext,
	SharedPtrTypeless& handle,
	bool instantStop
)
{
	__asm
	{
		mov     eax, [handle]
		mov     ebx, [eax + 4]
		push    ebx
		test	ebx, ebx
		jz		noIncrement
		mov		edx, 1
		add		ebx, 4
		lock xadd [ebx], edx

		noIncrement:
		mov     ebx, [eax]
		push    ebx
		mov     eax, pContext
		cmp     instantStop, 0
		jnz     jump
		call	[pCGlitterEnd]
		jmp     end

		jump:
		call	[pCGlitterKill]

		end:
	}
}

inline void ObjectCGlitterPlayerOneShot(void* pObject, Hedgehog::Base::CSharedString const& name)
{
	uint32_t* CGlitterPlayer = *(uint32_t**)((uint32_t)pObject + 0xF8);
	void* matrixNode = (void*)((uint32_t)pObject + 0xB8);
	if (*CGlitterPlayer == 0x16D0514)
	{
		FUNCTION_PTR(void, __thiscall, CGlitterPlayerOneShot, 0xE85F00, void* This, void* pMatrixTransformNode, Hedgehog::Base::CSharedString const& name, float a4, int a5);
		CGlitterPlayerOneShot(CGlitterPlayer, matrixNode, name, 1.0, 1);
	}
	else
	{
		MessageBox(NULL, L"Object does not contain CGlitterPlayer!", NULL, MB_ICONERROR);
	}
}

inline CSonicStateFlags* GetSonicStateFlags()
{
	auto* const context = reinterpret_cast<int*>(*PLAYER_CONTEXT);
	return reinterpret_cast<CSonicStateFlags*>(*reinterpret_cast<int*>(context[0x14D] + 4));
}

inline bool IsPlayerControlLocked()
{
	if (!*PLAYER_CONTEXT) return false;

	bool* unknownFlags = *(bool**)((uint32_t)*PLAYER_CONTEXT + 0x11C);
	return unknownFlags[0x98] || unknownFlags[0x99];
}

inline bool IsPlayerSuper()
{
	if (!*PLAYER_CONTEXT) return false;
    return GetSonicStateFlags()->InvokeSuperSonic;
}

inline bool IsPlayerOnBoard()
{
	if (!*PLAYER_CONTEXT) return false;
	return GetSonicStateFlags()->InvokeSkateBoard;
}

inline bool IsPlayerIn2D()
{
	// sub_E145A0 MsgIs2DMode
	if (!*PLAYER_CONTEXT) return false;
	return *(bool*)((uint32_t)*PLAYER_CONTEXT + 0x172);
}

inline bool IsPlayerInForwardPath()
{
	// sub_E145C0 MsgGetForwardPathInfo
	if (!*PLAYER_CONTEXT) return false;
	return *(uint32_t*)((uint32_t)*PLAYER_CONTEXT + 0x1278);
}

inline bool IsPlayerInDashPath()
{
	// sub_E14D30 MsgGetDashModeInfo
	// Note: Dash path also applies to forward path
	if (!*PLAYER_CONTEXT) return false;
	return *(uint32_t*)((uint32_t)*PLAYER_CONTEXT + 0x128C) && !IsPlayerInForwardPath();
}

inline bool IsPlayerGrinding()
{
	// sub_E144E0 MsgIsGrind
	if (!*PLAYER_CONTEXT) return false;
	return *(uint32_t*)((uint32_t)*PLAYER_CONTEXT + 0x11F0) != 0;
}

inline bool IsPlayerGrounded()
{
	// sub_E6ACA0 MsgGetGroundInfo
	if (!*PLAYER_CONTEXT) return false;
	return *(bool*)((uint32_t)*PLAYER_CONTEXT + 0x440);
}

inline bool IsAtLoadingScreen()
{
	uint32_t** hudCount = (uint32_t**)0x1E66B40;
	if (!*hudCount) return false;
	return (*hudCount)[2] > 0;
}

inline bool CheckPlayerNodeExist(const Hedgehog::Base::CSharedString& name)
{
	void* context = *PLAYER_CONTEXT;
	if (context)
	{
		void* player = *(void**)((char*)context + 0x110);
		if (player)
		{
			boost::shared_ptr<MatrixNodeSingleElementNode> node;
			FUNCTION_PTR(void, __thiscall, GetNode, 0x700B70, void* This, boost::shared_ptr<MatrixNodeSingleElementNode> & node, const Hedgehog::Base::CSharedString & name);
			GetNode(*(void**)((char*)player + 0x234), node, name);
			return (node ? true : false);
		}
	}

	return false;
}

inline bool CheckCurrentStage(char const* stageID)
{
    char const* currentStageID = (char*)0x01E774D4;
    return strcmp(currentStageID, stageID) == 0;
}

inline uint32_t GetCurrentStageID()
{
	uint32_t stageIDAddress = GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x80, 0x0 });
	return *(uint32_t*)stageIDAddress;
}

inline uint32_t GetPlayerSkill()
{
	if (!*PLAYER_CONTEXT) return 0;
	return *(uint32_t*)((uint32_t)*PLAYER_CONTEXT + 0x1A8);
}

inline uint32_t* GetPlayerRingCount()
{
	if (!*PLAYER_CONTEXT) return 0;
	return (uint32_t*)((uint32_t)*PLAYER_CONTEXT + 0x5B8);
}

inline float* GetPlayerBoost()
{
	if (!*PLAYER_CONTEXT) return 0;
	return (float*)((uint32_t)*PLAYER_CONTEXT + 0x5BC);
}

inline void SetPlayerAutoBoost(bool enabled)
{
	if (!*PLAYER_CONTEXT) return;
	GetSonicStateFlags()->AutoBoost = enabled;
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

inline bool GetPlayerTargetVelocity(Eigen::Vector3f& velocity)
{
	if (!*PLAYER_CONTEXT) return false;

	const uint32_t result = *(uint32_t*)((uint32_t) * (void**)((uint32_t)*PLAYER_CONTEXT + 0x110) + 0xAC);
	if (!result) return false;

	float* pVel = (float*)(result + 0x2A0);
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

inline void SonicContextGetAnimationInfo(MsgGetAnimationInfo& message)
{
	// Note: This doesn't work at result screen, use PlaySoundStatic instead
	void* pSonicContext = *PLAYER_CONTEXT;
	if (!pSonicContext) return;

	// Original code by Skyth: https://github.com/blueskythlikesclouds
	FUNCTION_PTR(void, __thiscall, CSonicSpeedProcMsgGetAnimationInfo, 0xE6A370, void* This, void* pMessage);
	void* player = *(void**)((uint32_t)*PLAYER_CONTEXT + 0x110);
	CSonicSpeedProcMsgGetAnimationInfo(player, &message);
}

inline void SonicContextRequestLocusEffect()
{
	// Note: This doesn't work at result screen, use PlaySoundStatic instead
	void* pSonicContext = *PLAYER_CONTEXT;
	if (!pSonicContext) return;

	// 1 seems to not stop? Force it to be 0
	WRITE_MEMORY(0xE178E5, uint32_t, 0);

	struct MsgRequestLocusEffect
	{
		INSERT_PADDING(0x10);
		uint32_t flag;
	};
	MsgRequestLocusEffect message {};
	message.flag = 0;

	FUNCTION_PTR(int, __thiscall, processMsgRequestLocusEffect, 0xE178D0, void* This, void* pMessage);
	void* player = *(void**)((uint32_t)*PLAYER_CONTEXT + 0x110);
	processMsgRequestLocusEffect(player, &message);
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

inline void SpawnBoostParticle(uint32_t** This, Eigen::Vector3f const& pos, int amount)
{
	if (amount <= 0) return;

	struct BoostParticleData
	{
		Eigen::Vector3f m_pos;
		float m_unknown0xC = 1.0f;
		int m_amount = 0;
		float m_unknown0x14 = 1.0f;
		float m_unknown0x18 = 0.0f;
	};

	FUNCTION_PTR(void, __cdecl, fpSpawnBoostParticle, 0x1125210, uint32_t* pCGameService, uint32_t* pCWorld, BoostParticleData* pData);
	alignas(16) BoostParticleData data = {};
	data.m_pos = pos;
	data.m_amount = amount;
	uint32_t CWorld = (This[41][1] + 0x7C);
	uint32_t* pCWorld = &CWorld;
	uint32_t* pCGameService = (uint32_t*)This[41][0];
	fpSpawnBoostParticle(pCGameService, pCWorld, &data);
}

inline void ApplyPlayerApplyImpulse(MsgApplyImpulse const& message)
{
	FUNCTION_PTR(void, __thiscall, processPlayerMsgAddImpulse, 0xE6CFA0, void* This, void* message);
	alignas(16) MsgApplyImpulse msgApplyImpulse = message;
	void* player = *(void**)((uint32_t)*PLAYER_CONTEXT + 0x110);
	processPlayerMsgAddImpulse(player, &msgApplyImpulse);
}

inline void ApplyObjectPhysicsPosition(void* pObject, Eigen::Vector3f const& pos)
{
	FUNCTION_PTR(void*, __thiscall, processObjectMsgSetPosition, 0xEA2130, void* This, void* message);
	alignas(16) MsgSetPosition msgSetPosition {};
	msgSetPosition.m_position = pos;
	processObjectMsgSetPosition(pObject, &msgSetPosition);
}

inline void ApplyObjectPhysicsRotation(void* pObject, Eigen::Quaternionf const& rot)
{
	FUNCTION_PTR(void*, __thiscall, processObjectMsgSetRotation, 0xEA20D0, void* This, void* message);
	alignas(16) MsgSetRotation msgSetRotation {};
	msgSetRotation.m_rotation = rot;
	processObjectMsgSetRotation(pObject, &msgSetRotation);
}

inline bool IsFileExist(std::string const& file)
{
	struct stat buffer;
	return stat(file.c_str(), &buffer) == 0;
}

inline void GetModIniList(std::vector<std::string>& modIniList)
{
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string exePath(buffer);
	std::string cpkRedirConfig = exePath.substr(0, exePath.find_last_of("\\")) + "\\cpkredir.ini";

	if (!Common::IsFileExist(cpkRedirConfig))
	{
		printf("%s not exist.\n", cpkRedirConfig.c_str());
		return;
	}

	INIReader reader(cpkRedirConfig);
	std::string modsDatabase = reader.Get("CPKREDIR", "ModsDbIni", "mods\\ModsDB.ini");

	if (!Common::IsFileExist(modsDatabase))
	{
		printf("%s not exist.\n", modsDatabase.c_str());
		return;
	}

	INIReader modsDatabaseReader(modsDatabase);
	int count = modsDatabaseReader.GetInteger("Main", "ActiveModCount", 0);
	for (int i = 0; i < count; i++)
	{
		std::string guid = modsDatabaseReader.Get("Main", "ActiveMod" + std::to_string(i), "");
		std::string config = modsDatabaseReader.Get("Mods", guid, "");
		if (!config.empty() && Common::IsFileExist(config))
		{
			modIniList.push_back(config);
		}
	}
}

inline bool IsModEnabled(std::string const& testModName, std::string* o_iniPath = nullptr)
{
	std::vector<std::string> modIniList;
	GetModIniList(modIniList);
	for (size_t i = 0; i < modIniList.size(); i++)
	{
		std::string const& config = modIniList[i];
		INIReader configReader(config);
		std::string name = configReader.Get("Desc", "Title", "");
		if (name == testModName)
		{
			if (o_iniPath)
			{
				*o_iniPath = config;
			}

			return true;
		}
	}

	return false;
}

inline bool TestModPriority(std::string const& currentModName, std::string const& testModName, bool higherPriority)
{
	printf("currentModName = %s, testModName = %s\n", currentModName.c_str(), testModName.c_str());

	int currentModIndex = -1;
	int testModIndex = -1;

	std::vector<std::string> modIniList;
	GetModIniList(modIniList);
	for (size_t i = 0; i < modIniList.size(); i++)
	{
		std::string const& config = modIniList[i];
		INIReader configReader(config);
		std::string name = configReader.Get("Desc", "Title", "");
		if (name == currentModName)
		{
			currentModIndex = i;
		}
		else if (name == testModName)
		{
			testModIndex = i;
		}
	}

	if (currentModIndex != -1 && testModIndex != -1)
	{
		bool success = true;
		if (higherPriority)
		{
			success = (testModIndex < currentModIndex);
		}
		else
		{
			success = (testModIndex > currentModIndex);
		}

		if (!success)
		{
			std::string errorMsg = testModName + " detected, please put it " + (higherPriority ? "higher" : "lower") + " priority than (" + (higherPriority ? "above" : "below") + ") this mod.";
			std::wstring stemp = std::wstring(errorMsg.begin(), errorMsg.end());
			std::wstring stemp2 = std::wstring(currentModName.begin(), currentModName.end());
			MessageBox(nullptr, stemp.c_str(), stemp2.c_str(), MB_ICONERROR);
			exit(-1);
		}

		return success;
	}

	// Mod not found
	return false;
}

inline std::string wideCharToMultiByte(LPCWSTR value)
{
	char multiByte[0x1000];
	WideCharToMultiByte(CP_UTF8, 0, value, -1, multiByte, _countof(multiByte), 0, 0);
	return std::string(multiByte);
}

inline std::wstring multiByteToWideChar(const char* value)
{
	WCHAR wideChar[0x1000];
	MultiByteToWideChar(CP_UTF8, 0, value, -1, wideChar, _countof(wideChar));
	return std::wstring(wideChar);
}

} // namespace Common
