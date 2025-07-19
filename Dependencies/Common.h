#pragma once

#define PI 3.141592
#define PI_F 3.141592f
#define DEG_TO_RAD PI_F / 180.0f
#define RAD_TO_DEG 180.0f / PI_F

#define DEBUG_VECTOR2(vec2) printf("%.2f, %.2f\n",vec2.x(),vec2.y());
#define DEBUG_VECTOR3(vec3) printf("%.2f, %.2f, %.2f\n",vec3.x(),vec3.y(),vec3.z());
#define DEBUG_VECTOR4(vec4) printf("%.2f, %.2f, %.2f, %.2f\n",vec4.x(),vec4.y(),vec4.z(),vec4.w());
#define RAND_FLOAT(LO, HI) LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));

typedef void CSonicContext;
CSonicContext** const PLAYER_CONTEXT = (CSonicContext**)0x1E5E2F0;
CSonicContext** const pModernSonicContext = (CSonicContext**)0x1E5E2F8;
CSonicContext** const pClassicSonicContext = (CSonicContext**)0x1E5E304;
CSonicContext** const pSuperSonicContext = (CSonicContext**)0x1E5E310;

uint32_t* const BACKBUFFER_WIDTH = (uint32_t*)0x1DFDDDC;
uint32_t* const BACKBUFFER_HEIGHT = (uint32_t*)0x1DFDDE0;

uint32_t const CStringConstructor = 0x6621A0;
uint32_t const CStringDestructor = 0x661550;

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

struct MsgSetPinballHud
{
	INSERT_PADDING(0x10);
	uint32_t m_flag;
	uint32_t m_score;
	INSERT_PADDING(0x8);
};

struct MsgGetAnimationInfo
{
	INSERT_PADDING(0x14);
	char* m_name;
	float m_frame;

	bool IsAnimation(char const* anim)
	{
		if (!m_name)
		{
			return false;
		}
		return strstr(m_name, anim);
	}
};

