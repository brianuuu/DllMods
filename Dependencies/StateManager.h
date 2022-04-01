#pragma once

enum class StateAction
{
	Stand,
	Walk,
	JumpShort,
	Jump,
	MoveStop,
	Brake,
	BrakeEnd,
	BrakeTurn,
	Fall,
	Land,
	LandJumpShort,
	DeadLanding,
	JumpSpring,
	JumpSpringHeadLand,
	HangOn,
	ReactionJump,    // THIS CRASHES THE GAME AND IS REALLY FUCKED UP
	ReactionLand,
	TrickJump,
	CrashWall,
	CrashWallDown,
	PressDead,
	PressDamage,
	WallJumpReady,
	WallJump,
	Battery,    // Game crashes lol
	Sliding,
	ShoesSliding,
	GoalAir,
	Goal,
	ExternalControl_Crash,
	TrickJumpSuccess,
	FinishExternalControlAir,
	PushingWall,
	PushObject,
	KickBox,
	PlayAnimation,
	NormalDamage,
	NormalDamageStandUp,
	NormalDamageAir,
	NormalDamageDead,
	NormalDamageDeadAir,
	DamageCancel,
	DamageOnRunning,
	DamageOnStomping,
	TrickAttackLand,
	TrickAttack,
	SpecialJump,
	DrowningDead,
	MoveToPositionAndWait,
	WallRunDamage,
	LookUp,
	Stagger,
	TakeBreath_Crash,
	Suffocate_Crash,
	HipSliding,
	AdlibTrick,
	AdlibTrickFailure,
	Pipe,
	AirBoost,
	Drift,
	NoStanding,
	HomingAttack,
	HomingAttackAfter,
	Grind,
	GrindJumpSide_Crash,
	GrindSquat_Crash,
	GrindJumpShort_Crash,
	GrindLandJumpShort_Crash,
	GrindSwitch_Crash,
	GrindDamageMiddle_Crash,
	GrindToWallWalk_Crash,
	OnSlipFloor,
	Stepping_Crash,
	TramRiding_Crash,
	BeforeBoundJump,
	BoundJump,
	StumbleAir,
	StumbleGround,
	QuickStep,
	RunQuickStep,
	StartDash,
	StartEvent,
	StartCrouching,
	BoardWalk,
	BoardGetOn,
	BoardGetOff,
	BoardNormalDamage,
	BoardJumpShort,
	BoardJump,
	BoardAdlibTrick,
	BoardQuickStep,
	BoardRunQuickStep,
	BoardFall,
	BoardGrind,
	BoardGrindJumpShort,
	BoardGrindJumpSide,
	BoardGrindLandJumpShort,
	BoardLandJumpShort,
	BoardAirBoost,
	BoardJumpSpring,
	BoardDrift,
	JumpHurdle,
	Squat,
	SlidingEnd,
	SelectJump,
	JumpSelector,
	Stomping,
	StompingLand,
	LightSpeedDash,
	SelectJumpAfter,
	DivingFloat,
	DivingDive,
	DivingDamage,
	SquatKick,
	OnIce,
	DamageFreeze,
	DamageShock,
	DamageNearFar,
	Spin,
	SpinCharge,
	SpinChargeSliding,
	SpinSliding,
	SquatCharge,
	FloatingBoost,
	TransformRocket,
	TransformSpike,
	TransformSp,
	TransformStandard,
	RocketIdle,
	RocketLaunch,
	RocketEnd,
	RocketOverHeat,
	SpikeIdle,
	SpikeWalk,
	SpikeFall,
	SpikeJump,
	SpikeLand,
	SpikeCharge,
	SpikeBoost,
	SpikeJumpSpring,
	SpikeSpecialJump,
	SpikeDamageShock,
	SpikeHomingAttack,
	SpikeHomingAttackAfter,
	NONE
};