struct MsgGetItemType
{
	INSERT_PADDING(0x10);
	uint32_t m_type;
	bool m_unknown0x14; // related to get on board velocity
	INSERT_PADDING(0xB);
	Eigen::Vector3f m_unknown0x20;
	INSERT_PADDING(0x4);
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
	Eigen::Vector3f m_position; // 0x10
	INSERT_PADDING(0x4);
	Eigen::Vector3f m_impulse; // 0x20
	INSERT_PADDING(0x4);
	float m_outOfControl; // 0x30
	INSERT_PADDING(0x4);
	ImpulseType m_impulseType; // 0x38
	float m_keepVelocityTime; // 0x3C
	bool m_notRelative; // 0x40 if false, add impulse direction relative to Sonic
	bool m_snapPosition; // 0x41 snap Sonic to m_position
	INSERT_PADDING(0x3);
	bool m_pathInterpolate; // 0x45 linked to 80
	INSERT_PADDING(0xA);
	Eigen::Vector3f m_unknown80; // 0x50 related to position interpolate?
	INSERT_PADDING(0x4);
	float m_alwaysMinusOne; // 0x60 seems to be always -1.0f
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

enum EPlayerParameter
{
	AccelerationForce = 0x1C,
	AccelerationForceBaseUp = 0x20,
	AccelerationForceBySkill = 0x1D,
	AccelerationForceOnBoard = 0x1E,
	AccelerationForceOnBoardSlow = 0x1F,
	AccelerationForceOnSpike = 0x1DE,
	AccelerationForceSps = 0x22,
	ActionEnableAfterHomingAttack = 0x11D,
	AdTrickAddChaosEnergyBonus = 0x180,
	AdTrickAddChaosEnergyLv1 = 0x17D,
	AdTrickAddChaosEnergyLv2 = 0x17E,
	AdTrickAddChaosEnergyLv3 = 0x17F,
	AdTrickAddChaosEnergyMin = 0x17C,
	AdTrickAnimSpeedOnSkill = 0x186,
	AdTrickGameSpeed = 0x183,
	AdTrickGameSpeedAwakeTime = 0x185,
	AdTrickGameSpeedAwakeTimeOfStart = 0x182,
	AdTrickGameSpeedOfStart = 0x181,
	AdTrickGameSpeedSV = 0x184,
	AdTrickVelocityXZOfHorz = 0x267,
	AdTrickVelocityXZOfRolling = 0x265,
	AdTrickVelocityXZOfVert = 0x269,
	AdTrickVelocityYOfHorz = 0x266,
	AdTrickVelocityYOfRolling = 0x264,
	AdTrickVelocityYOfVert = 0x268,
	AdditionalLimitTime = 0xAA,
	AirBoostCalibratedAirDragScale = 0x170,
	AirBoostDisableTimeAfterSpinAttack = 0x16C,
	AirBoostEnableChaosEnergy = 0x167,
	AirBoostFirstDecrease = 0x16D,
	AirBoostGravityRate = 0x168,
	AirBoostLevitationForce = 0x169,
	AirBoostLevitationTime = 0x16A,
	AirBoostStartVelocityRate = 0x16E,
	AirBoostTime = 0x16B,
	AisacSpeed1st = 0x1BF,
	AisacSpeed2nd = 0x1C0,
	AisacSpeed3rd = 0x1C1,
	AtomicSpinChargeMaxTime = 0x1ED,
	AtomicSpinChargeMinTime = 0x1EC,
	AtomicSpinChargeOffsX = 0x1EE,
	AtomicSpinChargeOffsY = 0x1EF,
	AtomicSpinChargeOffsZ = 0x1F0,
	AtomicSpinHomingAngle = 0x1F1,
	AtomicSpinMaxTime = 0x1EB,
	AtomicSpinMinTime = 0x1EA,
	AtomicSpinMoveTime = 0x1F5,
	AtomicSpinStartHomingTime = 0x1F4,
	AtomicSpinVelocity = 0x1E9,
	AtomicSpinVelocityKeep = 0x1F2,
	AtomicSpinVelocityMove = 0x1F3,
	AtomicSpinVelocityReturn = 0x1F6,
	AttackAfterImpluseVelocityCoeff = 0x148,
	BallJumpAirDragRateScale = 0x4D,
	BlinkNoDamageTime = 0x6A,
	BlockingEnemyMaxNum = 0xD4,
	BoardMaxPitchVelocity = 0x1B2,
	BoardMinPitchVelocity = 0x1B1,
	BoostAccelerationForce = 0x163,
	BoostAccelerationForceWhenSlow = 0x164,
	BoostBeginCameraShakeCount = 0x153,
	BoostBeginCameraShakeSize = 0x151,
	BoostBeginCameraShakeTime = 0x152,
	BoostCancelBrakeTime = 0x165,
	BoostEnableChaosEnergy = 0x14E,
	BoostEndKeepTime = 0x15E,
	BoostFirstDecrease = 0x158,
	BoostInAirKeepTime = 0x15D,
	BoostMaxVelocityScaleLv0 = 0x161,
	BoostMaxVelocityScaleLvMax = 0x162,
	BoostMinVelocity = 0x15C,
	BoostPullRadius = 0x154,
	BoostStartVelocityRate = 0x15A,
	BoostStrength = 0x15F,
	BoostTimeDecrease = 0x159,
	BoostTimeDecreaseOnBoard = 0x150,
	BoostTimeDecreaseOnWater = 0x14F,
	BorderTimeOfBrakeCancelForStartDrift = 0x1E0,
	BoudnJumpVelocity = 0x195,
	BoundJumpBeforeVelocity = 0x194,
	BoundJumpGravityScale = 0x196,
	BrakeCancelAngle = 0x46,
	BrakeDownForceEnableSpeed = 0x43,
	BrakeEnableAngle = 0x45,
	BrakeEnableSpeed = 0x41,
	BrakeEndSpeed = 0x40,
	BrakeForce = 0x42,
	BrakeForceScaleOfLarge = 0x48,
	BrakeForceScaleOfSkill = 0x49,
	BrakeLargeSpeed = 0x47,
	BuoyantForceMaxGravityRate = 0xA6,
	BuoyantForceMaxVelocity = 0xA3,
	BuoyantForceZeroVelocity = 0xA2,
	CameraOffsetReverseVelocityLength = 0x25F,
	CameraTargetOffsetSensitive = 0xB1,
	CameraTargetUpOffset = 0xAE,
	CameraTargetUpOffsetInAir = 0xAF,
	CameraTargetUpOffsetSensitive = 0xB0,
	ChaosEnergyAutoRecoverRate = 0x14D,
	ChaosEnergyRecoverRateByRing = 0x14A,
	ChaosEnergyRecoverRateByRingBonus = 0x14B,
	ChaosEnergyRecoverRateByRingPenalty = 0x14C,
	CharProxySpeedThreshold = 0x8D,
	CharProxyStaticFriction = 0x8C,
	CollisionPushingRadius = 0xC1,
	CollisionRadius = 0xC0,
	CollisionSquatHeight = 0xC3,
	CollisionStandHeight = 0xC2,
	CrashWallBorder = 0x50,
	DamageAirJumpPermitDistance = 0x6C,
	DamageCancelHorzVelocity = 0x70,
	DamageCancelVertVelocity = 0x6F,
	DamageDecForceOfWallWalk = 0x6B,
	DamageDownTime = 0x67,
	DamageElectricTime = 0x72,
	DamageFireTime = 0x71,
	DamageFreezeAirKeepTime = 0x73,
	DamageFreezeReflectEfficient = 0x75,
	DamageFreezeTime = 0x74,
	DamageHorzDecRateOnBoard = 0x6E,
	DamageMoveScale = 0x68,
	DamageStandUpCancelTime = 0x69,
	DamageVertVelOnBoard = 0x6D,
	DeadBgmFadeTime = 0x78,
	DeadBgmWaitTime = 0x77,
	DeadToRestartTime = 0x76,
	DebugNoDamage = 0x1C2,
	DebugNoDead = 0x1C3,
	DebugNoDeadDrowning = 0x1C6,
	DebugNoDrowningImmediately = 0x1C5,
	DebugNoFallDead = 0x1C4,
	DebugResultRankLv = 0x4,
	DebugSEEnabled = 0x0,
	DebugSkillCancelEnabled = 0x1C7,
	DebugSkillSpikeEndless = 0x1C8,
	DebugSkillSuperSonic = 0x1C9,
	DebugVoiceEnabled = 0x1,
	DecelerationForceBoardTurn = 0x26,
	DecelerationForceHigh = 0x24,
	DecelerationForceLow = 0x25,
	DistanceFrom2DPath = 0x1DE,
	DivingDamageTime = 0x228,
	DivingDamageVelocityDecreaseRate = 0x227,
	DivingDiveBeginGravityScale = 0x21F,
	DivingDiveBeginHorzVelocityRate = 0x21E,
	DivingDiveEndSpeed = 0x223,
	DivingDiveMaxVelocityDown = 0x220,
	DivingDiveVelocityFront = 0x222,
	DivingDiveVelocitySide = 0x221,
	DivingFloatMaxVelocityDown = 0x224,
	DivingFloatVelocityFront = 0x226,
	DivingFloatVelocitySide = 0x225,
	DivingRotationMax = 0x21D,
	DownForceEfficient = 0x59,
	DriftAngleReturnForce = 0x20F,
	DriftBoostYawOffset = 0x217,
	DriftFinishAngle = 0x215,
	DriftFinishSideVelocity = 0x216,
	DriftFrontAccelForceRate = 0x214,
	DriftFrontAngleForce = 0x211,
	DriftMaxAngleVelocity = 0x212,
	DriftSideAngleForce = 0x210,
	DriftSideDecForce = 0x213,
	DynamicFrictionalForce = 0x95,
	DynamicFrictionalForceCoef = 0x96,
	DynamicResistanceForceWhenRun = 0x8E,
	DynamicResistanceForceWhenStand = 0x8F,
	EnableHurdleJumpMinVelocity = 0x1CA,
	EnemyBlockedJumpPower1 = 0xD7,
	EnemyBlockedJumpPowerDec = 0xD8,
	EnemyBlockedMaxSquatVelocity = 0x218,
	EnemyBlockedMaxVelocity1 = 0xD5,
	EnemyBlockedMaxVelocityDec = 0xD6,
	EnemyBlockedRotationForce1 = 0xD9,
	EnemyBlockedRotationForceDec = 0xDA,
	EnemyBlockedRotationSpeedMax1 = 0xDB,
	EnemyBlockedRotationSpeedMaxDec = 0xDC,
	EnemyBlockedSpringImpulseMax1 = 0xDF,
	EnemyBlockedSpringImpulseMaxDec = 0xE0,
	EnemyBlockedSpringImpulseScale1 = 0xDD,
	EnemyBlockedSpringImpulseScaleDec = 0xDE,
	EnemyComboTime = 0x235,
	FinishSlidingVelocity = 0x1CE,
	ForceGroundAttribute = 0x2,
	ForceResultRank = 0x3,
	FrontNoGroundStopMaxInput = 0xFF,
	FrontNoGroundStopMaxSpeed = 0xFE,
	FrontNoGroundStopSearchLength = 0xFD,
	GhostAutoRec = 0x12,
	Gravity = 0x14,
	GrindAccelerationForce = 0x204,
	GrindBeginMinSpeed = 0x199,
	GrindCurveDecreaseForce = 0x209,
	GrindDamageTime = 0x206,
	GrindDamageVelocityDecreaseRate = 0x207,
	GrindJumpPowerMin = 0x198,
	GrindJumpShortReleaseTime = 0x197,
	GrindJumpSideMaxDistance = 0x19A,
	GrindJumpSideSearchFrontOffsetTime = 0x19B,
	GrindMinSpeed = 0x205,
	GrindSEMaxPitch = 0x19E,
	GrindSEMaxPitchVelocity = 0x19F,
	GrindSEMinPitch = 0x19C,
	GrindSEMinPitchVelocity = 0x19D,
	GrindSideJumpForce = 0x208,
	GrindStumbleLimit = 0x20A,
	GrindStumbleLimitTime = 0x20B,
	GroundSearchDiameter = 0x144,
	HangOnJumpHorzVel = 0x1A2,
	HangOnJumpPower = 0x1A3,
	HangOnJumpVelCoeffDown = 0x1A1,
	HangOnJumpVelCoeffUp = 0x1A0,
	HighSpeedEffectMaxVelocity = 0x261,
	HighSpeedEffectMaxVelocityDuringFall = 0x263,
	HighSpeedEffectMinVelocity = 0x260,
	HighSpeedEffectMinVelocityDuringFall = 0x262,
	HipSlidingMaxVelocity = 0x17A,
	HipSlidingVelocity = 0x179,
	HomingAttackAfterUpVelocity = 0x123,
	HomingAttackEffectWidth = 0x13B,
	HomingAttackEnableAfterFall = 0x124,
	HomingAttackEnableVertUp = 0x11A,
	HomingAttackSplinePosCount = 0x13A,
	HomingBackEnableSpeed = 0x125,
	HomingCalibratedAirDragScale = 0x120,
	HomingInertiaForceRate = 0x121,
	HomingLockonCollisionAspect = 0x127,
	HomingLockonCollisionAspectN = 0x12E,
	HomingLockonCollisionFar = 0x12A,
	HomingLockonCollisionFarN = 0x131,
	HomingLockonCollisionFovy = 0x128,
	HomingLockonCollisionFovyN = 0x12F,
	HomingLockonCollisionNear = 0x129,
	HomingLockonCollisionNearN = 0x130,
	HomingLockonCollisionOffsetX = 0x12C,
	HomingLockonCollisionOffsetXN = 0x133,
	HomingLockonCollisionOffsetY = 0x12D,
	HomingLockonCollisionOffsetYN = 0x134,
	HomingLockonCollisionRotationX = 0x12B,
	HomingLockonCollisionRotationXN = 0x132,
	HomingLockonKeepBack = 0x139,
	HomingLockonKeepDownOffset = 0x138,
	HomingLockonKeepDownSlope = 0x137,
	HomingLockonKeepUpOffset = 0x136,
	HomingLockonKeepUpSlope = 0x135,
	HomingNoLockonKeepTime = 0x122,
	HomingRotateForce = 0x11C,
	HomingSpeed = 0x11E,
	HomingSpeedOfDummy = 0x11F,
	HorzStandEnableMaxSlope = 0xB3,
	HurdleFirstAnimSpeed = 0x232,
	HurdleJumpRotateZRate = 0x22B,
	HurdleReleaseAnimSpeed = 0x233,
	IdleActionTime = 0x145,
	IdleSleepActionCount = 0x146,
	InAirForceAccelRate = 0x27,
	InAirForceAccelRateInLowSpd = 0x2C,
	InAirForceAccelRateOnBoard = 0x31,
	InAirForceBrakeRate = 0x29,
	InAirForceBrakeRateInLowSpd = 0x2E,
	InAirForceBrakeRateOnBoard = 0x33,
	InAirForceDecRate = 0x28,
	InAirForceDecRateInLowSpd = 0x2D,
	InAirForceDecRateOnBoard = 0x32,
	InAirLowSpdThreshold = 0x30,
	InAirMaxAccelVelocity = 0x2A,
	InAirMaxAccelVelocityInLowSpd = 0x2F,
	InAirMaxAccelVelocityOfNormalJump = 0x2B,
	IncreaseChaosEnergyDriftPerSecond = 0x1DD,
	IncreaseChaosEnergyDriftStart = 0x1DC,
	InputSubStickInTrickCommand = 0x11,
	InputThreshold = 0xE,
	InputTransformHalf = 0xF,
	InputTransformPower = 0x10,
	InternalScaleInReflection = 0x1DD,
	IsEnableBoost3DEffect = 0x166,
	IsEnableCrashWall = 0x51,
	IsEnableEffect = 0x5,
	IsHomingLockonCollisionEdit = 0x126,
	IsLightSpeedDashCollisionEdit = 0x1F4,
	IsLightSpeedDashHoldMode = 0x202,
	IsLightSpeedDashStartCollisionEdit = 0x1EC,
	ItemCollisionHeightInDiving = 0x229,
	ItemCollisionRadius = 0xBA,
	ItemCollisionRadiusInBoost = 0xBD,
	ItemCollisionRadiusInDiving = 0x22A,
	ItemCollisionSquatHeight = 0xBC,
	ItemCollisionSquatHeightInBoost = 0xBF,
	ItemCollisionStandHeight = 0xBB,
	ItemCollisionStandHeightInBoost = 0xBE,
	JumpGroundVelocityXZRate = 0x3E,
	JumpGroundVelocityYRate = 0x3D,
	JumpPower = 0x36,
	JumpPowerAllRounder = 0x3C,
	JumpPowerBySkill = 0x37,
	JumpPowerDamageAir = 0x38,
	JumpPowerMin = 0x3A,
	JumpPowerMinOnBoard = 0x3B,
	JumpPowerOnBoard = 0x39,
	JumpShortGroundResetDistance = 0x4B,
	JumpShortReleaseTime = 0x4A,
	KeepRunningDamageInternalScale = 0x106,
	KeepRunningDamageTime = 0x105,
	KeepRunningDamageVelocityDecreaseRate = 0x104,
	KeepRunningMaxVelocityOnBoard = 0x109,
	KeepRunningMinVelocity = 0x102,
	KeepRunningMinVelocityDebug = 0x107,
	KeepRunningMinVelocityOnBoard = 0x108,
	KickbackPower = 0x3F,
	LandEnableMaxSlope = 0xB2,
	LandStrongMaxHorzSpeed = 0xB5,
	LandStrongMinVertSpeed = 0xB4,
	LightAttackChargeTime = 0x1DF,
	LightAttackColRadius0 = 0x1E2,
	LightAttackColRadius1 = 0x1E3,
	LightAttackDurationTime = 0x1E6,
	LightAttackEffScale0 = 0x1E4,
	LightAttackEffScale1 = 0x1E5,
	LightAttackVelocity0 = 0x1E0,
	LightAttackVelocity1 = 0x1E1,
	LightAttackVelocityKeep = 0x1E8,
	LightAttackVelocityReturn = 0x1E7,
	LightSpeedDashCollisionAspect = 0x1F5,
	LightSpeedDashCollisionFar = 0x1F8,
	LightSpeedDashCollisionFovy = 0x1F6,
	LightSpeedDashCollisionNear = 0x1F7,
	LightSpeedDashCollisionOffsetX = 0x1FA,
	LightSpeedDashCollisionOffsetY = 0x1FB,
	LightSpeedDashCollisionRotationX = 0x1F9,
	LightSpeedDashEnableAngle = 0x201,
	LightSpeedDashHoldTime = 0x200,
	LightSpeedDashMaxVelocity = 0x1FD,
	LightSpeedDashMaxVelocity3D = 0x1FF,
	LightSpeedDashMinVelocity = 0x1FC,
	LightSpeedDashMinVelocity3D = 0x1FE,
	LightSpeedDashStartCollisionAspect = 0x1ED,
	LightSpeedDashStartCollisionFar = 0x1F0,
	LightSpeedDashStartCollisionFovy = 0x1EE,
	LightSpeedDashStartCollisionNear = 0x1EF,
	LightSpeedDashStartCollisionOffsetX = 0x1F2,
	LightSpeedDashStartCollisionOffsetY = 0x1F3,
	LightSpeedDashStartCollisionRotationX = 0x1F1,
	LimitTimeOfSpin = 0x1B,
	LimitTimeToWaterWalk = 0xA9,
	LostGridSideSpeedRate = 0x99,
	MaxDownVelocity = 0xC4,
	MaxHorzVelocity = 0xC6,
	MaxUpSpeedWhenBound = 0xFB,
	MaxUpSpeedWhenJump = 0xFA,
	MaxUpVelocity = 0xC5,
	MaxVelocityBasis = 0x16,
	MaxVelocityBasisBaseUp = 0x21,
	MaxVelocityBasisSps = 0x23,
	MaxVelocityCoefOnSpike = 0x1DF,
	MaxVelocityFinalMax = 0x17,
	MaxVelocitySonicToSpin = 0x1A,
	MaxVelocitySpinToSonic = 0x19,
	ModelScaleInPlayableMenu = 0x149,
	MotionDashToJet = 0x231,
	MotionJogToRun = 0x22F,
	MotionRunToDash = 0x230,
	MotionStandToWalk = 0x22C,
	MotionWalkFastToJog = 0x22E,
	MotionWalkToWalkFast = 0x22D,
	MoveEnableGroundAngleInner = 0xB7,
	MoveEnableGroundAngleOuter = 0xB8,
	MoveEnableSearchDistance = 0xB9,
	MoveHoldingMinVelocity = 0xB6,
	NoInputTimeInReflection = 0x1DC,
	NoPadStopWalkBeginMaxVelocity = 0xD3,
	NoPadStopWalkDecelerationForce = 0xD1,
	NoPadStopWalkPadLengthLimit = 0xD0,
	NormalStrength = 0x8B,
	PadCorrectWhenCameraChangeAlwaysAccelTime = 0x140,
	PadCorrectWhenCameraChangeKeepTime = 0x141,
	PadInputForceCoeffInAir = 0x142,
	ParaloopInterval = 0x156,
	ParaloopMaxLength = 0x155,
	ParaloopMinSpeed = 0x157,
	PathBinrmCorrectionAngle = 0x13C,
	PathBinrmCorrectionOffset = 0x13D,
	PitchRollRotateMaxVelocity = 0xF9,
	PitchRollRotateMinVelocity = 0xF8,
	PitchRollRotateSpeedAccele = 0xF4,
	PitchRollRotateSpeedInAirRate = 0xF5,
	PitchRollRotateSpeedMax = 0xF2,
	PitchRollRotateSpeedMaxDif = 0xF0,
	PitchRollRotateSpeedMaxVelocity = 0xF7,
	PitchRollRotateSpeedMin = 0xF3,
	PitchRollRotateSpeedMinDif = 0xF1,
	PitchRollRotateSpeedMinVelocity = 0xF6,
	PrecedeJumpEnableTime = 0xFC,
	ReactionFailPushOutForce = 0x5D,
	ReactionJumpGravity = 0x62,
	ReactionJumpMaxEnterVelocity = 0x61,
	ReactionJumpMaxVelocity = 0x5F,
	ReactionJumpMinEnterVelocity = 0x60,
	ReactionJumpMinVelocity = 0x5E,
	RecoverGridSideSpeedRate = 0x98,
	RecoverGridSpeed = 0x97,
	ResistanceStandRunBorder = 0x92,
	ResistanceStandRunIntgerpolate = 0x93,
	ResultPosOffsetX = 0x1F8,
	ResultPosOffsetY = 0x1F9,
	ResultPosOffsetZ = 0x1FA,
	RingDropAdditionalNum = 0x188,
	RingDropMin = 0x187,
	RingSubtractTime = 0x1F7,
	RocketAccel = 0x1A6,
	RocketChargeEndTime = 0x1A4,
	RocketForceScale = 0x1AA,
	RocketLaunchTime = 0x1A5,
	RocketMaxVelocity = 0x1A9,
	RocketMinVelocity = 0x1A8,
	RocketVelocity = 0x1A7,
	RotationForce0 = 0xE1,
	RotationForce1 = 0xE3,
	RotationForce2 = 0xE5,
	RotationForceInAir = 0xEC,
	RotationForceInBoardAir = 0xEE,
	RotationForceInBoost = 0xEA,
	RotationForceMaxAngle = 0xCF,
	RotationForceMaxAngleLw = 0xCB,
	RotationForceMinAngle = 0xCE,
	RotationForceMinAngleLw = 0xCA,
	RotationForceRateInAir = 0xED,
	RotationForceRateInBoardAir = 0xEF,
	RotationForceRateMax = 0xCD,
	RotationForceRateMaxLw = 0xC9,
	RotationForceRateMin = 0xCC,
	RotationForceRateMinLw = 0xC8,
	RotationForceThresholdSpd = 0xC7,
	RotationForceVelocity0 = 0xE2,
	RotationForceVelocity1 = 0xE4,
	RotationForceVelocity2 = 0xE6,
	RotationSpeedMax = 0xE7,
	RotationSpeedMaxShoesSliding = 0xE9,
	RotationSpeedMaxSpike = 0xE8,
	RunQuickStepMoveDistance = 0x1D7,
	SkillAddChaosEnergyOnBoardJump = 0x193,
	SkillCountAquaBarrier = 0x18F,
	SkillCountBarrier = 0x18D,
	SkillCountFlameBarrier = 0x18E,
	SkillCountHighSpeed = 0x191,
	SkillCountPreciousTime = 0x192,
	SkillCountSkateBoard = 0x18C,
	SkillCountThunderBarrier = 0x190,
	SkillHighSpeedTime = 0x18B,
	SkillPreciousTime = 0x18A,
	SkillUnbeatenTime = 0x189,
	SlidingBrakeForce = 0x1D0,
	SlidingMinTime = 0x1D1,
	SlipToWalkVelocity = 0x17B,
	SlopeVelocityKeepRate = 0x117,
	SparkleEffectCount = 0x243,
	SparkleEffectIntervalTime = 0x24D,
	SparkleEffectLife = 0x244,
	SparkleEffectLifeRand = 0x245,
	SparkleEffectMeshPointCount = 0x24A,
	SparkleEffectRandDir = 0x24B,
	SparkleEffectRandSpeed = 0x24C,
	SparkleEffectSpeedMax = 0x249,
	SparkleEffectSpeedMin = 0x248,
	SparkleEffectWidth = 0x246,
	SparkleEffectWidthRand = 0x247,
	SpikeBoostChargeTime1 = 0x1AE,
	SpikeBoostChargeTime2 = 0x1AF,
	SpikeBoostVelocity0 = 0x1AB,
	SpikeBoostVelocity1 = 0x1AC,
	SpikeBoostVelocity2 = 0x1AD,
	SpikeInvokeTime = 0x1B0,
	SpinAttackAfterUpVelocity = 0x176,
	SpinAttackAfterUpVelocityStrong = 0x177,
	SpinAttackThresholdVelocity = 0x178,
	SpinChargeTimeLv0 = 0x24E,
	SpinChargeTimeLv1 = 0x24F,
	SpinChargeTimeLv2 = 0x250,
	SpinDecMaxVelocity = 0x25E,
	SpinDecRate = 0x25A,
	SpinDecRateBySkill = 0x25B,
	SpinDecStartVelocity = 0x25D,
	SpinDecVelocity = 0x25C,
	SpinDynamicFrictionalForce = 0x258,
	SpinDynamicFrictionalForceBySkill = 0x259,
	SpinMinTime = 0x256,
	SpinTimeOfKeeprunning = 0x257,
	SpinVelocityWithChargeLv0 = 0x251,
	SpinVelocityWithChargeLv1 = 0x252,
	SpinVelocityWithChargeLv2 = 0x253,
	SpinVelocityWithSlidingChargeLv1 = 0x254,
	SpinVelocityWithSlidingChargeLv2 = 0x255,
	SpringImpluseVelocityCoeff = 0x147,
	SpsDecRingTime = 0x1B3,
	SpsDecRingTimeBoost = 0x1B4,
	SpsFloatingHeight = 0x1B9,
	SpsFloatingMoveSide = 0x1B7,
	SpsFloatingMoveSideSpd = 0x1B8,
	SpsFloatingMoveUp = 0x1B5,
	SpsFloatingMoveUpSpd = 0x1B6,
	SquatAndSlidingStairsMax = 0x9B,
	SquatKickInputTime = 0x234,
	SquatToSlidingSlope = 0x1CF,
	StaggerNoInputSecond = 0x175,
	StaggerSearchDistance = 0x174,
	StaggerStartVelocityLength = 0x173,
	StairsMax = 0x9A,
	StairsSensitive = 0x9C,
	StandToWalkVelocity = 0x143,
	StartDriftFromSlidingBorder = 0x1E3,
	StartDriftInputAngleBorder = 0x1E2,
	StartTimeToCountDown = 0xAB,
	StaticFrictionalForce = 0x94,
	StaticResistanceForceWhenRun = 0x90,
	StaticResistanceForceWhenStand = 0x91,
	StompingDisableTime = 0x1EA,
	StompingDownFirstVelocity = 0x1E7,
	StompingHorzDecForce = 0x1EB,
	StompingStrength = 0x20E,
	StumbleDecRate = 0x55,
	StumbleDecTime = 0x56,
	StumbleHeightMax = 0x53,
	StumbleInternalScale = 0x57,
	StumbleTime = 0x52,
	StumbleVelocityMin = 0x54,
	TramJumpPowerAdvancing = 0x1BA,
	TramJumpPowerBreak = 0x1BB,
	VelocityAdjustSpeed = 0x1DB,
	VelocityByABSRay = 0x1D8,
	VelocityInputVert = 0x1D6,
	VelocityOnDamage = 0x1D9,
	VelocityReflection = 0x1D7,
	VelocityStrengthScale = 0x116,
	VelocityUpInFloatingBoost0 = 0x34,
	VelocityUpInFloatingBoost1 = 0x35,
	VelocityUpOnDamage = 0x1DA,
	VertVelocityBallToFall = 0x58,
	VirtualStairsEnableMaxVelocity = 0x13F,
	VirtualStairsMax = 0x13E,
	WalkBrakeForce = 0x44,
	WalkSideMotionMaxInput = 0x111,
	WalkSideMotionMinInput = 0x110,
	WalkSideMotionMovePathScale = 0x112,
	WalkSideMotionSensitive = 0x113,
	WalkWallMotionSlopeAngleMax = 0x115,
	WalkWallMotionSlopeAngleMin = 0x114,
	WallJumpAMinTime = 0x86,
	WallJumpAtoBDistance = 0x88,
	WallJumpAtoBTime = 0x87,
	WallJumpBtoCSpeed = 0x89,
	WallJumpFrontForce = 0x85,
	WallJumpOutOfControlTime = 0x8A,
	WallJumpReadyAccel = 0x83,
	WallJumpReadyAccelStartTime = 0x82,
	WallJumpReadyBorder = 0x7A,
	WallJumpReadyDisableAirTime = 0x7B,
	WallJumpReadyGravity = 0x7E,
	WallJumpReadyMaxDownSpeed = 0x7F,
	WallJumpReadyMaxTime = 0x81,
	WallJumpReadyMinBottomDistance = 0x7C,
	WallJumpReadyMinTime = 0x80,
	WallJumpReadyStopTime = 0x7D,
	WallJumpUpForce = 0x84,
	WallKickForce = 0x10F,
	WallPushForce = 0x10E,
	WallPushLimitCharacterAngle = 0x10B,
	WallPushLimitInputIntensity = 0x10A,
	WallPushSpeed = 0x10D,
	WallPushStartTime = 0x10C,
	WallRunCancelVelocity = 0x118,
	WallRunCancelVelocityNoRun = 0x119,
	WaterAddVelocity = 0xAD,
	WaterDeadDownDepthMax = 0x9F,
	WaterDeadDownDepthMaxSpeed = 0x9E,
	WaterDeadDownDepthMin = 0x9D,
	WaterDecreaseForce = 0xA7,
	WaterDecreaseForceInDeepDepth = 0xA8,
	WaterDrowningDepthMin = 0xA0,
	WaterDrowningNoBoostTime = 0xA1,
	WaterGravityScale = 0xAC,
	WaterTensionMaxVelocity = 0xA5,
	WaterTensionZeroVelocity = 0xA4,
	WindEffectCount = 0x236,
	WindEffectDirOffset = 0x23A,
	WindEffectIntervalTime = 0x241,
	WindEffectLife = 0x23F,
	WindEffectLifeRand = 0x240,
	WindEffectMeshPointCount = 0x237,
	WindEffectRandAmplitude = 0x238,
	WindEffectRandFrequency = 0x239,
	WindEffectSpeedMax = 0x23C,
	WindEffectSpeedMin = 0x23B,
	WindEffectStartPosRange = 0x242,
	WindEffectWidth = 0x23D,
	WindEffectWidthRand = 0x23E,
	XButtonHoming = 0x11B,
};

enum StageMissionType : uint32_t
{
	SMT_ghz100 = 0x00,
	SMT_ghz200 = 0x01,
	SMT_cpz100 = 0x02,
	SMT_cpz200 = 0x03,
	SMT_ssz100 = 0x04,
	SMT_ssz200 = 0x05,
	SMT_sph100 = 0x06,
	SMT_sph200 = 0x07,
	SMT_cte100 = 0x08,
	SMT_cte200 = 0x09,
	SMT_ssh100 = 0x0A,
	SMT_ssh200 = 0x0B,
	SMT_csc100 = 0x0C,
	SMT_csc200 = 0x0D,
	SMT_euc100 = 0x0E,
	SMT_euc200 = 0x0F,
	SMT_pla100 = 0x10,
	SMT_pla200 = 0x11,
	SMT_cnz100 = 0x12,
	SMT_emu	= 0x13,
	SMT_bms	= 0x14,
	SMT_bsd	= 0x15,
	SMT_bsl	= 0x16,
	SMT_bde	= 0x17,
	SMT_bpc	= 0x18,
	SMT_bne	= 0x19,
	SMT_blb	= 0x1A,
	SMT_pam000 = 0x1B,
	SMT_fig000 = 0x1C,
	SMT_evt041 = 0x1D,
	SMT_evt105 = 0x1E,
	SMT_evt121 = 0x1F,

	SMT_Mission1 = 0x100,
	SMT_Mission2 = 0x200,
	SMT_Mission3 = 0x300,
	SMT_Mission4 = 0x400,
	SMT_Mission5 = 0x500,

	SMT_BossHard = 0x100,
};

enum LanguageType : uint8_t
{
	LT_English,
	LT_Japanese,
	LT_French,
	LT_German,
	LT_Spanish,
	LT_Italian
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
	static void* const pCGlitterCreate = (void*)0xE73890;
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
	static void* const pCGlitterEnd = (void*)0xE72650;
	static void* const pCGlitterKill = (void*)0xE72570;
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

static float fGetPlayerParameter
(
	EPlayerParameter enumID
)
{
	static void* const pGetParameter = (void*)0x53A9F0;
	__asm
	{
		mov		eax, PLAYER_CONTEXT
		mov		eax, [eax]
		mov		eax, [eax + 0x27C]
		push	enumID
		call	[pGetParameter]
	}
}

static bool fGetPlayerParameterPtr
(
	EPlayerParameter enumID,
	void* pTable,
	float*& pValue
)
{
	uint32_t* ppValue = nullptr;
	float unknownScale = 0.0f;
	static void* const pGetParameterPtr = (void*)0xD975B0;
	__asm
	{
		push	pTable
		mov		eax, enumID
		lea		edx, [ppValue]
		lea		edi, [unknownScale]
		call	[pGetParameterPtr]
	}
	
	if (ppValue)
	{
		pValue = (float*)(*ppValue + 4);
		return true;
	}

	return false;
}

static bool fRaycast
(
	Eigen::Vector4f const& rayStartPos,
	Eigen::Vector4f const& rayEndPos,
	Eigen::Vector4f& outPos,
	Eigen::Vector4f& outNormal,
	uint32_t flag
)
{
	// sub_10BE3B0(&rayStartPos, flag, **(_DWORD ***)(*PLAYER_CONTEXT+ 1516), &outPos, &outNormal, &rayEndPos)
	static void* const pfRaycast = (void*)0x10BE3B0;
	__asm
	{
		mov		ecx, rayEndPos
		push	ecx
		mov		edx, outNormal
		push	edx
		mov		eax, outPos
		push	eax
		mov		ebx, PLAYER_CONTEXT
		mov		ebx, [ebx]
		mov     eax, [ebx + 5ECh]
		mov     edi, [eax]
		mov		edx, flag
		mov		eax, rayStartPos
		call	[pfRaycast]
	}
}

static bool fRaycast
(
	hh::math::CVector const& rayStartPos,
	hh::math::CVector const& rayEndPos,
	hh::math::CVector& outPos,
	hh::math::CVector& outNormal,
	uint32_t flag
)
{
	static void* const pfRaycast = (void*)0x10BE3B0;
	__asm
	{
		mov		ecx, rayEndPos
		push	ecx
		mov		edx, outNormal
		push	edx
		mov		eax, outPos
		push	eax
		mov		ebx, PLAYER_CONTEXT
		mov		ebx, [ebx]
		mov     eax, [ebx + 5ECh]
		mov     edi, [eax]
		mov		edx, flag
		mov		eax, rayStartPos
		call	[pfRaycast]
	}
}

static void* fGetScreenPosition
(
	Eigen::Vector4f const& pos3D,
	Eigen::Vector4f& pos2D
)
{
	static void* const pfGetScreenPosition = (void*)0xD61C40;
	__asm
	{
		mov		ecx, 0x1E0BE5C
		mov		ecx, [ecx]
		mov		eax, pos3D
		mov		ebx, pos2D
		push	ebx
		call	[pfGetScreenPosition]
	}
}

inline void fEventTrigger(void* This, int Event)
{
	FUNCTION_PTR(void*, __stdcall, fpEventTrigger, 0xD5ED00, void* This, int Event);
	fpEventTrigger(This, Event);
}

inline void fSendMessageToSetObject
(
	Hedgehog::Universe::CMessageActor* messageActor,
	uint32_t setObjectID,
	boost::shared_ptr<Hedgehog::Universe::Message> message
)
{
	FUNCTION_PTR(bool, __thiscall, fpSendMessageToSetObject, 0xEB3A60,
		Hedgehog::Universe::CMessageActor* messageActor,
		Sonic::CSetObjectManager* setObjectManager,
		uint32_t setObjectID,
		boost::shared_ptr<Hedgehog::Universe::Message> message
	);

	Sonic::CGameDocument* gameDocument = Sonic::CGameDocument::GetInstance().get().get();
	fpSendMessageToSetObject(messageActor, gameDocument->m_pGameActParameter->m_pSetObjectManager, setObjectID, message);
}

static void fDestroyGameObject
(
	void* pGameObject
)
{
	static void* const pfDestroyGameObject = (void*)0xD5FD10;
	__asm
	{
		mov		edi, pGameObject
		call	[pfDestroyGameObject]
	}
}

static void* fObjectPhysicsDynamicPosition
(
	void* This,
	hh::math::CVector& pos
)
{
	static void* const pfObjectPhysicsDynamicPosition = (void*)0xEA1C80;
	__asm
	{
		mov		ecx, pos
		mov		eax, This
		call	[pfObjectPhysicsDynamicPosition]
	}
}

static uint32_t GetButtonArrayIndex(Sonic::EKeyState key)
{
	// base on button mapping array at 0x15D3668
	switch (key)
	{
	case Sonic::EKeyState::eKeyState_A: return 0;
	case Sonic::EKeyState::eKeyState_B: return 1;
	case Sonic::EKeyState::eKeyState_LeftBumper: return 2;
	case Sonic::EKeyState::eKeyState_RightBumper: return 3;
	case Sonic::EKeyState::eKeyState_X: return 4;
	case Sonic::EKeyState::eKeyState_Y: return 11;
	case Sonic::EKeyState::eKeyState_LeftStick: return 31;
	case Sonic::EKeyState::eKeyState_RightTrigger: return 32;
	default: return UINT32_MAX;
	}
}

static bool fIsButtonTapped
(
	Sonic::EKeyState key
)
{
	uint32_t index = GetButtonArrayIndex(key);
	static void* const pfIsButtonTapped = (void*)0xD97E00;
	__asm
	{
		mov		ebx, PLAYER_CONTEXT
		mov		ebx, [ebx]
		mov     ecx, [ebx + 534h]
		mov     ecx, [ecx + 4h]		// StateFlags
		mov     eax, [ebx + 11Ch]	// CInputPad
		mov		edi, index
		call	[pfIsButtonTapped]
	}
}

static bool fIsButtonHeld
(
	Sonic::EKeyState key
)
{
	uint32_t index = GetButtonArrayIndex(key);
	static void* const pfIsButtonHeld = (void*)0xD97DA0;
	__asm
	{
		mov		ebx, PLAYER_CONTEXT
		mov		ebx, [ebx]
		mov     ecx, [ebx + 534h]
		mov     ecx, [ecx + 4h]		// StateFlags
		mov     esi, [ebx + 11Ch]	// CInputPad
		mov		edi, index
		call	[pfIsButtonHeld]
	}
}

inline void ObjectCGlitterPlayerOneShot(void* pObject, Hedgehog::Base::CSharedString const& name)
{
	uint32_t* CGlitterPlayer = *(uint32_t**)((uint32_t)pObject + 0xF8);
	void* matrixNode = (void*)((uint32_t)pObject + 0xB8);
	if (CGlitterPlayer && *CGlitterPlayer == 0x16D0514)
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

inline bool IsPlayerDead()
{
	if (!*PLAYER_CONTEXT) return false;
	return GetSonicStateFlags()->Dead;
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

inline bool IsPlayerExtendedBoost()
{
	if (!*PLAYER_CONTEXT) return false;
	return *(uint32_t*)((uint32_t)*PLAYER_CONTEXT + 0x680) == 2;
}

inline bool IsPlayerHangOn()
{
	if (!*PLAYER_CONTEXT) return false;
	return *(uint32_t*)((uint32_t)*PLAYER_CONTEXT + 0x7D8) != 0;
}

static bool IsPlayerAnimationFinished
(
	Sonic::Player::CPlayer* pPlayer
)
{
	static void* const pfIsPlayerAnimationFinished = (void*)0xCDF4C0;
	__asm
	{
		mov		eax, pPlayer
		cmp     dword ptr [eax + 244h], 0
		jz		jump

		mov     esi, [eax + 244h]
		call	[pfIsPlayerAnimationFinished]
		jmp		end

		jump:
		mov     al, 0

		end:
	}
}

static bool IsAnimationFinished
(
	Sonic::CAnimationStateMachine const* pStateMachine
)
{
	static void* const pfIsPlayerAnimationFinished = (void*)0xCDF4C0;
	__asm
	{
		mov     esi, pStateMachine
		call	[pfIsPlayerAnimationFinished]
	}
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

inline char* GetCurrentTerrain()
{
	uint32_t terrainAddress = GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x80, 0x20 });
	return *(char**)terrainAddress;
}

inline bool IsStageMission(uint32_t stageID)
{
	return (stageID & 0xFF00) > 0 && (stageID & 0xFF) <= SMT_pla200;
}

inline bool IsCurrentStageMission()
{
	return IsStageMission(GetCurrentStageID());
}

inline bool IsStageBoss(uint32_t stageID)
{
	stageID &= 0xFF;
	return stageID >= SMT_bms && stageID <= SMT_blb;
}

inline bool IsCurrentStageBoss()
{
	return IsStageBoss(GetCurrentStageID());
}

inline bool IsStagePlayed(uint32_t stageID)
{
	// Note: Only works for regular stages not mission
	// fake it as CHudGateMenuMain object
	uint32_t dummyAddress = (uint32_t)&stageID - 0x208;
	FUNCTION_PTR(bool, __stdcall, CHudGateMenuMain_IsStagePlayed, 0x107AE80, uint32_t pCHudGateMenuMain);
	return CHudGateMenuMain_IsStagePlayed(dummyAddress);
}

inline bool IsStageCompleted(uint32_t stageID)
{
	FUNCTION_PTR(bool, __stdcall, fpIsStageCompleted, 0x107ADC0, uint32_t stageID);
	return fpIsStageCompleted(stageID);
}

inline bool GetStageData
(
	uint32_t stageID, 
	uint32_t& bestScore,
	float& bestTime, 
	float& bestTime2, 
	float& bestTime3, 
	uint32_t& bestRank,
	uint32_t& bestRing,
	uint32_t& redRingCount
)
{
	FUNCTION_PTR(int, __fastcall, fpGetSaveSlotID, 0x552130, uint32_t stageID);
	int saveSlotID = fpGetSaveSlotID(stageID);

	uint32_t CGameParameterAddress = *(uint32_t*)GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4 });
	uint32_t unknownOffset = *(uint32_t*)(CGameParameterAddress + 0x7C) + 0x1C;
	uint32_t saveSlotAddress = 0x120 * saveSlotID + unknownOffset;
	if (saveSlotID < 0x8E && saveSlotAddress != 0)
	{
		bestScore = *(uint32_t*)(saveSlotAddress);
		if (IsStageCompleted(stageID))
		{
			bestTime = *(float*)(saveSlotAddress + 0x4);
			bestTime2 = *(float*)(saveSlotAddress + 0x8);
			bestTime3 = *(float*)(saveSlotAddress + 0xC);
		}
		else
		{
			bestTime = -1.0f;
			bestTime2 = -1.0f;
			bestTime3 = -1.0f;
		}
		bestRank = 4 - *(uint32_t*)(saveSlotAddress + 0x10);
		bestRing = *(uint32_t*)(saveSlotAddress + 0x14);

		if ((stageID & 0xFF00) > 0 && (stageID & 0xFF) <= 0x11)
		{
			redRingCount = 0;
		}
		else
		{
			FUNCTION_PTR(int, __fastcall, fpGetRedRingCount, 0xD591B0, uint32_t stageID, int dummy, uint32_t CGameParameter);
			redRingCount = fpGetRedRingCount(stageID, 0, CGameParameterAddress);
		}
		
		return true;
	}
	else
	{
		return false;
	}
}

inline void PlayStageMusic(char const* cueName, float fadeInTime)
{
	FUNCTION_PTR(bool, __stdcall, PlayStageMusicFromCueName, 0xD63070, void* gameDocument, Hedgehog::Base::CSharedString & cueName, float fadeInTime);
	Hedgehog::Base::CSharedString name = Hedgehog::Base::CSharedString(cueName);
	PlayStageMusicFromCueName(*(void**)0x1E0BE5C, name, fadeInTime);
}

inline void PlayBGM(char const* cueName, float fadeInTime)
{
	FUNCTION_PTR(unsigned int, __stdcall, PlayAudioFromCueName, 0xD62440, void* gameDocument, Hedgehog::Base::CSharedString& cueName, float fadeInTime);
	Hedgehog::Base::CSharedString name = Hedgehog::Base::CSharedString(cueName);
	PlayAudioFromCueName(*(void**)0x1E0BE5C, name, fadeInTime);
}

inline void StopBGM(char const* cueName, float fadeOutTime)
{
	FUNCTION_PTR(int, __stdcall, StopAudioFromCueName, 0xD61E40, void* gameDocument, Hedgehog::Base::CSharedString & cueName, float fadeOutTime);
	Hedgehog::Base::CSharedString name = Hedgehog::Base::CSharedString(cueName);
	StopAudioFromCueName(*(void**)0x1E0BE5C, name, fadeOutTime);
}

static void PlayEvent
(
	Sonic::CEventManager* pEventManager,
	hh::base::CSharedString const& eventName,
	hh::math::CMatrix44 const& matrix,
	uint32_t flag
)
{
	// m_pMember->m_pGameDocument->m_pMember->m_spEventManager
	static void* const pPlayEvent = (void*)0xB276E0;
	__asm
	{
		mov		edi, pEventManager
		push	flag
		push	matrix
		push	eventName
		call	[pPlayEvent]
	}
}

inline void* GetPlayer()
{
	if (!*PLAYER_CONTEXT) return 0;
	// Sonic::Player::CSonicContext -> Sonic::Player::CSonic
	return *(void**)((uint32_t)*PLAYER_CONTEXT + 0x110);
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

inline uint32_t* GetPlayerLives()
{
	if (!*PLAYER_CONTEXT) return 0;
	uint32_t lifeAddress = GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0x9FDC });
	return (uint32_t*)lifeAddress;
}