namespace StateManager
{

inline int* currentStateStringPtrVolatile;
inline char currentStateName[256];

// Credit to Sajid for finding this out in his double jump mod.
inline void* changeStateOriginal = (void*)0xE4FF30;

// This will help us check what state we're currently active in.
inline void SetSharedStringPtr()
{
	const auto currentStateStringPtr = (const char*)*currentStateStringPtrVolatile;
	strcpy(currentStateName, currentStateStringPtr);
	printf("[StateManager] State change: %s\n", currentStateName);
}

inline uint32_t changeStateJumpAddr = 0x00E4FF37;
inline void __declspec(naked) ChangeStateHOOK()
{
	__asm
	{
		// Original begining of function.
		push    ecx
		push    esi
		mov     esi, ecx
		push    edi
		mov     edi, eax

		// We need eax. Let's pipe it into our CSharedString* real quick.
		mov		currentStateStringPtrVolatile, eax
	}

	// Grab this volatile value 
#pragma region GrabValue
	__asm
	{
		push	eax
		push	ecx
		push	edx
	}

	SetSharedStringPtr();

	__asm
	{
		pop		edx
		pop		ecx
		pop		eax
	}
#pragma endregion 

	__asm
	{
		// Go back to the rest of the function now.
		jmp		[changeStateJumpAddr]
	}
}

// Gets us a string from our enum. Very helpful, especially when we're checking which state we're currently in.
inline const char* StringFromActionEnum(StateAction state)
{
	const char* result;

	switch (state)
	{
	default:
	case StateAction::Stand:
	{
		result = "Stand";
		break;
	}
	case StateAction::Walk:
	{
		result = "Walk";
		break;
	}
	case StateAction::JumpShort:
	{
		result = "JumpShort";
		break;
	}
	case StateAction::Jump:
	{
		result = "Jump";
		break;
	}
	case StateAction::MoveStop:
	{
		result = "MoveStop";
		break;
	}
	case StateAction::Brake:
	{
		result = "Brake";
		break;
	}
	case StateAction::BrakeEnd:
	{
		result = "BrakeEnd";
		break;
	}
	case StateAction::BrakeTurn:
	{
		result = "BrakeTurn";
		break;
	}
	case StateAction::Fall:
	{
		result = "Fall";
		break;
	}
	case StateAction::Land:
	{
		result = "Land";
		break;
	}
	case StateAction::LandJumpShort:
	{
		result = "LandJumpShort";
		break;
	}
	case StateAction::DeadLanding:
	{
		result = "DeadLanding";
		break;
	}
	case StateAction::JumpSpring:
	{
		result = "JumpSpring";
		break;
	}
	case StateAction::JumpSpringHeadLand:
	{
		result = "JumpSpringHeadLand";
		break;
	}
	case StateAction::HangOn:
	{
		result = "HangOn";
		break;
	}
	case StateAction::ReactionJump:
	{
		result = "ReactionJump";
		break;
	}
	case StateAction::ReactionLand:
	{
		result = "ReactionLand";
		break;
	}
	case StateAction::TrickJump:
	{
		result = "TrickJump";
		break;
	}
	case StateAction::CrashWall:
	{
		result = "CrashWall";
		break;
	}
	case StateAction::CrashWallDown:
	{
		result = "CrashWallDown";
		break;
	}
	case StateAction::PressDead:
	{
		result = "PressDead";
		break;
	}
	case StateAction::PressDamage:
	{
		result = "PressDamage";
		break;
	}
	case StateAction::WallJumpReady:
	{
		result = "WallJumpReady";
		break;
	}
	case StateAction::WallJump:
	{
		result = "WallJump";
		break;
	}
	case StateAction::Battery:
	{
		result = "Battery";
		break;
	}
	case StateAction::Sliding:
	{
		result = "Sliding";
		break;
	}
	case StateAction::ShoesSliding:
	{
		result = "ShoesSliding";
		break;
	}
	case StateAction::GoalAir:
	{
		result = "GoalAir";
		break;
	}
	case StateAction::Goal:
	{
		result = "Goal";
		break;
	}
	case StateAction::ExternalControl_Crash:
	{
		result = "ExternalControl_Crash";
		break;
	}
	case StateAction::TrickJumpSuccess:
	{
		result = "TrickJumpSuccess";
		break;
	}
	case StateAction::FinishExternalControlAir:
	{
		result = "FinishExternalControlAir";
		break;
	}
	case StateAction::PushingWall:
	{
		result = "PushingWall";
		break;
	}
	case StateAction::PushObject:
	{
		result = "PushObject";
		break;
	}
	case StateAction::KickBox:
	{
		result = "KickBox";
		break;
	}
	case StateAction::PlayAnimation:
	{
		result = "PlayAnimation";
		break;
	}
	case StateAction::NormalDamage:
	{
		result = "NormalDamage";
		break;
	}
	case StateAction::NormalDamageStandUp:
	{
		result = "NormalDamageStandUp";
		break;
	}
	case StateAction::NormalDamageAir:
	{
		result = "NormalDamageAir";
		break;
	}
	case StateAction::NormalDamageDead:
	{
		result = "NormalDamageDead";
		break;
	}
	case StateAction::NormalDamageDeadAir:
	{
		result = "NormalDamageDeadAir";
		break;
	}
	case StateAction::DamageCancel:
	{
		result = "DamageCancel";
		break;
	}
	case StateAction::DamageOnRunning:
	{
		result = "DamageOnRunning";
		break;
	}
	case StateAction::DamageOnStomping:
	{
		result = "DamageOnStomping";
		break;
	}
	case StateAction::TrickAttackLand:
	{
		result = "TrickAttackLand";
		break;
	}
	case StateAction::TrickAttack:
	{
		result = "TrickAttack";
		break;
	}
	case StateAction::SpecialJump:
	{
		result = "SpecialJump";
		break;
	}
	case StateAction::DrowningDead:
	{
		result = "DrowningDead";
		break;
	}
	case StateAction::MoveToPositionAndWait:
	{
		result = "MoveToPositionAndWait";
		break;
	}
	case StateAction::WallRunDamage:
	{
		result = "WallRunDamage";
		break;
	}
	case StateAction::LookUp:
	{
		result = "LookUp";
		break;
	}
	case StateAction::Stagger:
	{
		result = "Stagger";
		break;
	}
	case StateAction::TakeBreath_Crash:
	{
		result = "TakeBreath_Crash";
		break;
	}
	case StateAction::Suffocate_Crash:
	{
		result = "Suffocate_Crash";
		break;
	}
	case StateAction::HipSliding:
	{
		result = "HipSliding";
		break;
	}
	case StateAction::AdlibTrick:
	{
		result = "AdlibTrick";
		break;
	}
	case StateAction::AdlibTrickFailure:
	{
		result = "AdlibTrickFailure";
		break;
	}
	case StateAction::Pipe:
	{
		result = "Pipe";
		break;
	}
	case StateAction::AirBoost:
	{
		result = "AirBoost";
		break;
	}
	case StateAction::Drift:
	{
		result = "Drift";
		break;
	}
	case StateAction::NoStanding:
	{
		result = "NoStanding";
		break;
	}
	case StateAction::HomingAttack:
	{
		result = "HomingAttack";
		break;
	}
	case StateAction::HomingAttackAfter:
	{
		result = "HomingAttackAfter";
		break;
	}
	case StateAction::Grind:
	{
		result = "Grind";
		break;
	}
	case StateAction::GrindJumpSide_Crash:
	{
		result = "GrindJumpSide_Crash";
		break;
	}
	case StateAction::GrindSquat_Crash:
	{
		result = "GrindSquat_Crash";
		break;
	}
	case StateAction::GrindJumpShort_Crash:
	{
		result = "GrindJumpShort_Crash";
		break;
	}
	case StateAction::GrindLandJumpShort_Crash:
	{
		result = "GrindLandJumpShort_Crash";
		break;
	}
	case StateAction::GrindSwitch_Crash:
	{
		result = "GrindSwitch_Crash";
		break;
	}
	case StateAction::GrindDamageMiddle_Crash:
	{
		result = "GrindDamageMiddle_Crash";
		break;
	}
	case StateAction::GrindToWallWalk_Crash:
	{
		result = "GrindToWallWalk_Crash";
		break;
	}
	case StateAction::OnSlipFloor:
	{
		result = "OnSlipFloor";
		break;
	}
	case StateAction::Stepping_Crash:
	{
		result = "Stepping_Crash";
		break;
	}
	case StateAction::TramRiding_Crash:
	{
		result = "TramRiding_Crash";
		break;
	}
	case StateAction::BeforeBoundJump:
	{
		result = "BeforeBoundJump";
		break;
	}
	case StateAction::BoundJump:
	{
		result = "BoundJump";
		break;
	}
	case StateAction::StumbleAir:
	{
		result = "StumbleAir";
		break;
	}
	case StateAction::StumbleGround:
	{
		result = "StumbleGround";
		break;
	}
	case StateAction::QuickStep:
	{
		result = "QuickStep";
		break;
	}
	case StateAction::RunQuickStep:
	{
		result = "RunQuickStep";
		break;
	}
	case StateAction::StartDash:
	{
		result = "StartDash";
		break;
	}
	case StateAction::StartEvent:
	{
		result = "StartEvent";
		break;
	}
	case StateAction::StartCrouching:
	{
		result = "StartCrouching";
		break;
	}
	case StateAction::BoardWalk:
	{
		result = "BoardWalk";
		break;
	}
	case StateAction::BoardGetOn:
	{
		result = "BoardGetOn";
		break;
	}
	case StateAction::BoardGetOff:
	{
		result = "BoardGetOff";
		break;
	}
	case StateAction::BoardNormalDamage:
	{
		result = "BoardNormalDamage";
		break;
	}
	case StateAction::BoardJumpShort:
	{
		result = "BoardJumpShort";
		break;
	}
	case StateAction::BoardJump:
	{
		result = "BoardJump";
		break;
	}
	case StateAction::BoardAdlibTrick:
	{
		result = "BoardAdlibTrick";
		break;
	}
	case StateAction::BoardQuickStep:
	{
		result = "BoardQuickStep";
		break;
	}
	case StateAction::BoardRunQuickStep:
	{
		result = "BoardRunQuickStep";
		break;
	}
	case StateAction::BoardFall:
	{
		result = "BoardFall";
		break;
	}
	case StateAction::BoardGrind:
	{
		result = "BoardGrind";
		break;
	}
	case StateAction::BoardGrindJumpShort:
	{
		result = "BoardGrindJumpShort";
		break;
	}
	case StateAction::BoardGrindJumpSide:
	{
		result = "BoardGrindJumpSide";
		break;
	}
	case StateAction::BoardGrindLandJumpShort:
	{
		result = "BoardGrindLandJumpShort";
		break;
	}
	case StateAction::BoardLandJumpShort:
	{
		result = "BoardLandJumpShort";
		break;
	}
	case StateAction::BoardAirBoost:
	{
		result = "BoardAirBoost";
		break;
	}
	case StateAction::BoardJumpSpring:
	{
		result = "BoardJumpSpring";
		break;
	}
	case StateAction::BoardDrift:
	{
		result = "BoardDrift";
		break;
	}
	case StateAction::JumpHurdle:
	{
		result = "JumpHurdle";
		break;
	}
	case StateAction::Squat:
	{
		result = "Squat";
		break;
	}
	case StateAction::SlidingEnd:
	{
		result = "SlidingEnd";
		break;
	}
	case StateAction::SelectJump:
	{
		result = "SelectJump";
		break;
	}
	case StateAction::JumpSelector:
	{
		result = "JumpSelector";
		break;
	}
	case StateAction::Stomping:
	{
		result = "Stomping";
		break;
	}
	case StateAction::StompingLand:
	{
		result = "StompingLand";
		break;
	}
	case StateAction::LightSpeedDash:
	{
		result = "LightSpeedDash";
		break;
	}
	case StateAction::SelectJumpAfter:
	{
		result = "SelectJumpAfter";
		break;
	}
	case StateAction::DivingFloat:
	{
		result = "DivingFloat";
		break;
	}
	case StateAction::DivingDive:
	{
		result = "DivingDive";
		break;
	}
	case StateAction::DivingDamage:
	{
		result = "DivingDamage";
		break;
	}
	case StateAction::SquatKick:
	{
		result = "SquatKick";
		break;
	}
	case StateAction::OnIce:
	{
		result = "OnIce";
		break;
	}
	case StateAction::DamageFreeze:
	{
		result = "DamageFreeze";
		break;
	}
	case StateAction::DamageShock:
	{
		result = "DamageShock";
		break;
	}
	case StateAction::DamageNearFar:
	{
		result = "DamageNearFar";
		break;
	}
	case StateAction::Spin:
	{
		result = "Spin";
		break;
	}
	case StateAction::SpinCharge:
	{
		result = "SpinCharge";
		break;
	}
	case StateAction::SpinChargeSliding:
	{
		result = "SpinChargeSliding";
		break;
	}
	case StateAction::SpinSliding:
	{
		result = "SpinSliding";
		break;
	}
	case StateAction::SquatCharge:
	{
		result = "SquatCharge";
		break;
	}
	case StateAction::FloatingBoost:
	{
		result = "FloatingBoost";
		break;
	}
	case StateAction::TransformRocket:
	{
		result = "TransformRocket";
		break;
	}
	case StateAction::TransformSpike:
	{
		result = "TransformSpike";
		break;
	}
	case StateAction::TransformSp:
	{
		result = "TransformSp";
		break;
	}
	case StateAction::TransformStandard:
	{
		result = "TransformStandard";
		break;
	}
	case StateAction::RocketIdle:
	{
		result = "RocketIdle";
		break;
	}
	case StateAction::RocketLaunch:
	{
		result = "RocketLaunch";
		break;
	}
	case StateAction::RocketEnd:
	{
		result = "RocketEnd";
		break;
	}
	case StateAction::RocketOverHeat:
	{
		result = "RocketOverHeat";
		break;
	}
	case StateAction::SpikeIdle:
	{
		result = "SpikeIdle";
		break;
	}
	case StateAction::SpikeWalk:
	{
		result = "SpikeWalk";
		break;
	}
	case StateAction::SpikeFall:
	{
		result = "SpikeFall";
		break;
	}
	case StateAction::SpikeJump:
	{
		result = "SpikeJump";
		break;
	}
	case StateAction::SpikeLand:
	{
		result = "SpikeLand";
		break;
	}
	case StateAction::SpikeCharge:
	{
		result = "SpikeCharge";
		break;
	}
	case StateAction::SpikeBoost:
	{
		result = "SpikeBoost";
		break;
	}
	case StateAction::SpikeJumpSpring:
	{
		result = "SpikeJumpSpring";
		break;
	}
	case StateAction::SpikeSpecialJump:
	{
		result = "SpikeSpecialJump";
		break;
	}
	case StateAction::SpikeDamageShock:
	{
		result = "SpikeDamageShock";
		break;
	}
	case StateAction::SpikeHomingAttack:
	{
		result = "SpikeHomingAttack";
		break;
	}
	case StateAction::SpikeHomingAttackAfter:
	{
		result = "SpikeHomingAttackAfter";
		break;
	}
	}

	return result;
}

// One-off string comparison. Shitty, but necessary, if we want to check which state we're in.
// Thanks Unleashed.
inline bool isCurrentAction(StateAction state)
{
	const char* actionChar = StringFromActionEnum(state);

	const std::string actionString = actionChar;
	const std::string cStateString = currentStateName;

	return actionString == cStateString;
}

inline void __cdecl ChangeState(Hedgehog::Base::CSharedString* state, int* context)
{
	__asm
	{
		mov		eax, state
		mov		ecx, context
		call	[changeStateOriginal]
	}
}

// Override that takes in an enum and does conversion, minimizing user error.
inline void ChangeState(StateAction action, int* context)
{
	const char* stateString = StringFromActionEnum(action);
	auto state = Hedgehog::Base::CSharedString(stateString);
	ChangeState(&state, context);
}

// Overrides that take in a context and convert to an int, for compatibility's sake.
inline void ChangeState(Hedgehog::Base::CSharedString* state, CSonicContext* context)
{
	auto* const contextPtr = reinterpret_cast<int*>(context);
	ChangeState(state, contextPtr);
}

// Same as above but uses the enum method. VERY LIKELY the most common use case.
inline void ChangeState(StateAction state, CSonicContext* context)
{
	auto* const contextPtr = reinterpret_cast<int*>(context);
	ChangeState(state, contextPtr);
}

} // namespace StateManager