inline float* GetPlayerBoost()
{
	if (!*PLAYER_CONTEXT) return 0;
	return (float*)((uint32_t)*PLAYER_CONTEXT + 0x5BC);
}

inline float GetPlayerMaxBoost()
{
	return IsPlayerExtendedBoost() ? 200.0f : 100.0f;
}

inline float* GetPlayerMaxSpeed()
{
	if (!*PLAYER_CONTEXT) return 0;
	return (float*)((uint32_t)*PLAYER_CONTEXT + 0x53C);
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

inline void GetPlayerHUBTransform(Eigen::Vector3f& position, Eigen::Quaternionf& rotation)
{
	uint32_t posAddress = GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0x9FF0 });
	uint32_t rotAddress = GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0xA2D0 });
	position = *(Eigen::Vector3f*)posAddress;
	rotation = *(Eigen::Quaternionf*)rotAddress;
}

inline void SetPlayerPosition(Eigen::Vector3f const& position)
{
	if (!*PLAYER_CONTEXT) return;

	alignas(16) MsgSetPosition message;
	message.m_position = position;

	FUNCTION_PTR(void*, __thiscall, playerProcessMsgSetPosition, 0xE772D0, void* This, void* message);
	playerProcessMsgSetPosition(Common::GetPlayer(), &message);
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

	*(bool*)(result + 1512) = true; // velocity dirty
	*(bool*)(result + 1513) = false; // horizontal dirty

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

inline float GetPlayerModelScale()
{
	if (!*PLAYER_CONTEXT) return 1.0f;

	Sonic::Player::CPlayer const* player = Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer;
	float* animationPose = *(float**)((uint32_t)player + 0x23C);
	return animationPose[31];
}

inline void SetPlayerModelScale(float scale)
{
	if (!*PLAYER_CONTEXT) return;

	Sonic::Player::CPlayer const* player = Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer;
	float* animationPose = *(float**)((uint32_t)player + 0x23C);
	animationPose[31] = scale;
}

inline void SetPlayerOutOfControl(float duration)
{
	if (!*PLAYER_CONTEXT) return;

	FUNCTION_PTR(int, __stdcall, SetOutOfControl, 0xE5AC00, void* context, float duration);
	SetOutOfControl(Sonic::Player::CPlayerSpeedContext::GetInstance(), duration);
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
	void* pSonicContext = *PLAYER_CONTEXT;
	if (!pSonicContext) return;

	// Original code by Skyth: https://github.com/blueskythlikesclouds
	FUNCTION_PTR(void, __thiscall, CSonicSpeedProcMsgGetAnimationInfo, 0xE6A370, void* This, void* pMessage);
	void* player = *(void**)((uint32_t)*PLAYER_CONTEXT + 0x110);
	CSonicSpeedProcMsgGetAnimationInfo(player, &message);
}

inline void SonicContextGetItemType(uint32_t type)
{
	void* pSonicContext = *PLAYER_CONTEXT;
	if (!pSonicContext) return;

	alignas(16) MsgGetItemType message {};
	message.m_type = type;

	// Original code by Skyth: https://github.com/blueskythlikesclouds
	FUNCTION_PTR(void, __thiscall, CSonicSpeedProcMsgGetItemType, 0xE6D7D0, void* This, void* pMessage);
	void* player = *(void**)((uint32_t)*PLAYER_CONTEXT + 0x110);
	CSonicSpeedProcMsgGetItemType(player, &message);
}

inline void SonicContextRequestLocusEffect()
{
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

inline void SonicContextAddPlugin(Hedgehog::Base::CSharedString const& plugin)
{
	FUNCTION_PTR(void, __thiscall, addPlayerPlugin, 0xE77D80, Hedgehog::Base::CSharedString const* plugin, void* player);
	addPlayerPlugin(&plugin, Common::GetPlayer());
}

static void* SonicContextSetCollision(SonicCollision collisionType, bool enabled)
{
	static void* const pEnableFunc = (void*)0xE65610;
	static void* const pDisableFunc = (void*)0xE655C0;
	__asm
	{
		mov		edi, PLAYER_CONTEXT
		mov		edi, [edi]

		mov		ecx, collisionType
		mov		ecx, [ecx]
		push	ecx

		cmp		enabled, 0
		je		jump

		call	[pEnableFunc]
		jmp		end

		jump:
		call	[pDisableFunc]
		
		end:
	}
}

static void SonicContextUpdateRotationToVelocity
(
	void* pContext,
	Hedgehog::Math::CVector* pVelocity,
	bool updateMatrix
)
{
	static void* const pUpdateRotationToVelocity = (void*)0xE56860;
	__asm
	{
		push	updateMatrix
		push	pVelocity
		mov		ebx, pContext
		mov		esi, ebx
		call	[pUpdateRotationToVelocity]
	}
}

static void SonicContextSetInAirData
(
	void* pContext
)
{
	static void* const pSetInAirData = (void*)0xE4E0C0;
	__asm
	{
		mov		ebx, pContext
		mov		esi, ebx
		call	[pSetInAirData]
	}
}

inline void SonicContextHudHomingAttackOutro(Sonic::Player::CPlayerSpeedContext* context)
{
	if (context->m_spSonicHudHomingAttackImpl)
	{
		FUNCTION_PTR(int, __thiscall, SonicHudHomingAttackOutro, 0xDEBC90, Sonic::Player::CSonicHudHomingAttackImpl * This);
		SonicHudHomingAttackOutro(context->m_spSonicHudHomingAttackImpl.get());
	}
}

inline void SonicContextHudHomingAttackClear(Sonic::Player::CPlayerSpeedContext* context)
{
	if (context->m_spSonicHudHomingAttackImpl)
	{
		FUNCTION_PTR(void, __thiscall, SonicHudHomingAttackClear, 0xDEBC50, Sonic::Player::CSonicHudHomingAttackImpl * This);
		SonicHudHomingAttackClear(context->m_spSonicHudHomingAttackImpl.get());
	}
}

static void CStateSetStringBool
(
	hh::fnd::CStateMachineBase::CStateBase* This,
	Hedgehog::Base::CSharedString const& name,
	bool const& condition
)
{
	static void* const pCStateSetStringBool = (void*)0x50DD40;
	__asm
	{
		push	condition
		mov		edi, name
		mov		eax, This
		call	[pCStateSetStringBool]
	}
}

static void CopyCCsdProject
(
	Sonic::CCsdProject* pCsdProject, 
	Chao::CSD::RCPtr<Chao::CSD::CProject>& rcptr
)
{
	static void* const pCopyCCsdProject = (void*)0x10DCE40;
	__asm
	{
		push	rcptr
		mov		eax, pCsdProject
		call	[pCopyCCsdProject]
	}
}

inline void ToggleRigidBodyCollision(Sonic::CRigidBody* This, bool enabled)
{
	if (enabled)
	{
		FUNCTION_PTR(void, __thiscall, fpEnableCollision, 0x10C0F90, Sonic::CRigidBody* This, Sonic::CPhysicsWorld* pWorld);
		fpEnableCollision(This, This->m_pPhysicsWorld);
	}
	else
	{
		FUNCTION_PTR(void, __thiscall, fpDisableCollision, 0x10C0F40, Sonic::CRigidBody* This, Sonic::CPhysicsWorld* pWorld);
		fpDisableCollision(This, This->m_pPhysicsWorld);
	}
}

inline uint32_t MakeCollisionID(uint64_t self, uint64_t other)
{
	FUNCTION_PTR(uint32_t, __cdecl, fpMakeCollisionID, 0x10DFB30, int a1, int a2, bool a3, uint64_t self, uint64_t other);
	return fpMakeCollisionID(0, 0, true, self, other);
}

static void* ObjectPlaySound
(
	void* pObject,
	uint32_t cue,
	SharedPtrTypeless& soundHandle
)
{
	static void* const pObjectPlaySound = (void*)0xD6E970;
	__asm
	{
		mov		ecx, pObject
		mov     eax, [ecx + 0B8h]
		mov     ecx, [ecx + 0FCh]
		add     eax, 70h
		push    eax
		mov     eax, cue
		mov		edi, soundHandle
		call	[pObjectPlaySound]
	}
}

static void* ObjectEnableEventCollision
(
	Sonic::CEventCollisionHolder* pHolder
)
{
	if (!pHolder) return nullptr;

	static void* const pObjectEnableEventCollision = (void*)0x1182E70;
	__asm
	{
		mov		eax, pHolder
		call	[pObjectEnableEventCollision]
	}
}

static void* ObjectDisableEventCollision
(
	Sonic::CEventCollisionHolder* pHolder
)
{
	if (!pHolder) return nullptr;

	static void* const pObjectDisableEventCollision = (void*)0x1182F20;
	__asm
	{
		mov		eax, pHolder
		call	[pObjectDisableEventCollision]
	}
}

static void* ObjectToggleEventCollision
(
	Sonic::CEventCollisionHolder* pHolder,
	Hedgehog::Base::CStringSymbol const& symbol,
	bool enabled
)
{
	if (!pHolder) return nullptr;

	static void* const pObjectToggleEventCollision = (void*)0x1182DB0;
	__asm
	{
		mov		eax, pHolder
		mov		ecx, symbol
		push	enabled
		call	[pObjectToggleEventCollision]
	}
}

static void MakeSharedShapeCollision
(
	boost::shared_ptr<Sonic::CShapeCollision>& pShapeCollision,
	Sonic::CWorld* pWorld,
	hk2010_2_0::hkpShape* pShape,
	uint32_t collisionID
)
{
	uint32_t holder = (uint32_t)pWorld + 0x7C;
	static void* const pMakeSharedShapeCollision = (void*)0x11823C0;
	__asm
	{
		push	collisionID
		lea		ecx, [pShape]
		push	ecx
		lea		edx, [holder]
		push	edx
		mov		esi, pShapeCollision
		call	[pMakeSharedShapeCollision]
	}
}

static int fCCharacterProxyIntegrate
(
	Sonic::CCharacterProxy* proxy,
	float dt
)
{
	static void* const pCCharacterProxyIntegrate = (void*)0x10E3CF0;
	__asm
	{
		push	dt
		mov		eax, proxy
		call	[pCCharacterProxyIntegrate]
	}
}

static void EnemyChangeState
(
	void* pCEnemyBase,
	Hedgehog::Base::CSharedString const& name
)
{
	static void* const pEnemyChangeState = (void*)0xBDDE60;
	__asm
	{
		push	pCEnemyBase
		mov		eax, name
		call	[pEnemyChangeState]
	}
}

static void NPCAnimationChangeState
(
	void* pCEnemyBase,
	Hedgehog::Base::CSharedString const& name
)
{
	static void* const pNPCAnimationChangeState = (void*)0xB679A0;
	__asm
	{
		mov		esi, pCEnemyBase
		mov     edx, [esi + 124h]
		mov		eax, name
		call	[pNPCAnimationChangeState]
	}
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

inline void CreatePlayerSupportShockWave(hh::math::CVector const& pos, float height, float radius, float duration)
{
	struct ShockWaveParam
	{
		float m_height;
		float m_radius;
		float m_duration;
		size_t m_actorID;
	};
	FUNCTION_PTR(void*, __cdecl, fCreatePlayerSupportShockWave, 0x123D090, boost::shared_ptr<Sonic::CGameObject>& spObject, hh::math::CVector const* pos, ShockWaveParam const& param);
	boost::shared_ptr<Sonic::CGameObject> spObject;
	fCreatePlayerSupportShockWave(spObject, &pos, { height, radius, duration, Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_ActorID });
	Sonic::CGameDocument::GetInstance()->AddGameObject(spObject);
}

inline uint32_t GetSoundCueFromSynth(Hedgehog::Base::CSharedString const& name)
{
	FUNCTION_PTR(uint32_t, __thiscall, fpGetSoundCueFromSynth, 0x7624D0, void* pCSoundCri, Hedgehog::Base::CSharedString const& name);
	
	uint32_t* pCSoundCri = (uint32_t*)Common::GetMultiLevelAddress(0x1E79044, { 0x14, 0x0, 0x0 });
	return fpGetSoundCueFromSynth(pCSoundCri, name);
}

inline float GetSoundCueDuration(uint32_t cueID)
{
	FUNCTION_PTR(SharedPtrTypeless*, __thiscall, fpGetCSoundCueSheetStream, 0x762500, void* pCSoundCri, SharedPtrTypeless * pCSoundCueSheetStream, uint32_t cueID);
	FUNCTION_PTR(bool, __thiscall, fpGetSoundCueData, 0x763800, void* pCSoundCueSheetStream, uint32_t cueID, uint32_t * flag, uint32_t * sampleRate, uint32_t * numSamples);

	uint32_t* pCSoundCri = (uint32_t*)Common::GetMultiLevelAddress(0x1E79044, { 0x14, 0x0, 0x0 });
	SharedPtrTypeless pCSoundCueSheetStream;
	fpGetCSoundCueSheetStream(pCSoundCri, &pCSoundCueSheetStream, cueID);

	uint32_t flag = 0u;
	uint32_t sampleRate = 0u;
	uint32_t numSamples = 0u;
	if (fpGetSoundCueData(pCSoundCueSheetStream.get(), cueID, &flag, &sampleRate, &numSamples) && sampleRate > 0u)
	{
		return (float)numSamples / (float)sampleRate;
	}

	return 0.0f;
}

inline LanguageType GetVoiceLanguageType()
{
	return *(LanguageType*)Common::GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0x10 });
}

inline LanguageType GetUILanguageType()
{
	return *(LanguageType*)Common::GetMultiLevelAddress(0x1E66B34, { 0x8 });
}

inline bool SolveBallisticArc
(
	const Eigen::Vector3f& start, 
	const Eigen::Vector3f& end,
	const float speed, 
	const float gravity,
	const bool bUseHighArc, 
	Eigen::Vector3f& outTrajectory,
	float& outTime
)
{
	Eigen::Vector3f diff = end - start;
	Eigen::Vector3f diffXZ(diff.x(), 0.0f, diff.z());
	float groundDist = diffXZ.norm();

	float speed2 = speed * speed;
	float speed4 = speed2 * speed2;
	float y = diff.y();
	float x = groundDist;
	float gx = gravity * x;

	float root = speed4 - gravity * (gravity * x * x + 2 * y * speed2);

	// No solution
	if (root < 0) return false;

	root = std::sqrt(root);
	float lowAng = std::atan2f(speed2 - root, gx);
	float highAng = std::atan2f(speed2 + root, gx);

	Eigen::Vector3f groundDir = diffXZ.normalized();
	if (bUseHighArc)
	{
		outTrajectory = groundDir * std::cos(highAng) * speed + Eigen::Vector3f::UnitY() * std::sin(highAng) * speed;
	}
	else
	{
		outTrajectory = groundDir * std::cos(lowAng) * speed + Eigen::Vector3f::UnitY() * std::sin(lowAng) * speed;
	}

	float speedXZ = Eigen::Vector3f(outTrajectory.x(), 0.0f, outTrajectory.z()).norm();
	outTime = groundDist / speedXZ;

	return true;
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

inline bool IsModEnabled(std::string const& section, std::string const& name, std::string const& str, std::string* o_iniPath = nullptr)
{
	std::vector<std::string> modIniList;
	GetModIniList(modIniList);
	for (size_t i = 0; i < modIniList.size(); i++)
	{
		std::string const& config = modIniList[i];
		INIReader configReader(config);
		std::string value = configReader.Get(section, name, "");
		if (value == str)
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

inline bool GetModIDFromDLL(std::string const& name, std::string& o_modID)
{
	bool found = false;
	o_modID.clear();

	std::vector<std::string> modIniList;
	GetModIniList(modIniList);
	for (size_t i = 0; i < modIniList.size(); i++)
	{
		std::string const& config = modIniList[i];
		INIReader configReader(config);
		std::string value = configReader.Get("Main", "DLLFile", "");
		if (value == name)
		{
			if (found)
			{
				MessageBox(nullptr, TEXT("There are multiple mods with the same .dll"), TEXT("ERROR"), MB_ICONERROR);
				exit(-1);
			}

			found = true;
			o_modID = configReader.Get("Main", "ID", "");

			if (o_modID.empty())
			{
				MessageBox(nullptr, TEXT("One of the mods has no valid ID"), TEXT("ERROR"), MB_ICONERROR);
				exit(-1);
			}
		}
	}

	return found;
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

inline bool DoesArchiveExist(std::string const& archiveName, std::set<std::string> ignoreModList = {})
{
	std::vector<std::string> modIniList;
	GetModIniList(modIniList);
	for (std::string const& modIni : modIniList)
	{
		bool ignore = false;
		INIReader configReader(modIni);
		std::string modName = configReader.Get("Desc", "Title", "");
		for (std::string const& ignoreMod : ignoreModList)
		{
			if (modName.find(ignoreMod) != std::string::npos)
			{
				ignore = true;
				break;
			}
		}
		
		if (ignore)
		{
			continue;
		}

		std::string folder = modIni.substr(0, modIni.length() - 7);
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(folder))
		{
			if (dirEntry.path().filename() == archiveName)
			{
				return true;
			}
		}
	}
	return false;
}

} // namespace Common
