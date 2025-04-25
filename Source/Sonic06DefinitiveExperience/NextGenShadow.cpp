#include "NextGenShadow.h"
#include "Configuration.h"
#include "Application.h"
#include "AnimationSetPatcher.h"
#include "CustomCamera.h"
#include "EnemyShock.h"

//---------------------------------------------------
// Animation
//---------------------------------------------------
void NextGenShadow::setAnimationSpeed_Shadow(NextGenAnimation& data)
{
    data.jog_playbackSpeed = 2.5f;
    data.jog_speedFactor = -1.0f;
    data.run_playbackSpeed = 1.69f;
    data.run_speedFactor = -1.0f;
    data.dash_playbackSpeed = 1.69f;
    data.dash_speedFactor = -1.0f;
    data.jet_playbackSpeed = 1.69f;;
    data.jet_speedFactor = -1.0f;
    data.jetWall_playbackSpeed = 1.17f;
    data.jetWall_speedFactor = -1.0f;
    data.boost_playbackSpeed = 1.27f;
    data.boost_speedFactor = -1.0f;
    data.boostWall_playbackSpeed = 0.64f;
    data.boostWall_speedFactor = -1.0f;
}

//---------------------------------------------------
// Main Variables
//---------------------------------------------------
bool NextGenShadow::m_isSquatKick = false;
bool NextGenShadow::m_isBrakeFlip = false;
Eigen::Vector3f NextGenShadow::m_brakeFlipDir(0, 0, 1);
float NextGenShadow::m_squatKickSpeed = 0.0f;
float const cShadow_squatKickPressMaxTime = 0.3f;
float const cShadow_startTeleportTime = 1.0f;

// Triple kick
int NextGenShadow::m_tripleKickCount = -1;
bool NextGenShadow::m_tripleKickBuffered = false;
bool NextGenShadow::m_tripleKickShockWaveSpawned = false;
float const cShadow_tripleKickShockWaveSpawn = 0.2f;
float const cShadow_tripleKickShockWaveDuration = 0.5f;
float const cShadow_tripleKickShockWaveHeight = 1.0f;
float const cShadow_tripleKickShockWaveRadius = 4.0f;

// Chaos Spear
std::vector<NextGenShadow::TargetData> NextGenShadow::m_targetData;
uint8_t const cShadow_chaosSpearMaxCount = 5;
uint8_t const cShadow_chaosSpearSuperMaxCount = 8;
float const cShadow_chaosSpearSpeed = 50.0f;
float const cShadow_chaosSpearLife = 1.0f;
float const cShadow_chaosSpearAddTime = 0.05f;
float const cShadow_chaosSpearAccumulate = 0.5f;
float const cShadow_chaosSpearStiffening = 0.3f;
float const cShadow_chaosSpearSuperStiffening = 0.25f;
float const cShadow_chaosSpearUpAngle = 80.0f * DEG_TO_RAD;
float const cShadow_chaosSpearDownAngle = 30.0f * DEG_TO_RAD;
float const cShadow_chaosSpearTurnRate = 180.0f * DEG_TO_RAD;
float const cShadow_chaosSpearSuperTurnRate = 720.0f * DEG_TO_RAD;
float const cShadow_chaosSpearShockDuration = 8.0f;
float const cShadow_chaosSpearSuperLockDist = 30.0f;

// Chaos Snap
bool NextGenShadow::m_chaosSnapActivated = false;
bool NextGenShadow::m_chaosSnapNoDamage = false;
float const cShadow_chaosSnapWaitTime = 0.1f;
float const cShadow_chaosSnapStartHold = 0.25f;

// Chaos Blast
float const cShadow_chaosBlastWaitTime = 1.6f;
float const cShadow_chaosBlastAttackTime = 1.7f;
float const cShadow_chaosBlastRadius = 16.0f; // same as green Gem
float const cShadow_chaosBlastDuration = 0.3f;
float const cShadow_chaosBlastVelocityObjPhy = 30.0f;
float const cShadow_chaosBlastVelocityEnemy = 15.0f;

// Chaos Control
float const cShadow_chaosControlDuration = 10.0f;
float const cShadow_chaosControlSlowScale = 0.1f;
float const cShadow_chaosControlSlowTime = 0.5f;
bool isChaosControl = false;

// Sliding/Spindash
bool slidingEndWasSliding_Shadow = false;
bool NextGenShadow::m_isSpindash = false;
bool NextGenShadow::m_isSliding = false;
float NextGenShadow::m_slidingTime = 0.0f;
float NextGenShadow::m_slidingSpeed = 0.0f;
float const cShadow_slidingTime = 3.0f;
float const cShadow_slidingSpeedMin = 10.0f;
float const cShadow_slidingSpeedMax = 16.0f;
float const cShadow_spindashTime = 3.0f;
float const cShadow_spindashSpeed = 30.0f;

Eigen::Vector3f NextGenShadow::m_holdPosition = Eigen::Vector3f::Zero();
int NextGenShadow::m_chaosAttackCount = -1;
bool NextGenShadow::m_chaosAttackBuffered = false;
float const cShadow_chaosAttackWaitTime = 0.2f;

float NextGenShadow::m_xHeldTimer = 0.0f;
bool NextGenShadow::m_enableAutoRunAction = true;

// Chaos Boost
uint8_t NextGenShadow::m_chaosBoostLevel = 0u;
float NextGenShadow::m_chaosMaturity = 0.0f;
float const cShadow_chaosBoostStartTime = 1.2f;

NextGenShadow::OverrideType NextGenShadow::m_overrideType = NextGenShadow::OverrideType::SH_None;

//---------------------------------------------------
// Jet Effect
//---------------------------------------------------
bool NextGenShadow::ShouldPlayJetEffect()
{
    static std::set<std::string> c_jetAnimations =
    {
        "Jog", "JogL", "JogR",
        "Run", "RunL", "RunR",
        "Dash", "DashL", "DashR",
        "Jet", "JetL", "JetR", "JetWallL", "JetWallR",
        "Boost", "BoostL", "BoostR", "BoostWallL", "BoostWallR",
        "AirBoost", "FloatingBoost", "RunQuickStepL", "RunQuickStepR"
    };

    alignas(16) MsgGetAnimationInfo message {};
    Common::SonicContextGetAnimationInfo(message);

    return IsModelVisible() && c_jetAnimations.count(message.m_name);
}

SharedPtrTypeless jetRightFront;
SharedPtrTypeless jetRightBack;
SharedPtrTypeless jetLeftFront;
SharedPtrTypeless jetLeftBack;
SharedPtrTypeless superJetRightFront;
SharedPtrTypeless superJetRightBack;
SharedPtrTypeless superJetLeftFront;
SharedPtrTypeless superJetLeftBack;
void NextGenShadow::SetJetEffectVisible(bool visible, hh::mr::CSingleElement* pModel, bool isSuper)
{
    if (visible)
    {
        {
            auto attachBone = pModel->GetNode("RightFoot");
            Common::fCGlitterCreate(*PLAYER_CONTEXT, isSuper ? superJetRightBack : jetRightBack, &attachBone, "ef_bo_sha_yh2_jet_back", 1);
        }
        {
            auto attachBone = pModel->GetNode("RightToeBase");
            Common::fCGlitterCreate(*PLAYER_CONTEXT, isSuper ? superJetRightFront : jetRightFront, &attachBone, "ef_bo_sha_yh2_jet_front", 1);
        }
        {
            auto attachBone = pModel->GetNode("LeftFoot");
            Common::fCGlitterCreate(*PLAYER_CONTEXT, isSuper ? superJetLeftBack : jetLeftBack, &attachBone, "ef_bo_sha_yh2_jet_back", 1);
        }
        {
            auto attachBone = pModel->GetNode("LeftToeBase");
            Common::fCGlitterCreate(*PLAYER_CONTEXT, isSuper ? superJetLeftFront : jetLeftFront, &attachBone, "ef_bo_sha_yh2_jet_front", 1);
        }
    }
    else if (isSuper ? superJetRightFront : jetRightFront)
    {
        if (isSuper)
        {
            Common::fCGlitterEnd(*PLAYER_CONTEXT, superJetRightFront, false);
            Common::fCGlitterEnd(*PLAYER_CONTEXT, superJetRightBack, false);
            Common::fCGlitterEnd(*PLAYER_CONTEXT, superJetLeftFront, false);
            Common::fCGlitterEnd(*PLAYER_CONTEXT, superJetLeftBack, false);

            superJetRightFront = nullptr;
            superJetRightBack = nullptr;
            superJetLeftFront = nullptr;
            superJetLeftBack = nullptr;
        }
        else
        {
            Common::fCGlitterEnd(*PLAYER_CONTEXT, jetRightFront, false);
            Common::fCGlitterEnd(*PLAYER_CONTEXT, jetRightBack, false);
            Common::fCGlitterEnd(*PLAYER_CONTEXT, jetLeftFront, false);
            Common::fCGlitterEnd(*PLAYER_CONTEXT, jetLeftBack, false);

            jetRightFront = nullptr;
            jetRightBack = nullptr;
            jetLeftFront = nullptr;
            jetLeftBack = nullptr;
        }
    }
}

bool NextGenShadow::IsModelVisible()
{
    auto const& model = Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_spCharacterModel;
    return model->m_spModel->m_NodeGroupModels[0]->m_Visible
        |= model->m_spModel->m_NodeGroupModels[1]->m_Visible;
}

bool jetSoundIsLeft = false;
HOOK(int, __fastcall, NextGenShadow_AssignFootstepFloorCues, 0xDFD420, Sonic::Player::CPlayerSpeedContext* context, void* Edx, int stepID)
{
    if (!NextGenShadow::IsModelVisible())
    {
        return 0;
    }

    if ((stepID == 0 || stepID == 1) && NextGenShadow::ShouldPlayJetEffect()) // walk or run
    {
        jetSoundIsLeft = !jetSoundIsLeft;
        return jetSoundIsLeft ? 80041041 : 80041042;
    }
    else
    {
        return originalNextGenShadow_AssignFootstepFloorCues(context, Edx, stepID);
    }
}

HOOK(void, __fastcall, NextGenShadow_CSonicUpdate, 0xE6BF20, Sonic::Player::CPlayerSpeed* This, void* Edx, float* dt)
{
    if (*pModernSonicContext)
    {
        auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
        hh::mr::CSingleElement* pModel = context->m_pPlayer->m_spCharacterModel.get();

        // jet effect
        if (NextGenShadow::ShouldPlayJetEffect() && !Common::IsPlayerSuper())
        {
            if (!jetRightFront)
            {
                NextGenShadow::SetJetEffectVisible(true, pModel, false);
            }
        }
        else if (jetRightFront)
        {
            NextGenShadow::SetJetEffectVisible(false, pModel, false);
        }

        // chaos boost drain
        if (NextGenShadow::m_chaosBoostLevel > 0 && !Common::IsPlayerSuper())
        {
            float* currentBoost = Common::GetPlayerBoost();
            float const previousBoost = *currentBoost;
            *currentBoost = max(0.0f, *currentBoost - (NextGenShadow::m_chaosBoostLevel + 1) * *dt);

            if (*currentBoost == 0.0f)
            {
                NextGenShadow::SetChaosBoostLevel(0, true);
            }
        }
    }

    originalNextGenShadow_CSonicUpdate(This, Edx, dt);
}

HOOK(bool, __fastcall, NextGenShadow_CSonicSpRenderableSsnUpdate, 0xE71510, uint32_t This, void* Edx, Hedgehog::Universe::SUpdateInfo const& a2)
{
    if (*pModernSonicContext && *(uint32_t*)This == 0x16DA834)
    {
        // jet effect
        hh::mr::CSingleElement* pModel = *(hh::mr::CSingleElement**)(This + 0xFC);
        if (NextGenShadow::ShouldPlayJetEffect() && Common::IsPlayerSuper())
        {
            if (!superJetRightFront)
            {
                NextGenShadow::SetJetEffectVisible(true, pModel, true);
            }
        }
        else if (superJetRightFront)
        {
            NextGenShadow::SetJetEffectVisible(false, pModel, true);
        }
    }

    return originalNextGenShadow_CSonicSpRenderableSsnUpdate(This, Edx, a2);
}

//---------------------------------------------------
// Chaos Attack/Chaos Snap
//---------------------------------------------------
HOOK(int, __stdcall, NextGenShadow_HomingUpdate, 0xE5FF10, CSonicContext* context)
{
    return originalNextGenShadow_HomingUpdate(context);
}

bool hasChaosSnapHiddenModel = false;
bool hasChaosSnapTeleported = false;
void PlayChaosSnap()
{
    auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
    NextGenShadow::m_chaosSnapActivated = true;

    // warp start pfx
    SharedPtrTypeless warpHandle;
    void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x10);
    Common::fCGlitterCreate(*PLAYER_CONTEXT, warpHandle, matrixNode, "ef_ch_sha_warp_s", 1);

    SharedPtrTypeless soundHandle;
    Common::SonicContextPlaySound(soundHandle, 80041038, 1);

    // hide all model
    NextGenShadow::SetChaosBoostModelVisible(true, true);
    hasChaosSnapHiddenModel = true;
    hasChaosSnapTeleported = false;
    if (context->m_SuperRenderableActorID)
    {
        context->m_pPlayer->SendMessageImm(context->m_SuperRenderableActorID, boost::make_shared<Sonic::Message::MsgSetVisible>(false));
    }

    // Stop in air
    Common::SetPlayerVelocity(Eigen::Vector3f::Zero());

    // Freeze camera
    CustomCamera::m_freezeCameraEnabled = true;

    // kill homing attack effects
    NextGenPhysics::killHomingAttackParticle();
    if (context->m_pSparkEffectManager)
    {
        FUNCTION_PTR(int, __stdcall, StopLocusEffect, 0xE8C940, Sonic::CSparkEffectManager * pSparkEffectManager, int sharedString);
        StopLocusEffect(context->m_pSparkEffectManager, 0x1E61C48);
    }
}

float chaosSnapHoldDuration = 0.0f;
HOOK(int, __fastcall, NextGenShadow_CSonicStateHomingAttackBegin, 0x1232040, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();

    // Chaos Snap
    chaosSnapHoldDuration = 0.0f;
    hasChaosSnapTeleported = false;
    if (NextGenShadow::CheckChaosSnapTarget())
    {
        // Skip initial velocity
        WRITE_JUMP(0x1232102, (void*)0x123211C);

        // Disable homing trail/sfx
        WRITE_JUMP(0x1232508, (void*)0x1232511);
        WRITE_MEMORY(0x12324AF, int, -1);

        // Change animation to ChaosAttackWait
        WRITE_MEMORY(0x1232056, char*, AnimationSetPatcher::ChaosAttackWait);

        PlayChaosSnap();
    }

    return originalNextGenShadow_CSonicStateHomingAttackBegin(This);
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateHomingAttackAdvance, 0x1231C60, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();

    Eigen::Vector3f velocity;
    Common::GetPlayerVelocity(velocity);

    // velocity control
    if (!context->m_HomingAttackTargetActorID)
    {
        if (velocity.norm() < 10.0f)
        {
            // hit a wall, unable to keep velocity
            StateManager::ChangeState(StateAction::Fall, *PLAYER_CONTEXT);
            return;
        }
        else
        {
            // No homing attack target, keep constant forward velocity
            velocity.y() = 0.0f;
            velocity = velocity.normalized() * context->m_spParameter->Get<float>(Sonic::Player::ePlayerSpeedParameter_HomingSpeedOfDummy);
            Common::SetPlayerVelocity(velocity);
        }
    }

    // hold duration for auto target
    Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
    if (padState->IsDown(Sonic::EKeyState::eKeyState_A))
    {
        chaosSnapHoldDuration += This->GetDeltaTime();
    }
    else
    {
        chaosSnapHoldDuration = 0.0f;
    }

    // Chaos Snap
    if (NextGenShadow::m_chaosBoostLevel > 0 && !hasChaosSnapTeleported)
    {
        // teleport after 0.1s
        if (hasChaosSnapHiddenModel && This->m_Time >= cShadow_chaosSnapWaitTime)
        {
            hasChaosSnapTeleported = true;

            hh::math::CVector targetPosition = hh::math::CVector::Zero();
            context->m_pPlayer->SendMessageImm(context->m_HomingAttackTargetActorID, boost::make_shared<Sonic::Message::MsgGetPosition>(&targetPosition));

            if (targetPosition.isZero())
            {
                // lost target
                StateManager::ChangeState(StateAction::Fall, *PLAYER_CONTEXT);
                return;
            }
            else
            {
                hh::math::CVector currentPosition = context->m_spMatrixNode->m_Transform.m_Position;
                currentPosition.y() -= 0.5f;

                // set velocity for MsgDamage reference
                hh::math::CVector direction = (context->m_HomingAttackPosition - currentPosition).normalized();
                Common::SetPlayerVelocity(direction * context->m_spParameter->Get<float>(Sonic::Player::ePlayerSpeedParameter_HomingSpeed));
                Common::SonicContextUpdateRotationToVelocity(context, &context->m_Velocity, true);

                // teleport
                hh::math::CVector targetPosition = context->m_HomingAttackPosition - direction * 1.0f;
                Common::SetPlayerPosition(targetPosition);
            }
        }
        else if (!hasChaosSnapHiddenModel && chaosSnapHoldDuration >= cShadow_chaosSnapStartHold)
        {
            // try to find target if not teleported
            originalNextGenShadow_HomingUpdate(context);

            // always allow Chaos Snap if holding A
            if (context->m_HomingAttackTargetActorID)
            {
                context->m_pPlayer->SendMessageImm(context->m_HomingAttackTargetActorID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&context->m_HomingAttackPosition));
                Common::SetPlayerVelocity(Eigen::Vector3f::Zero());
                Common::SonicContextHudHomingAttackOutro(context);

                // Send MsgStartHomingChase message to homing target actor
                context->m_pPlayer->SendMessage(context->m_HomingAttackTargetActorID, boost::make_shared<Sonic::Message::MsgStartHomingChase>());

                // play animations
                This->m_Time = 0.0f;
                PlayChaosSnap();
                Common::SonicContextChangeAnimation(AnimationSetPatcher::ChaosAttackWait);
            }
        }
    }

    originalNextGenShadow_CSonicStateHomingAttackAdvance(This);

    // keep velocity if hitting ground
    if (context->m_Grounded)
    {
        Common::SetPlayerVelocity(velocity);
    }
}

HOOK(int*, __fastcall, NextGenShadow_CSonicStateHomingAttackEnd, 0x1231F80, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();

    // resume Chaos Snap modification
    WRITE_MEMORY(0x1232102, uint8_t, 0x0F, 0x28, 0x44, 0x24, 0x5C);
    WRITE_MEMORY(0x1232508, uint8_t, 0x6A, 0x00, 0x8B, 0xC3, 0xE8);
    WRITE_MEMORY(0x12324AF, uint32_t, 2002029);
    WRITE_MEMORY(0x1232056, uint32_t, 0x15F84E8);

    // Unhide model
    if (hasChaosSnapHiddenModel)
    {
        // warp end pfx
        SharedPtrTypeless warpHandle;
        void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x10);
        Common::fCGlitterCreate(*PLAYER_CONTEXT, warpHandle, matrixNode, "ef_ch_sha_warp_e", 1);

        // Unhide model
        NextGenShadow::SetChaosBoostModelVisible(NextGenShadow::m_chaosBoostLevel > 0);
        hasChaosSnapHiddenModel = false;
        hasChaosSnapTeleported = true;
        if (context->m_SuperRenderableActorID)
        {
            context->m_pPlayer->SendMessageImm(context->m_SuperRenderableActorID, boost::make_shared<Sonic::Message::MsgSetVisible>(true));
        }
        
        // Unfreeze camera
        CustomCamera::m_freezeCameraEnabled = false;
    }

    // if Chaos Snap didn't hit any target, reset count
    if (!StateManager::isCurrentAction(StateAction::HomingAttackAfter))
    {
        NextGenShadow::m_chaosSnapActivated = false;
        NextGenShadow::m_chaosAttackCount = -1;
        hasChaosSnapTeleported = false;
    }

    return originalNextGenShadow_CSonicStateHomingAttackEnd(This);
}

void PlayNextChaosAttack()
{
    auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
    static SharedPtrTypeless soundHandleSfx;
    static SharedPtrTypeless soundHandleVfx;

    Common::SonicContextChangeAnimation(AnimationSetPatcher::ChaosAttack[NextGenShadow::m_chaosAttackCount]);
    Common::SonicContextPlaySound(soundHandleSfx, 80041028, 1);
    Common::SonicContextPlayVoice(soundHandleVfx, NextGenShadow::m_chaosAttackCount < 4 ? 3002032 : 3002031, 11 + NextGenShadow::m_chaosAttackCount);

    // kick effect (may not exist)
    static SharedPtrTypeless chaosAttackKickPfx[5];
    {
        auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("Root");
        Common::fCGlitterCreate(*PLAYER_CONTEXT, chaosAttackKickPfx[NextGenShadow::m_chaosAttackCount], &attachBone, ("ef_ch_sh_chaosattack0" + std::to_string(NextGenShadow::m_chaosAttackCount)).c_str(), 0);
    }

    // glow effect
    static SharedPtrTypeless chaosAttackPfx[5];
    {
        auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode
        (
            NextGenShadow::m_chaosAttackCount == 0 ? "Nose" : (NextGenShadow::m_chaosAttackCount == 2 ? "LeftToeBase" : "RightToeBase")
        );
        Common::fCGlitterCreate(*PLAYER_CONTEXT, chaosAttackPfx[NextGenShadow::m_chaosAttackCount], &attachBone, ("ef_ch_sh_chaosattack0" + std::to_string(NextGenShadow::m_chaosAttackCount) + "_kick").c_str(), 0);
    }
}

HOOK(int32_t*, __fastcall, NextGenShadow_CSonicStateHomingAttackAfterBegin, 0x1118300, hh::fnd::CStateMachineBase::CStateBase* This)
{
    // run the original function as some context members need to be set
    int32_t* result = originalNextGenShadow_CSonicStateHomingAttackAfterBegin(This);

    // handle Shadow stopping for chaos attack
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    if (!context->StateFlag(eStateFlag_EnableHomingAttackOnDiving) && !context->StateFlag(eStateFlag_KeepRunning) && Common::GetCurrentStageID() != SMT_bsl)
    {
        NextGenShadow::m_holdPosition = context->m_spMatrixNode->m_Transform.m_Position;
        NextGenShadow::m_chaosAttackBuffered = false;

        // Chaos Snap immediately goes to next attack
        if (NextGenShadow::m_chaosBoostLevel > 0 && hasChaosSnapTeleported)
        {
            // first attack
            if (NextGenShadow::m_chaosAttackCount < 0)
            {
                NextGenShadow::m_chaosAttackCount = 0;
            }

            PlayNextChaosAttack();
            NextGenShadow::m_chaosAttackCount++;
        }
        else if (NextGenShadow::m_chaosAttackCount < 0 || NextGenShadow::m_chaosBoostLevel == 0)
        {
            // normal chaos attack
            NextGenShadow::m_chaosAttackCount = 0;

            Common::SonicContextChangeAnimation(AnimationSetPatcher::ChaosAttackWait);
            Common::SetPlayerVelocity(Eigen::Vector3f::Zero());
        }

        // No damage during Chaos Snap
        if (NextGenShadow::m_chaosBoostLevel > 0 && !NextGenShadow::m_chaosSnapNoDamage)
        {
            NextGenShadow::m_chaosSnapNoDamage = true;
            context->StateFlag(eStateFlag_NoDamage)++;
        }
    }

    hasChaosSnapTeleported = false;
    return result;
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateHomingAttackAfterAdvance, 0x1118600, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    if (NextGenShadow::m_chaosAttackCount >= 0)
    {
        Common::SetPlayerVelocity(Eigen::Vector3f::Zero());
        Common::SetPlayerPosition(NextGenShadow::m_holdPosition);

        alignas(16) MsgGetAnimationInfo message {};
        Common::SonicContextGetAnimationInfo(message);

        Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
        bool const isPressedA = padState->IsTapped(Sonic::EKeyState::eKeyState_A);
        bool const isHoldingA = padState->IsDown(Sonic::EKeyState::eKeyState_A);

        static SharedPtrTypeless soundHandleSfx;
        static SharedPtrTypeless soundHandleVfx;

        if (message.IsAnimation(AnimationSetPatcher::ChaosAttackWait))
        {
            bool const isChaosSnapSearch = isHoldingA && NextGenShadow::m_chaosBoostLevel > 0 && NextGenShadow::m_chaosAttackCount > 0 && NextGenShadow::m_chaosAttackCount < 5 && !NextGenShadow::m_chaosAttackBuffered;
            bool const nextAttackNotBuffered = NextGenShadow::m_chaosAttackCount > 0 && !NextGenShadow::m_chaosAttackBuffered;

            if (isChaosSnapSearch)
            {
                originalNextGenShadow_HomingUpdate(context);
            }
            else if (This->m_Time >= cShadow_chaosAttackWaitTime || nextAttackNotBuffered)
            {
                // timeout, resume original homing attack after
                This->m_Time = 0.0f;
                NextGenShadow::m_chaosSnapActivated = false;
                NextGenShadow::m_chaosAttackCount = -1;

                // apply up velocity
                Eigen::Vector3f velocity(0, 0, 0);
                velocity.y() = context->m_spParameter->Get<float>(Sonic::Player::ePlayerSpeedParameter_HomingAttackAfterUpVelocity)
                             * context->m_spParameter->Get<float>(Sonic::Player::ePlayerSpeedParameter_AttackAfterImpluseVelocityCoeff);
                Common::SetPlayerVelocity(velocity);

                // play jump sfx, voice as sound to prevent replacing previous voice
                Common::SonicContextPlaySound(soundHandleSfx, 2002027, 1);
                Common::SonicContextPlaySound(soundHandleVfx, 3002000, 0);

                // play random homing attack after animation
                Common::SonicContextChangeAnimation((const char*)*((uint32_t*)0x1E75E18 + rand() % 6));

                // make sure it's in air again
                Common::SonicContextSetInAirData(context);
                context->StateFlag(eStateFlag_EnableHomingAttack) = true;
                context->StateFlag(eStateFlag_EnableAirOnceAction) = true;

                Common::SonicContextHudHomingAttackClear(context);
                context->m_HomingAttackTargetActorID = 0;

                // resume damage
                if (NextGenShadow::m_chaosSnapNoDamage)
                {
                    NextGenShadow::m_chaosSnapNoDamage = false;
                    context->StateFlag(eStateFlag_NoDamage)--;
                }
                return;
            }

            // Next attack
            bool useNextAttack = false;
            useNextAttack |= NextGenShadow::m_chaosAttackCount == 0 && isPressedA; // first attack must press A to start
            useNextAttack |= NextGenShadow::m_chaosAttackCount > 0 && NextGenShadow::m_chaosAttackBuffered; // next attacks can be buffered
            useNextAttack |= NextGenShadow::m_chaosAttackCount == 0 && NextGenShadow::m_chaosBoostLevel > 0 && isHoldingA; // Chaos Snap immediately goes to next attack

            if (isChaosSnapSearch && NextGenShadow::CheckChaosSnapTarget())
            {
                // Chaos Snap immediately goes to next target if found
                StateManager::ChangeState(StateAction::HomingAttack, *PLAYER_CONTEXT);
            }
            else if (useNextAttack)
            {
                PlayNextChaosAttack();
                NextGenShadow::m_chaosAttackCount++;
                NextGenShadow::m_chaosAttackBuffered = false;

                if (context->m_HomingAttackTargetActorID)
                {
                    hh::math::CVector targetPosition = hh::math::CVector::Identity();
                    context->m_pPlayer->SendMessageImm(context->m_HomingAttackTargetActorID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&targetPosition));
                
                    hh::math::CVector applyVelocity = targetPosition - context->m_spMatrixNode->m_Transform.m_Position;
                    applyVelocity.y() = max(0.0f, applyVelocity.y());
                    applyVelocity = applyVelocity.normalized() * context->m_spParameter->Get<float>(Sonic::Player::ePlayerSpeedParameter_HomingSpeed);

                    // Apply damage to lock-on target
                    context->m_pPlayer->SendMessage
                    (
                        context->m_HomingAttackTargetActorID, boost::make_shared<Sonic::Message::MsgDamage>
                        (
                            *(uint32_t*)0x1E61B90, // DamageTypeSonicSquatKick
                            targetPosition,
                            applyVelocity
                        )
                    );

                    Common::SonicContextHudHomingAttackOutro(context);
                    context->m_HomingAttackTargetActorID = 0;
                }
            }
        }
        else
        {
            // buffer next attack during attack animation
            This->m_Time = 0.0f;
            if (NextGenShadow::m_chaosAttackCount < 5 && !NextGenShadow::m_chaosAttackBuffered)
            {
                NextGenShadow::m_chaosAttackBuffered = isPressedA;
            }

            if (NextGenShadow::m_chaosAttackCount < 5)
            {
                originalNextGenShadow_HomingUpdate(context);
            }
        }
    }
    else
    {
        originalNextGenShadow_CSonicStateHomingAttackAfterAdvance(This);
    }
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateHomingAttackAfterEnd, 0x11182F0)
{
    auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();

    // resume ground detection
    WRITE_MEMORY(0xE63A31, uint8_t, 0x74);

    // resume disable lock-on cursor
    WRITE_MEMORY(0xDEBAA0, uint8_t, 0x75);

    // resume damage
    if (NextGenShadow::m_chaosSnapNoDamage)
    {
        NextGenShadow::m_chaosSnapNoDamage = false;
        context->StateFlag(eStateFlag_NoDamage)--;
    }

    // Chaos Snap retains previous count
    if (NextGenShadow::m_chaosBoostLevel == 0 || !StateManager::isCurrentAction(StateAction::HomingAttack))
    {
        NextGenShadow::m_chaosSnapActivated = false;
        NextGenShadow::m_chaosAttackCount = -1;
    }
    NextGenShadow::m_chaosAttackBuffered = false;
}

//---------------------------------------------------
// Chaos Boost
//---------------------------------------------------
HOOK(int, __fastcall, NextGenShadow_MsgRestartStage, 0xE76810, Sonic::Player::CPlayer* player, void* Edx, void* message)
{
    int result = originalNextGenShadow_MsgRestartStage(player, Edx, message);

    // HUD reset is handled by 06 HUD
    NextGenShadow::SetChaosBoostLevel(0, false);
    NextGenShadow::SetJetEffectVisible(false, nullptr, false);
    NextGenShadow::SetJetEffectVisible(false, nullptr, true);

    // Re-enable auto run actions (squat kick)
    NextGenShadow::m_enableAutoRunAction = true;

    return result;
}

SharedPtrTypeless chaosBoostSpine;
SharedPtrTypeless chaosBoostTopHair;
SharedPtrTypeless chaosBoostLeftHand;
SharedPtrTypeless chaosBoostRightHand;
SharedPtrTypeless chaosBoostLeftFoot;
SharedPtrTypeless chaosBoostRightFoot;
void NextGenShadow::SetChaosBoostModelVisible(bool visible, bool allInvisible)
{
    if (!*pModernSonicContext) return;

    auto const& model = Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_spCharacterModel;
    model->m_spModel->m_NodeGroupModels[0]->m_Visible = !visible && !allInvisible;
    model->m_spModel->m_NodeGroupModels[1]->m_Visible = visible && !allInvisible;

    auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
    context->StateFlag(eStateFlag_DisableGroundSmoke) = allInvisible;

    if (visible && !allInvisible && !Common::IsPlayerSuper())
    {
        if (!chaosBoostSpine)
        {
            {
                auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("Spine");
                Common::fCGlitterCreate(*PLAYER_CONTEXT, chaosBoostSpine, &attachBone, "ef_ch_sh_yh1_forceaura2", 1);
            }
            {
                auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("TopHair");
                Common::fCGlitterCreate(*PLAYER_CONTEXT, chaosBoostTopHair, &attachBone, "ef_ch_sh_yh1_forceaura1", 1);
            }
            {
                auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("LeftHand");
                Common::fCGlitterCreate(*PLAYER_CONTEXT, chaosBoostLeftHand, &attachBone, "ef_ch_sh_yh1_forceaura1", 1);
            }
            {
                auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("RightHand");
                Common::fCGlitterCreate(*PLAYER_CONTEXT, chaosBoostRightHand, &attachBone, "ef_ch_sh_yh1_forceaura1", 1);
            }
            {
                auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("LeftFoot");
                Common::fCGlitterCreate(*PLAYER_CONTEXT, chaosBoostLeftFoot, &attachBone, "ef_ch_sh_yh1_forceaura1", 1);
            }
            {
                auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("RightFoot");
                Common::fCGlitterCreate(*PLAYER_CONTEXT, chaosBoostRightFoot, &attachBone, "ef_ch_sh_yh1_forceaura1", 1);
            }
        }
    }
    else if (chaosBoostSpine)
    {
        Common::fCGlitterEnd(*PLAYER_CONTEXT, chaosBoostSpine, allInvisible);
        Common::fCGlitterEnd(*PLAYER_CONTEXT, chaosBoostTopHair, allInvisible);
        Common::fCGlitterEnd(*PLAYER_CONTEXT, chaosBoostLeftHand, allInvisible);
        Common::fCGlitterEnd(*PLAYER_CONTEXT, chaosBoostRightHand, allInvisible);
        Common::fCGlitterEnd(*PLAYER_CONTEXT, chaosBoostLeftFoot, allInvisible);
        Common::fCGlitterEnd(*PLAYER_CONTEXT, chaosBoostRightFoot, allInvisible);

        chaosBoostSpine = nullptr;
        chaosBoostTopHair = nullptr;
        chaosBoostLeftHand = nullptr;
        chaosBoostRightHand = nullptr;
        chaosBoostLeftFoot = nullptr;
        chaosBoostRightFoot = nullptr;
    }
}

void NextGenShadow::SetChaosBoostLevel(uint8_t level, bool notifyHUD)
{
    if (!Configuration::m_characterMoveset && level > 0) return;

    m_chaosBoostLevel = level;
    m_chaosMaturity = 0.0f; // change level always resets maturity
    SetChaosBoostModelVisible(level > 0);

    if (notifyHUD)
    {
        S06HUD_API::SetShadowChaosLevel(level, 0.0f);
    }
}

bool NextGenShadow::CheckChaosBoost()
{
    Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
    if (!padState->IsTapped(Sonic::EKeyState::eKeyState_B))
    {
        return false;
    }

    if (*Common::GetPlayerBoost() == 0.0f || m_chaosMaturity < 100.0f)
    {
        return false;
    }

    auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
    if (context->StateFlag(eStateFlag_KeepRunning))
    {
        return false;
    }

    if (m_chaosBoostLevel == 3 && !isChaosControl)
    {
        Common::SonicContextAddPlugin("TimeBreak");
        return true;
    }

    if (m_chaosBoostLevel < 3)
    {
        m_overrideType = OverrideType::SH_ChaosBoost;
        StateManager::ChangeState(StateAction::TrickAttack, *PLAYER_CONTEXT);

        return true;
    }

    return false;
}

bool NextGenShadow::CheckChaosControl()
{
    Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
    if (!padState->IsTapped(Sonic::EKeyState::eKeyState_B))
    {
        return false;
    }

    if (*Common::GetPlayerBoost() == 0.0f || m_chaosMaturity < 100.0f)
    {
        return false;
    }

    if (m_chaosBoostLevel == 3 && !isChaosControl)
    {
        Common::SonicContextAddPlugin("TimeBreak");
        return true;
    }

    return false;
}

bool NextGenShadow::CheckChaosSnapTarget()
{
    if (NextGenShadow::m_chaosBoostLevel == 0)
    {
        return false;
    }

    auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
    if (!context->m_HomingAttackTargetActorID)
    {
        return false;
    }

    if (NextGenShadow::m_chaosSnapActivated)
    {
        return true;
    }

    uint32_t enemyType = 0u;
    context->m_pPlayer->SendMessageImm(context->m_HomingAttackTargetActorID, boost::make_shared<Sonic::Message::MsgGetEnemyType>(&enemyType));
    if (enemyType > 0)
    {
        return true;
    }
    
    return false;
}

void NextGenShadow::AddChaosMaturity(float amount)
{
    if (!Configuration::m_characterMoveset) return;
    if (Configuration::m_model != Configuration::ModelType::Shadow) return;

    float maturity = m_chaosMaturity;
    m_chaosMaturity = max(0.0f, min(100.0f, maturity + amount));
    S06HUD_API::SetShadowChaosLevel(m_chaosBoostLevel, m_chaosMaturity);

    if (m_chaosMaturity == 100.0f && maturity < 100.0f)
    {
        static SharedPtrTypeless soundHandle;
        Common::SonicContextPlaySound(soundHandle, 80041040, 1);
    }
}

//-------------------------------------------------------
// Chaos Spear/Chaos Blast/Chaos Boost
//-------------------------------------------------------
class CObjChaosSpear : public Sonic::CGameObject3D
{
private:
    uint32_t m_TargetID;

    float m_Speed;
    Hedgehog::Math::CVector m_Position;
    Hedgehog::Math::CVector m_Velocity;
    float m_LifeTime;
    bool m_IsDamage;
    bool m_IsSuper;

    SharedPtrTypeless spearHandle;
    SharedPtrTypeless spearTailHandle;
    SharedPtrTypeless spearVanishHandle;
    boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;

public:
    CObjChaosSpear
    (
        uint32_t _TargetID,
        Hedgehog::Math::CVector const& _Position,
        Hedgehog::Math::CVector const& _StartDir,
        float _SpeedAdd,
        bool _IsDamage
    )
        : m_TargetID(_TargetID)
        , m_Speed(cShadow_chaosSpearSpeed + _SpeedAdd)
        , m_Position(_Position)
        , m_Velocity(_StartDir.normalized() * m_Speed)
        , m_LifeTime(0.0f)
        , m_IsDamage(_IsDamage)
        , m_IsSuper(false)
    {

    }

    ~CObjChaosSpear()
    {
    }

    void AddCallback
    (
        const Hedgehog::Base::THolder<Sonic::CWorld>& worldHolder,
        Sonic::CGameDocument* pGameDocument,
        const boost::shared_ptr<Hedgehog::Database::CDatabase>& spDatabase
    ) override
    {
        Sonic::CGameObject3D::AddCallback(worldHolder, pGameDocument, spDatabase);

        Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
        pGameDocument->AddUpdateUnit("0", this);

        // Set initial transform
        UpdateTransform();

        // play pfx
        if (Common::IsPlayerSuper())
        {
            m_IsSuper = true;
            m_IsDamage = true;
            Common::fCGlitterCreate(*PLAYER_CONTEXT, spearHandle, &m_spMatrixNodeTransform, "ef_bo_ssh_yh2_spear", 1);
            Common::fCGlitterCreate(*PLAYER_CONTEXT, spearTailHandle, &m_spMatrixNodeTransform, "ef_bo_ssh_yh2_spear_tail", 1);
        }
        else if (m_IsDamage)
        {
            Common::fCGlitterCreate(*PLAYER_CONTEXT, spearHandle, &m_spMatrixNodeTransform, "ef_bo_sha_yh2_lance", 1);
            Common::fCGlitterCreate(*PLAYER_CONTEXT, spearTailHandle, &m_spMatrixNodeTransform, "ef_bo_sha_yh2_lance_tail", 1);
        }
        else
        {
            Common::fCGlitterCreate(*PLAYER_CONTEXT, spearHandle, &m_spMatrixNodeTransform, "ef_bo_sha_yh2_spear", 1);
            Common::fCGlitterCreate(*PLAYER_CONTEXT, spearTailHandle, &m_spMatrixNodeTransform, "ef_bo_sha_yh2_spear_tail", 1);
        }
    
        // set up collision with enemy
        m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
        m_spNodeEventCollision->m_Transform.SetPosition(hh::math::CVector(0.0f, 0.0f, 0.0f));
        m_spNodeEventCollision->NotifyChanged();
        m_spNodeEventCollision->SetParent(m_spMatrixNodeTransform.get());

        hk2010_2_0::hkpCapsuleShape* eventTriggerDamage = new hk2010_2_0::hkpCapsuleShape(hh::math::CVector(0.f, 0.f, 0.f), hh::math::CVector(0.f, 0.f, -1.f), 0.2f);
        AddEventCollision("Damage", eventTriggerDamage, *reinterpret_cast<int*>(0x1E0AF84), true, m_spNodeEventCollision); // SpikeAttack

        hk2010_2_0::hkpCapsuleShape* eventTriggerTerrain = new hk2010_2_0::hkpCapsuleShape(hh::math::CVector(0.f, 0.f, -0.3f), hh::math::CVector(0.f, 0.f, -1.f), 0.05f);
        AddEventCollision("Terrain", eventTriggerTerrain, *reinterpret_cast<int*>(0x1E0AFAC), true, m_spNodeEventCollision); // BasicAndTerrainCheck
    }

    bool ProcessMessage
    (
        Hedgehog::Universe::Message& message,
        bool flag
    ) override
    {
        if (flag)
        {
            if (std::strstr(message.GetType(), "MsgRestartStage") != nullptr
             || std::strstr(message.GetType(), "MsgStageClear") != nullptr)
            {
                Kill();
                return true;
            }

            if (std::strstr(message.GetType(), "MsgHitEventCollision") != nullptr)
            {
                HitTarget(message.m_SenderActorID);
                return true;
            }
        }

        return Sonic::CGameObject3D::ProcessMessage(message, flag);
    }

    void UpdateParallel
    (
        const Hedgehog::Universe::SUpdateInfo& updateInfo
    ) override
    {
        hh::math::CVector newDirection = hh::math::CVector::Zero();
        if (m_TargetID)
        {
            // get current target position
            hh::math::CVector targetPosition = hh::math::CVector::Zero();
            SendMessageImm(m_TargetID, boost::make_shared<Sonic::Message::MsgGetPosition>(&targetPosition));
            if (!targetPosition.isZero())
            {
                SendMessageImm(m_TargetID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&targetPosition));
                newDirection = (targetPosition - m_Position).normalized();

                // getting close enough to target
                if ((targetPosition - m_Position).norm() <= m_Speed * updateInfo.DeltaTime)
                {
                    HitTarget(m_TargetID);
                    return;
                }
            }
        }

        if (!newDirection.isZero())
        {
            hh::math::CVector oldDirection = m_Velocity.normalized();
            float dot = oldDirection.dot(newDirection);
            Common::ClampFloat(dot, -1.0f, 1.0f);

            float const angle = acos(dot);
            float const maxAngle = updateInfo.DeltaTime * (m_IsSuper ? cShadow_chaosSpearSuperTurnRate : cShadow_chaosSpearTurnRate);
            if (angle > maxAngle)
            {
                hh::math::CVector cross = oldDirection.cross(newDirection).normalized();
                Eigen::AngleAxisf rot(maxAngle, cross);
                newDirection = rot * oldDirection;
            }

            m_Velocity = newDirection * m_Speed;
        }

        m_Position += m_Velocity * updateInfo.DeltaTime;
        UpdateTransform();

        // time out
        m_LifeTime += updateInfo.DeltaTime;
        if (m_LifeTime >= cShadow_chaosSpearLife)
        {
            Kill();
        }
    }

    void UpdateTransform()
    {
        // Rotate spear to correct rotation
        Hedgehog::Math::CVector const dir = m_Velocity.normalized();
        Hedgehog::Math::CVector dirXZ = dir; dirXZ.y() = 0.0f;
        Hedgehog::Math::CQuaternion rotYaw = Hedgehog::Math::CQuaternion::FromTwoVectors(Hedgehog::Math::CVector::UnitZ(), dirXZ.head<3>());
        Hedgehog::Math::CQuaternion rotPitch = Hedgehog::Math::CQuaternion::FromTwoVectors(dirXZ.head<3>(), dir.head<3>());

        m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(rotPitch * rotYaw, m_Position);
        m_spMatrixNodeTransform->NotifyChanged();
    }

    void HitTarget(uint32_t actorID)
    {
        auto* senderMessageActor = m_pMessageManager->GetMessageActor(actorID);
        uint32_t senderActor = (uint32_t)senderMessageActor - 0x28;
        bool cannotDamage = false;
        if (*(uint32_t*)senderActor == 0x16F70BC) // CEnemySpinner
        {
            cannotDamage = *(bool*)(senderActor + 0x239);
        }

        Common::fCGlitterCreate(*PLAYER_CONTEXT, spearVanishHandle, &m_spMatrixNodeTransform, m_IsDamage ? "ef_bo_sha_yh2_lance_vanish" : "ef_bo_sha_yh2_spear_vanish", 1);
        if (m_IsDamage && !cannotDamage)
        {
            SendMessage
            (
                actorID, boost::make_shared<Sonic::Message::MsgDamage>
                (
                    *(uint32_t*)0x1E0BE30, // DamageID_SonicLight
                    m_Position,
                    hh::math::CVector::Identity()
                )
            );
        }

        SendMessage
        (
            actorID, boost::make_shared<Sonic::Message::MsgNotifyShockWave>
            (
                cShadow_chaosSpearShockDuration
            )
        );

        Kill();
    }

    void Kill()
    {
        Common::fCGlitterEnd(*PLAYER_CONTEXT, spearHandle, true);
        Common::fCGlitterEnd(*PLAYER_CONTEXT, spearTailHandle, false);

        SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
    }
};

class CObjChaosLockonCursor : public Sonic::CGameObject
{
    Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectLockonCursor;
    boost::shared_ptr<Sonic::CGameObjectCSD> m_spLockonCursor;
    Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneLockonCursor;

    Eigen::Vector4f m_pos;
    bool m_isOutro;

    std::mutex m_mutex;

public:
    CObjChaosLockonCursor(Eigen::Vector4f inPos)
        : m_pos(inPos)
        , m_isOutro(false)
    {
    }

    ~CObjChaosLockonCursor()
    {
        if (m_spLockonCursor)
        {
            m_spLockonCursor->SendMessage(m_spLockonCursor->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
            m_spLockonCursor = nullptr;
        }

        Chao::CSD::CProject::DestroyScene(m_projectLockonCursor.Get(), m_sceneLockonCursor);
        m_projectLockonCursor = nullptr;
    }

    void AddCallback
    (
        const Hedgehog::Base::THolder<Sonic::CWorld>& worldHolder,
        Sonic::CGameDocument* pGameDocument,
        const boost::shared_ptr<Hedgehog::Database::CDatabase>& spDatabase
    ) override
    {
        Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
        pGameDocument->AddUpdateUnit("1", this);

        Sonic::CCsdDatabaseWrapper wrapper(m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());
        auto spCsdProject = wrapper.GetCsdProject("ui_lockon_cursor");
        Common::CopyCCsdProject(spCsdProject.get(), m_projectLockonCursor);

        if (m_projectLockonCursor)
        {
            m_sceneLockonCursor = m_projectLockonCursor->CreateScene("cursor");
            PlayIntro();

            if (!m_spLockonCursor)
            {
                m_spLockonCursor = boost::make_shared<Sonic::CGameObjectCSD>(m_projectLockonCursor, 0.5f, "HUD", false);
                Sonic::CGameDocument::GetInstance()->AddGameObject(m_spLockonCursor, "main", this);
            }
        }
    }

    bool ProcessMessage
    (
        Hedgehog::Universe::Message& message,
        bool flag
    ) override
    {
        if (flag)
        {
            if (std::strstr(message.GetType(), "MsgRestartStage") != nullptr
                || std::strstr(message.GetType(), "MsgStageClear") != nullptr)
            {
                Kill();
                return true;
            }
        }

        return Sonic::CGameObject::ProcessMessage(message, flag);
    }

    void UpdateParallel
    (
        const Hedgehog::Universe::SUpdateInfo& updateInfo
    ) override
    {
        std::lock_guard<std::mutex> guard(m_mutex);

        // loop animation
        if (m_sceneLockonCursor && m_sceneLockonCursor->m_MotionDisableFlag)
        {
            if (m_isOutro)
            {
                Kill();
            }
            else
            {
                PlayMotion("Usual_Anim", true);
            }
        }

        // Handle position
        if (m_sceneLockonCursor)
        {
            Eigen::Vector4f screenPosition;
            Common::fGetScreenPosition(m_pos, screenPosition);
            m_sceneLockonCursor->SetHideFlag(screenPosition.z() < 0.0f);
            m_sceneLockonCursor->SetPosition(screenPosition.x(), screenPosition.y());
        }
    }

    void PlayIntro()
    {
        PlayMotion("Intro_Anim", false);
    }

    void SetPos(Eigen::Vector4f inPos)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_pos = inPos;

        if (m_isOutro)
        {
            PlayIntro();
        }
        m_isOutro = false;
    }

    void SetOutro()
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        PlayMotion("Outro_Anim", false);
        m_isOutro = true;
    }

    void PlayMotion(char const* motion, bool loop = false)
    {
        if (!m_sceneLockonCursor) return;
        m_sceneLockonCursor->SetHideFlag(false);
        m_sceneLockonCursor->SetMotion(motion);
        m_sceneLockonCursor->m_MotionDisableFlag = false;
        m_sceneLockonCursor->m_MotionFrame = 0.0f;
        m_sceneLockonCursor->m_MotionSpeed = 1.0f;
        m_sceneLockonCursor->m_MotionRepeatType = loop ? Chao::CSD::eMotionRepeatType_Loop : Chao::CSD::eMotionRepeatType_PlayOnce;
        m_sceneLockonCursor->Update();
    }

    void Kill()
    {
        SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
    }
};

class CObjChaosBlast : public Sonic::CGameObject3D
{
private:
    float m_Radius;
    float m_LifeTime;
    Hedgehog::Math::CVector m_Position;
    boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;

public:
    CObjChaosBlast
    (
        float _Radius,
        Hedgehog::Math::CVector const& _Position
    )
        : m_Radius(_Radius)
        , m_Position(_Position)
        , m_LifeTime(0.0f)
    {

    }

    ~CObjChaosBlast()
    {
    }

    void AddCallback
    (
        const Hedgehog::Base::THolder<Sonic::CWorld>& worldHolder,
        Sonic::CGameDocument* pGameDocument,
        const boost::shared_ptr<Hedgehog::Database::CDatabase>& spDatabase
    ) override
    {
        Sonic::CGameObject3D::AddCallback(worldHolder, pGameDocument, spDatabase);

        Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
        pGameDocument->AddUpdateUnit("0", this);

        // set initial position
        m_spMatrixNodeTransform->m_Transform.SetPosition(m_Position);
        m_spMatrixNodeTransform->NotifyChanged();

        // set up collision with enemy
        m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
        m_spNodeEventCollision->m_Transform.SetPosition(hh::math::CVector(0.0f, 0.0f, 0.0f));
        m_spNodeEventCollision->NotifyChanged();
        m_spNodeEventCollision->SetParent(m_spMatrixNodeTransform.get());

        hk2010_2_0::hkpSphereShape* shapeEventTrigger1 = new hk2010_2_0::hkpSphereShape(m_Radius);
        AddEventCollision("Damage", shapeEventTrigger1, *reinterpret_cast<int*>(0x1E0AF84), true, m_spNodeEventCollision);
    }

    bool ProcessMessage
    (
        Hedgehog::Universe::Message& message,
        bool flag
    ) override
    {
        if (flag)
        {
            if (std::strstr(message.GetType(), "MsgRestartStage") != nullptr
             || std::strstr(message.GetType(), "MsgStageClear") != nullptr)
            {
                Kill();
                return true;
            }

            if (std::strstr(message.GetType(), "MsgHitEventCollision") != nullptr)
            {
                uint32_t enemyType = 0u;
                SendMessageImm(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgGetEnemyType>(&enemyType));

                auto* senderMessageActor = m_pMessageManager->GetMessageActor(message.m_SenderActorID);
                uint32_t* senderActor = (uint32_t*)((uint32_t)senderMessageActor - 0x28);
                bool isObjectPhysics = *(uint32_t*)senderMessageActor == 0x16CF58C;

                hh::math::CVector targetPosition = hh::math::CVector::Identity();
                if (enemyType > 0)
                {
                    // try to get center position from lock-on for enemy
                    SendMessageImm(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&targetPosition));
                }
                else if (isObjectPhysics)
                {
                    // get dynamic position for object physics
                    Common::fObjectPhysicsDynamicPosition(senderActor, targetPosition);
                }
                else
                {
                    SendMessageImm(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgGetPosition>(&targetPosition));
                }

                // apply damage
                if (!targetPosition.isIdentity())
                {
                    SendMessage(message.m_SenderActorID, boost::make_shared<Sonic::Message::MsgDamage>
                        (
                            *(uint32_t*)0x1E0BE34, // DamageID_NoAttack
                            m_Position, 
                            (targetPosition - m_Position) * (enemyType > 0 ? cShadow_chaosBlastVelocityEnemy : cShadow_chaosBlastVelocityObjPhy)
                        ), (targetPosition - m_Position).norm() * 0.2f / m_Radius); // delay base on distance
                }

                return true;
            }
        }

        return Sonic::CGameObject3D::ProcessMessage(message, flag);
    }

    void UpdateParallel
    (
        const Hedgehog::Universe::SUpdateInfo& updateInfo
    ) override
    {
        // time out
        m_LifeTime += updateInfo.DeltaTime;
        if (m_LifeTime >= cShadow_chaosBlastDuration)
        {
            Kill();
        }
    }

    void Kill()
    {
        SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
    }
};

bool NextGenShadow::AirActionCheck()
{
    auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
    if (context->StateFlag(eStateFlag_KeepRunning))
    {
        return false;
    }

    Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();

    // Chaos Spear
    if (padState->IsTapped(Sonic::EKeyState::eKeyState_X))
    {
        if (NextGenPhysics::checkUseLightSpeedDash())
        {
            return true;
        }

        m_overrideType = OverrideType::SH_SpearWait;
        StateManager::ChangeState(StateAction::TrickAttack, *PLAYER_CONTEXT);
        return true;
    }

    // Chaos Boost
    if (CheckChaosBoost())
    {
        return true;
    }

    // Chaos Blast
    if (m_chaosBoostLevel == 3)
    {
        if ((padState->IsTapped(Sonic::EKeyState::eKeyState_LeftBumper) && padState->IsDown(Sonic::EKeyState::eKeyState_RightBumper))
         || (padState->IsTapped(Sonic::EKeyState::eKeyState_RightBumper) && padState->IsDown(Sonic::EKeyState::eKeyState_LeftBumper)))
        {
            m_overrideType = OverrideType::SH_ChaosBlastWait;
            StateManager::ChangeState(StateAction::TrickAttack, *PLAYER_CONTEXT);
            return true;
        }
    }

    return false;
}

void NextGenShadow::AddTargetData(uint32_t actorID, float dist, uint32_t priority)
{
    if (m_targetData.size() < (Common::IsPlayerSuper() ? cShadow_chaosSpearSuperMaxCount : cShadow_chaosSpearMaxCount))
    {
        m_targetData.push_back(TargetData{ actorID, dist, priority });
    }
}

HOOK(bool, __fastcall, NextGenShadow_AirAction, 0xDFFE30, Sonic::Player::CPlayerSpeedContext* context, void* Edx, int a2)
{
    // Handle air action that ignores various flag checks
    Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();

    // Chaos Spear
    if (Configuration::Shadow::m_chaosSpearMomentum && !context->StateFlag(eStateFlag_EnableGravityControl) && padState->IsTapped(Sonic::EKeyState::eKeyState_X))
    {
        if (NextGenPhysics::checkUseLightSpeedDash())
        {
            return true;
        }

        NextGenShadow::m_overrideType = NextGenShadow::OverrideType::SH_SpearWait;
        StateManager::ChangeState(StateAction::TrickAttack, *PLAYER_CONTEXT);
        return true;
    }

    return originalNextGenShadow_AirAction(context, Edx, a2);
}

void __declspec(naked) NextGenShadow_AirActionAfterFlag()
{
    static uint32_t successAddress = 0xDFFEDC;
    static uint32_t returnAddress = 0xDFFEAD;
    static uint32_t fnButtonPress = 0xD97E00;
    __asm
    {
        call    NextGenShadow::AirActionCheck
        test    al, al
        jnz     jump

        // original function
        mov     eax, [esi + 11Ch]
        jmp     [returnAddress]

        jump:
        jmp     [successAddress]
    }
}

void __declspec(naked) NextGenShadow_GetAllHomingTargets()
{
    static uint32_t returnAddress = 0xE7484F;
    __asm
    {
        fstp    [esp + 14h]
        
        mov     esi, [esp + 18h]
        mov     ecx, [esp + 14h]
        mov     eax, [esp + 34h]

        push    esi
        push    ecx
        push    eax
        call    NextGenShadow::AddTargetData
        add     esp, 0xC

        // original function
        fld     [esp + 14h]
        jmp     [returnAddress]
    }
}

static SharedPtrTypeless soundHandle_TrickAttack;
static SharedPtrTypeless pfxHandle_TrickAttack;
std::map<uint32_t, boost::shared_ptr<CObjChaosLockonCursor>> m_spLockonCursors;
HOOK(int, __fastcall, NextGenShadow_CSonicStateTrickAttackBegin, 0x1202270, hh::fnd::CStateMachineBase::CStateBase* This)
{
    static SharedPtrTypeless voiceHandle;
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();

    switch (NextGenShadow::m_overrideType)
    {
    case NextGenShadow::OverrideType::SH_SpearWait:
    {
        NextGenShadow::m_holdPosition = context->m_spMatrixNode->m_Transform.m_Position;
        Common::SonicContextChangeAnimation(Common::IsPlayerSuper() ? AnimationSetPatcher::SpearSuperWait : AnimationSetPatcher::SpearWait);

        // sound, voice, pfx
        Common::SonicContextPlaySound(soundHandle_TrickAttack, 80041031, 1);
        Common::SonicContextPlayVoice(voiceHandle, 3002030, 10);

        auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("RightHandMiddle1");
        Common::fCGlitterCreate(*PLAYER_CONTEXT, pfxHandle_TrickAttack, &attachBone, NextGenShadow::m_chaosBoostLevel >= 2 ? "ef_bo_sha_yh2_lance_attack" : "ef_bo_sha_yh2_spear_charge", 1);
        
        if (Common::IsPlayerSuper())
        {
            // change homing distance
            NextGenPhysics::setHomingCollisionDistance(cShadow_chaosSpearSuperLockDist);
        }
        break;
    }
    case NextGenShadow::OverrideType::SH_ChaosBoost:
    {
        Common::SonicContextHudHomingAttackClear(context);
        context->m_HomingAttackTargetActorID = 0;

        Common::SonicContextChangeAnimation(AnimationSetPatcher::ChaosBoost);
        context->StateFlag(eStateFlag_OutOfControl)++;
        context->StateFlag(eStateFlag_NoDamage)++;
        context->m_GravityTimer = -100000000.0f;
        
        Common::SonicContextPlaySound(soundHandle_TrickAttack, 80041035, 1);
        void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x30);
        Common::fCGlitterCreate(*PLAYER_CONTEXT, pfxHandle_TrickAttack, matrixNode, "ef_ch_sh_chaosboost", 1);

        NextGenShadow::SetChaosBoostLevel(NextGenShadow::m_chaosBoostLevel + 1, true);
        break;
    }
    case NextGenShadow::OverrideType::SH_ChaosBlastWait:
    {
        Common::SonicContextHudHomingAttackClear(context);
        context->m_HomingAttackTargetActorID = 0;

        Common::SonicContextChangeAnimation(AnimationSetPatcher::ChaosBlastWait);
        context->StateFlag(eStateFlag_OutOfControl)++;
        context->StateFlag(eStateFlag_NoDamage)++;
        context->m_GravityTimer = -100000000.0f;

        Common::SonicContextPlayVoice(voiceHandle, 3002034, 20);
        void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x30);
        Common::fCGlitterCreate(*PLAYER_CONTEXT, pfxHandle_TrickAttack, matrixNode, "ef_ch_sh_chaosblast_charge", 1);
        
        CustomCamera::m_chaosBlastCameraEnabled = true;
        isChaosControl = false;
        break;
    }
    }

    return 0;
}

HOOK(void*, __fastcall, NextGenShadow_CSonicStateTrickAttackAdvance, 0x1201B30, hh::fnd::CStateMachineBase::CStateBase* This)
{
    static SharedPtrTypeless soundHandle;
    static SharedPtrTypeless voiceHandle;
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();

    switch (NextGenShadow::m_overrideType)
    {
    case NextGenShadow::OverrideType::SH_SpearWait:
    {
        if (context->m_Grounded)
        {
            StateManager::ChangeState(StateAction::LandJumpShort, *PLAYER_CONTEXT);
            return nullptr;
        }

        if (!Configuration::Shadow::m_chaosSpearMomentum)
        {
            Common::SetPlayerVelocity(Eigen::Vector3f::Zero());
            Common::SetPlayerPosition(NextGenShadow::m_holdPosition);
        }

        // Ignore priority comparison
        WRITE_NOP(0xE74593, 6);
        WRITE_JUMP(0xE74847, NextGenShadow_GetAllHomingTargets);

        NextGenShadow::m_targetData.clear();
        originalNextGenShadow_HomingUpdate(context);

        // Resume
        WRITE_MEMORY(0xE74593, uint8_t, 0x0F, 0x8C, 0xE5, 0x02, 0x00, 0x00);
        WRITE_MEMORY(0xE74847, uint8_t, 0xD9, 0x5C, 0x24, 0x14, 0xD9);

        // handle multiple lock-on targets and HUD
        uint8_t const spearMaxCount = Common::IsPlayerSuper() ? cShadow_chaosSpearSuperMaxCount : cShadow_chaosSpearMaxCount;
        float const accumulateStartTime = cShadow_chaosSpearAccumulate - cShadow_chaosSpearAddTime * (spearMaxCount - 1);
        if (This->m_Time >= accumulateStartTime)
        {
            uint32_t currentTargetCount = min(spearMaxCount, 1 + uint32_t((This->m_Time - accumulateStartTime) / cShadow_chaosSpearAddTime));

            // main lock-on target may or may not in the list
            bool foundMainTarget = false;
            for (NextGenShadow::TargetData const& data : NextGenShadow::m_targetData)
            {
                if (context->m_HomingAttackTargetActorID == data.m_actorID)
                {
                    foundMainTarget = true;
                    break;
                }
            }
            if (context->m_HomingAttackTargetActorID && !foundMainTarget)
            {
                NextGenShadow::m_targetData.push_back(NextGenShadow::TargetData{ context->m_HomingAttackTargetActorID, 0.0f, 100 });
            }

            // super will only fire one spear straight if no target found
            if (Common::IsPlayerSuper() && NextGenShadow::m_targetData.empty())
            {
                currentTargetCount = 1u;
            }

            // sort by priority then distance
            std::sort(NextGenShadow::m_targetData.begin(), NextGenShadow::m_targetData.end(),
                [](NextGenShadow::TargetData const& a, NextGenShadow::TargetData const& b)
                {
                    return a.m_priority > b.m_priority || (a.m_priority == b.m_priority && a.m_dist < b.m_dist);
                }
            );

            // record new targets (except for main target)
            std::set<uint32_t> lockonActors;
            for (NextGenShadow::TargetData const& data : NextGenShadow::m_targetData)
            {
                if (data.m_actorID != context->m_HomingAttackTargetActorID && lockonActors.size() < currentTargetCount - 1)
                {
                    lockonActors.insert(data.m_actorID);
                }
            }

            // clean unused targets
            if (currentTargetCount < NextGenShadow::m_targetData.size())
            {
                NextGenShadow::m_targetData.resize(currentTargetCount);
            }

            if (Common::IsPlayerSuper())
            {
                // super can fire at the same target
                int const repeatTargetSize = NextGenShadow::m_targetData.size();
                int i = 0;
                while (NextGenShadow::m_targetData.size() < currentTargetCount)
                {
                    NextGenShadow::m_targetData.push_back(NextGenShadow::m_targetData.at(i++));
                    if (i >= repeatTargetSize)
                    {
                        i = 0;
                    }
                }
            }
            else if (NextGenShadow::m_targetData.empty())
            {
                // push dummy targets
                NextGenShadow::m_targetData.resize(currentTargetCount);
            }

            if (!Configuration::m_noCursor)
            {
                // update existing cursors
                for (auto iter = m_spLockonCursors.begin(); iter != m_spLockonCursors.end();)
                {
                    uint32_t actorID = iter->first;
                    auto& spLockonCursor = iter->second;

                    // check if actor still locked
                    if (lockonActors.count(actorID))
                    {
                        hh::math::CVector targetPosition = hh::math::CVector::Zero();
                        context->m_pPlayer->SendMessageImm(actorID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&targetPosition));

                        Eigen::Vector4f pos4f(targetPosition.x(), targetPosition.y(), targetPosition.z(), 1.0f);
                        spLockonCursor->SetPos(pos4f);

                        lockonActors.erase(actorID);
                        iter++;
                    }
                    else
                    {
                        spLockonCursor->Kill();
                        iter = m_spLockonCursors.erase(iter);
                    }
                }

                // create cursors for new targets
                if (!lockonActors.empty())
                {
                    // play single lock-on sound
                    static SharedPtrTypeless soundHandle;
                    Common::SonicContextPlaySound(soundHandle, 4002012, 0);

                    for (uint32_t actorID : lockonActors)
                    {
                        hh::math::CVector targetPosition = hh::math::CVector::Zero();
                        context->m_pPlayer->SendMessageImm(actorID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&targetPosition));

                        Eigen::Vector4f pos4f(targetPosition.x(), targetPosition.y(), targetPosition.z(), 1.0f);
                        m_spLockonCursors[actorID] = boost::make_shared<CObjChaosLockonCursor>(pos4f);

                        context->m_pPlayer->m_pMember->m_pGameDocument->AddGameObject(m_spLockonCursors[actorID]);
                    }
                }
            }
        }

        Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
        if (!padState->IsDown(Sonic::EKeyState::eKeyState_X))
        {
            NextGenShadow::m_overrideType = NextGenShadow::OverrideType::SH_SpearShot;
            Common::SonicContextChangeAnimation(Common::IsPlayerSuper() ? AnimationSetPatcher::SpearSuperShot : AnimationSetPatcher::SpearShot);

            if (This->m_Time < accumulateStartTime)
            {
                // m_HomingAttackTargetActorID can be 0
                NextGenShadow::m_targetData.clear();
                NextGenShadow::m_targetData.push_back(NextGenShadow::TargetData{ context->m_HomingAttackTargetActorID, 0.0f, 0 });
            }

            int noTargetCount = 0;
            for (uint8_t i = 0; i < NextGenShadow::m_targetData.size() && i < spearMaxCount; i++)
            {
                NextGenShadow::TargetData const& data = NextGenShadow::m_targetData[i];

                // get initial target position
                hh::math::CVector targetPosition = hh::math::CVector::Zero();
                if (data.m_actorID)
                {
                    // get initial target position
                    context->m_pPlayer->SendMessageImm(data.m_actorID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&targetPosition));
                
                    // clear main target HUD
                    if (data.m_actorID == context->m_HomingAttackTargetActorID)
                    {
                        Common::SonicContextHudHomingAttackOutro(context);
                        context->m_HomingAttackTargetActorID = 0;
                    }
                }

                // spawn chaos spear
                Hedgehog::Math::CVector position = context->m_spMatrixNode->m_Transform.m_Position + Hedgehog::Math::CVector::UnitY() * 0.7f;
                Hedgehog::Math::CVector targetDir = targetPosition - position;
                Hedgehog::Math::CVector playerDir = context->m_HorizontalRotation * Hedgehog::Math::CVector::UnitZ();
                if (Common::IsPlayerSuper())
                {
                    if (NextGenShadow::m_targetData.size() <= 1u)
                    {
                        // single or no target, fire straight
                        targetDir = playerDir;
                    }
                    else
                    {
                        float angle = (360.0f * (i + 1)) * DEG_TO_RAD / (float)NextGenShadow::m_targetData.size();
                        if (NextGenShadow::m_targetData.size() == 2)
                        {
                            // if only 2 shoot sideways
                            angle -= 90 * DEG_TO_RAD;
                        }
                        else
                        {
                            // shot more from top for odd numbers
                            angle -= 180 * DEG_TO_RAD;
                        }

                        // pitch up, then rotate
                        Hedgehog::Math::CVector playerRight = context->m_HorizontalRotation * Hedgehog::Math::CVector::UnitX();
                        targetDir = Eigen::AngleAxisf(angle, playerDir) * Eigen::AngleAxisf(cShadow_chaosSpearUpAngle, -playerRight) * context->m_HorizontalRotation * Hedgehog::Math::CVector::UnitZ();
                    }
                }
                else if (targetPosition.isZero() || targetDir.dot(playerDir) <= 0.0f)
                {
                    // spread out targets
                    noTargetCount++;
                    float yawAngle = 0.0f;
                    switch (noTargetCount)
                    {
                    case 2: yawAngle = 10.0f * DEG_TO_RAD; break;
                    case 3: yawAngle = -10.0f * DEG_TO_RAD; break;
                    case 4: yawAngle = 20.0f * DEG_TO_RAD; break;
                    case 5: yawAngle = -20.0f * DEG_TO_RAD; break;
                    }

                    // make shots symmetric
                    if (NextGenShadow::m_targetData.size() % 2 == 0)
                    {
                        yawAngle -= 5.0f *DEG_TO_RAD;
                    }

                    // pitch down if no target
                    Hedgehog::Math::CVector playerRight = context->m_HorizontalRotation * Hedgehog::Math::CVector::UnitX();
                    targetDir = Eigen::AngleAxisf(yawAngle, hh::math::CVector::UnitY()) * Eigen::AngleAxisf(cShadow_chaosSpearDownAngle, playerRight) * context->m_HorizontalRotation * Hedgehog::Math::CVector::UnitZ();
                }

                float const horizontalSpeed = context->m_HorizontalVelocity.norm();
                context->m_pPlayer->m_pMember->m_pGameDocument->AddGameObject(boost::make_shared<CObjChaosSpear>(data.m_actorID, position, targetDir, horizontalSpeed, NextGenShadow::m_chaosBoostLevel >= 2));
            }

            // clear lock-on cursors
            for (auto& iter : m_spLockonCursors)
            {
                auto& spLockonCursor = iter.second;
                spLockonCursor->SetOutro();
            }
            m_spLockonCursors.clear();

            // only play one sound, not per spear
            soundHandle_TrickAttack.reset();
            Common::fCGlitterEnd(*PLAYER_CONTEXT, pfxHandle_TrickAttack, true);
            Common::SonicContextPlaySound(soundHandle, Common::IsPlayerSuper() && NextGenShadow::m_targetData.size() > 1 ? 80041044 : 80041032, 1);
            This->m_Time = 0.0f;

            // Set OutOfControl
            if (!Configuration::Shadow::m_chaosSpearMomentum)
            {
                FUNCTION_PTR(int, __stdcall, SetOutOfControl, 0xE5AC00, CSonicContext * context, float duration);
                SetOutOfControl(*pModernSonicContext, (Common::IsPlayerSuper() ? cShadow_chaosSpearSuperStiffening : cShadow_chaosSpearStiffening));
                context->m_GravityTimer = -100000000.0f;
            }
            break;
        }

        break;
    }
    case NextGenShadow::OverrideType::SH_SpearShot:
    {
        if (context->m_Grounded)
        {
            StateManager::ChangeState(StateAction::LandJumpShort, *PLAYER_CONTEXT);
            return nullptr;
        }

        if (This->m_Time >= (Common::IsPlayerSuper() ? cShadow_chaosSpearSuperStiffening : cShadow_chaosSpearStiffening))
        {
            if (!Configuration::Shadow::m_chaosSpearMomentum && !Common::IsPlayerSuper())
            {
                Common::SonicContextPlaySound(soundHandle, 2002027, 1);
                Common::SonicContextPlaySound(voiceHandle, 3002000, 0);

                Common::CStateSetStringBool(This, "ContinuesAnimation", true);
                Common::SonicContextChangeAnimation("HomingAttackAfter1");
            }

            StateManager::ChangeState(StateAction::Fall, *PLAYER_CONTEXT);
            break;
        }

        break;
    }
    case NextGenShadow::OverrideType::SH_ChaosBoost:
    {
        Common::SetPlayerVelocity(Eigen::Vector3f::Zero());
        if (This->m_Time >= cShadow_chaosBoostStartTime)
        {
            StateManager::ChangeState(context->m_Grounded ? StateAction::LandJumpShort : StateAction::Fall, *PLAYER_CONTEXT);
            break;
        }

        break;
    }
    case NextGenShadow::OverrideType::SH_ChaosBlastWait:
    {
        Common::SetPlayerVelocity(Eigen::Vector3f::Zero());
        if (This->m_Time >= cShadow_chaosBlastWaitTime)
        {
            This->m_Time = 0.0f;
            NextGenShadow::m_overrideType = NextGenShadow::OverrideType::SH_ChaosBlast;

            Common::SonicContextChangeAnimation(AnimationSetPatcher::ChaosBlast);
            Common::SonicContextPlaySound(soundHandle_TrickAttack, 80041037, 1);

            void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x30);
            Common::fCGlitterCreate(*PLAYER_CONTEXT, pfxHandle_TrickAttack, matrixNode, "ef_ch_sh_chaosblast", 1);

            boost::shared_ptr<Sonic::CCamera> const& spCamera = context->m_pPlayer->m_pMember->m_pWorld->GetCamera();
            hh::fnd::CMessageActor* cameraMessageActor = (hh::fnd::CMessageActor*)((uint32_t)spCamera.get() + 0x28);
            context->m_pPlayer->SendMessage
            (
                cameraMessageActor->m_ActorID, boost::make_shared<Sonic::Message::MsgShakeCamera>
                (
                    0.5f, 1.0f, 10, hh::math::CVector::UnitY(), 0.05f
                )
            );

            auto spChaosBlast = boost::make_shared<CObjChaosBlast>(cShadow_chaosBlastRadius, context->m_spMatrixNode->m_Transform.m_Position);
            Sonic::CGameDocument::GetInstance()->AddGameObject(spChaosBlast);
            break;
        }

        break;
    }
    case NextGenShadow::OverrideType::SH_ChaosBlast:
    {
        Common::SetPlayerVelocity(Eigen::Vector3f::Zero());
        if (This->m_Time >= cShadow_chaosBlastAttackTime)
        {
            StateManager::ChangeState(context->m_Grounded ? StateAction::LandJumpShort : StateAction::Fall, *PLAYER_CONTEXT);
            break;
        }

        break;
    }
    }

    return nullptr;
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateTrickAttackEnd, 0x1202110, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();

    switch (NextGenShadow::m_overrideType)
    {
    case NextGenShadow::OverrideType::SH_SpearWait:
    case NextGenShadow::OverrideType::SH_SpearShot:
    {
        context->m_GravityTimer = 1000.0f;

        soundHandle_TrickAttack.reset();
        Common::fCGlitterEnd(*PLAYER_CONTEXT, pfxHandle_TrickAttack, true);
        Common::SonicContextHudHomingAttackClear(context);

        // clear cursors if not already
        for (auto& iter : m_spLockonCursors)
        {
            auto& spLockonCursor = iter.second;
            spLockonCursor->Kill();
        }
        m_spLockonCursors.clear();

        // reset homing distance
        NextGenPhysics::setHomingCollisionDistance();
        break;
    }
    case NextGenShadow::OverrideType::SH_ChaosBoost:
    case NextGenShadow::OverrideType::SH_ChaosBlastWait:
    case NextGenShadow::OverrideType::SH_ChaosBlast:
    {
        context->StateFlag(eStateFlag_OutOfControl)--;
        context->StateFlag(eStateFlag_NoDamage)--;
        context->m_GravityTimer = 1000.0f;
        break;
    }
    }

    NextGenShadow::m_overrideType = NextGenShadow::OverrideType::SH_None;
}

//---------------------------------------------------
// Chaos Control
//---------------------------------------------------
float timeBreakDeltaTime_Shadow;
HOOK(void, __fastcall, NextGenShadow_CPlayerSpeedStatePluginTimeBreakBegin, 0x111AF10, int This)
{
    isChaosControl = true;

    static SharedPtrTypeless voiceHandle;
    Common::SonicContextPlayVoice(voiceHandle, rand() % 2 ? 3002033 : 3002037, 20);

    static SharedPtrTypeless pfxHandle;
    void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x30);
    Common::fCGlitterCreate(*PLAYER_CONTEXT, pfxHandle, matrixNode, "ef_ch_sh_chaoscontrol", 1);

    static SharedPtrTypeless soundHandle;
    Common::SonicContextPlaySound(soundHandle, 80041043, 1);

    timeBreakDeltaTime_Shadow = 0.0f;
    *(bool*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x19D }) = true;
    *(float*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x1A4 }) = cShadow_chaosControlSlowScale;

    // don't use state time to quit
    WRITE_JUMP(0x111B03D, (void*)0x111B0F3);

    originalNextGenShadow_CPlayerSpeedStatePluginTimeBreakBegin(This);
}

HOOK(void, __fastcall, NextGenShadow_CPlayerSpeedStatePluginTimeBreakAdvance, 0x111B030, Sonic::Player::CPlayerSpeedContext::CStateSpeedBase* This)
{
    auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();

    // scale time
    float* timeScale = (float*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x1A4 });
    if (This->m_Time >= cShadow_chaosControlSlowTime)
    {
        *timeScale = cShadow_chaosControlSlowScale;
    }
    else
    {
        float const scaleProp = This->m_Time / cShadow_chaosControlSlowTime;
        *timeScale = 1.0f - (1.0f - cShadow_chaosControlSlowScale) * scaleProp;
    }

    // drain gauge
    NextGenShadow::AddChaosMaturity((timeBreakDeltaTime_Shadow - This->m_Time) * 100.0f / cShadow_chaosControlDuration);
    timeBreakDeltaTime_Shadow = This->m_Time;

    // no more gauge, quit plugin
    if (!isChaosControl || context->StateFlag(eStateFlag_Dead) || NextGenShadow::m_chaosMaturity == 0.0f)
    {
        WRITE_JUMP(0x111B03D, (void*)0x111B05E);
    }

    originalNextGenShadow_CPlayerSpeedStatePluginTimeBreakAdvance(This);
}

HOOK(void, __fastcall, NextGenShadow_CPlayerSpeedStatePluginTimeBreakEnd, 0x111AE00, int This)
{
    isChaosControl = false;

    *(bool*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x19D }) = false;
    *(float*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x1A4 }) = 1.0f;

    originalNextGenShadow_CPlayerSpeedStatePluginTimeBreakEnd(This);
}

HOOK(int*, __fastcall, NextGenShadow_CParticleManagerAdvance, 0xE8F000, void* This, void* Edx, float* dt)
{
    return originalNextGenShadow_CParticleManagerAdvance(This, Edx, dt);
}

HOOK(void, __fastcall, NextGenShadow_GlitterCHandleAdvance, 0x6BC8B0, uint32_t This, void* Edx, float* dt, void* database)
{
    bool const slowEnabled = *(bool*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x19D });
    if (slowEnabled)
    {
        std::string const particleName = *(char const**)(This + 0xA0);
        static std::set<std::string> c_noSlowParticles =
        {
            "ef_bo_sha",
            "ef_ch_sh",
            "ef_ch_sn",
            "ef_ch_sps",
        };

        for (std::string const& name : c_noSlowParticles)
        {
            if (particleName.find(name) != std::string::npos)
            {
                float const timeScale = *(float*)Common::GetMultiLevelAddress(0x1E0BE5C, { 0x8, 0x1A4 });
                float dtTemp[2];
                dtTemp[0] = dt[0] / timeScale;
                dtTemp[1] = dtTemp[0] * 60.0f;
                originalNextGenShadow_GlitterCHandleAdvance(This, Edx, dtTemp, database);
                return;
            }
        }
    }

    originalNextGenShadow_GlitterCHandleAdvance(This, Edx, dt, database);
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateDivingDiveAdvance, 0x124AEE0, int This)
{
    NextGenShadow::CheckChaosControl();
    originalNextGenShadow_CSonicStateDivingDiveAdvance(This);
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateDivingFloatAdvance, 0x11BCFB0, int This)
{
    NextGenShadow::CheckChaosControl();
    originalNextGenShadow_CSonicStateDivingFloatAdvance(This);
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateFloatingBoostAdvance, 0x123FCD0, int This)
{
    NextGenShadow::CheckChaosControl();
    originalNextGenShadow_CSonicStateFloatingBoostAdvance(This);
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateGrindBegin, 0xDF2890, void* This)
{
    NextGenShadow::CheckChaosControl();
    originalNextGenShadow_CSonicStateGrindBegin(This);
}

HOOK(int, __fastcall, NextGenShadow_MsgPlayerGoal, 0xE6C2C0, Sonic::Player::CPlayer* player, void* Edx, void* message)
{
    isChaosControl = false;
    return originalNextGenShadow_MsgPlayerGoal(player, Edx, message);
}

HOOK(void, __fastcall, NextGenShadow_MsgStartHangOn, 0xE6C0D0, Sonic::Player::CPlayer* player, void* Edx, uint32_t* message)
{
    uint32_t type = message[4];
    isChaosControl = false;
    originalNextGenShadow_MsgStartHangOn(player, Edx, message);
}

HOOK(int, __fastcall, NextGenShadow_CObjCannonMsgNotifyObjectEvent, 0x468EB0, void* This, void* Edx, uint32_t* message)
{
    isChaosControl = false;
    return originalNextGenShadow_CObjCannonMsgNotifyObjectEvent(This, Edx, message);
}

HOOK(int, __fastcall, NextGenShadow_CObjSelectCanonMsgNotifyObjectEvent, 0x100F8A0, void* This, void* Edx, uint32_t* message)
{
    isChaosControl = false;
    return originalNextGenShadow_CObjSelectCanonMsgNotifyObjectEvent(This, Edx, message);
}

HOOK(int, __fastcall, NextGenShadow_CObjJumpSelectorMsgNotifyObjectEvent, 0x101E3E0, void* This, void* Edx, uint32_t* message)
{
    isChaosControl = false;
    return originalNextGenShadow_CObjJumpSelectorMsgNotifyObjectEvent(This, Edx, message);
}

HOOK(int, __fastcall, NextGenShadow_CObjStopPeopleMsgNotifyObjectEvent, 0xFDDFB0, void* This, void* Edx, uint32_t* message)
{
    isChaosControl = false;
    return originalNextGenShadow_CObjStopPeopleMsgNotifyObjectEvent(This, Edx, message);
}

HOOK(void, __fastcall, NextGenShadow_CRivalShadowMsgDamage, 0xCB4620, void* This, void* Edx, hh::fnd::Message& message)
{
    auto const* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
    if (context && message.m_SenderActorID == context->m_pPlayer->m_ActorID && context->StateFlag(eStateFlag_AutoBoost))
    {
        isChaosControl = false;
    }
    originalNextGenShadow_CRivalShadowMsgDamage(This, Edx, message);
}

//---------------------------------------------------
// X Button Action
//---------------------------------------------------
HOOK(bool, __stdcall, NextGenShadow_BActionHandler, 0xDFF660, CSonicContext* context, bool buttonHoldCheck)
{
    // Handle X button action
    bool result = originalNextGenShadow_BActionHandler(context, buttonHoldCheck);
    if (result || Common::IsPlayerControlLocked())
    {
        NextGenShadow::m_xHeldTimer = 0.0f;
        return result;
    }

    return NextGenShadow::bActionHandlerImpl();
}

HOOK(void, __fastcall, NextGenShadow_MsgNotifyObjectEvent, 0xEA4F50, void* This, void* Edx, uint32_t a2)
{
    uint32_t* pEvent = (uint32_t*)(a2 + 16);
    uint32_t* pObject = (uint32_t*)This;

    // Only use [1001-2000] events
    if (*pEvent > 1000 && *pEvent <= 2000)
    {
        switch (*pEvent)
        {
            // Disable action in auto run (mach speed)
        case 1001:
        {
            NextGenShadow::m_enableAutoRunAction = false;
            break;
        }
        }
    }

    originalNextGenShadow_MsgNotifyObjectEvent(This, Edx, a2);
}

bool NextGenShadow::bActionHandlerImpl()
{
    Eigen::Vector3f playerVelocity;
    if (!Common::GetPlayerVelocity(playerVelocity))
    {
        return false;
    }

    if (CheckChaosControl())
    {
        return true;
    }

    CSonicStateFlags* flags = Common::GetSonicStateFlags();
    if (flags->KeepRunning && !m_enableAutoRunAction)
    {
        // Cannot use any action during auto run
        return false;
    }

    // activate chaos boost on ground
    if (CheckChaosBoost())
    {
        return true;
    }

    bool moving = playerVelocity.norm() > 0.2f;
    bool canUseSpindash = !moving || (!Configuration::Shadow::m_antiGravity && !flags->KeepRunning);

    bool bDown, bPressed, bReleased;
    NextGenPhysics::getActionButtonStates(bDown, bPressed, bReleased);
    if (bDown)
    {
        // Standing still and held B for a while (Spin Dash)
        if (canUseSpindash && m_xHeldTimer > cShadow_squatKickPressMaxTime && !NextGenShadow::m_isBrakeFlip)
        {
            StateManager::ChangeState(StateAction::Squat, *PLAYER_CONTEXT);
            m_xHeldTimer = 0.0f;
            return true;
        }

        // Remember how long we held X
        // NOTE: Apparently this code always runs at 60fps
        // If 30fps this will run twice per frame!
        m_xHeldTimer += 1.0f / 60.0f;
    }
    else
    {
        if (bReleased && !flags->OnWater)
        {
            if (m_xHeldTimer <= cShadow_squatKickPressMaxTime)
            {
                // Release X without holding it for too long (Squat Kick)
                StateManager::ChangeState(StateAction::SquatKick, *PLAYER_CONTEXT);
                m_xHeldTimer = 0.0f;
                return true;
            }
            else if (moving && !flags->KeepRunning && m_xHeldTimer > cShadow_squatKickPressMaxTime)
            {
                if (Configuration::Shadow::m_antiGravity)
                {
                    // Moving and released X (Anti-Gravity)
                    StateManager::ChangeState(StateAction::Sliding, *PLAYER_CONTEXT);
                    m_xHeldTimer = 0.0f;
                    return true;
                }
            }
        }

        m_xHeldTimer = 0.0f;
    }

    return false;
}

//---------------------------------------------------
// CSonicStateSlidingEnd
//---------------------------------------------------
HOOK(int, __fastcall, NextGenShadow_CSonicStateSlidingEndBegin, 0x1230F80, void* This)
{
    // For Sonic only, do a flip if no stick input
    Eigen::Vector3f inputDirection;
    if (NextGenShadow::m_tripleKickCount >= 0 || (Common::GetWorldInputDirection(inputDirection) && inputDirection.isZero()))
    {
        // Do brake flip animation
        NextGenShadow::m_tripleKickCount = -1;
        NextGenShadow::m_isBrakeFlip = true;
        WRITE_MEMORY(0x1230F88, char*, AnimationSetPatcher::BrakeFlip);
    }
    else
    {
        // Original SlidingToWalk animation
        WRITE_MEMORY(0x1230F88, uint32_t, 0x15E6A00);
    }

    return originalNextGenShadow_CSonicStateSlidingEndBegin(This);
}

HOOK(int*, __fastcall, NextGenShadow_CSonicStateSlidingEndAdvance, 0x1230EE0, void* This)
{
    int* result = originalNextGenShadow_CSonicStateSlidingEndAdvance(This);
    if (NextGenShadow::m_isBrakeFlip)
    {
        if (!slidingEndWasSliding_Shadow)
        {
            // Only detect B-action during the flip, not normal SlidingEnd
            NextGenShadow::bActionHandlerImpl();
        }

        // Enforce brake flip rotation
        alignas(16) float dir[4] = { NextGenShadow::m_brakeFlipDir.x(), NextGenShadow::m_brakeFlipDir.y(), NextGenShadow::m_brakeFlipDir.z(), 0 };
        NextGenPhysics::applyCSonicRotationAdvance(This, dir);

        // Move during flip
        Eigen::Vector3f inputDirection;
        if (Common::GetWorldInputDirection(inputDirection) && !inputDirection.isZero())
        {
            StateManager::ChangeState(StateAction::Walk, *PLAYER_CONTEXT);
        }
    }
    return result;
}

HOOK(int*, __fastcall, NextGenShadow_CSonicStateSlidingEndEnd, 0x1230E60, void* This)
{
    NextGenShadow::m_tripleKickCount = -1;
    NextGenShadow::m_isBrakeFlip = false;
    return originalNextGenShadow_CSonicStateSlidingEndEnd(This);
}

//---------------------------------------------------
// CSonicStateSquatKick
//---------------------------------------------------
static SharedPtrTypeless tripleKickPfxR;
static SharedPtrTypeless tripleKickPfxL;
void NextGenShadow::NextTripleKick(Sonic::Player::CPlayerSpeedContext* context)
{
    // next kick
    m_tripleKickCount++;
    m_tripleKickBuffered = false;
    m_tripleKickShockWaveSpawned = false;
    Common::SonicContextChangeAnimation(AnimationSetPatcher::SpinAttack[m_tripleKickCount]);

    static SharedPtrTypeless soundHandle;
    static SharedPtrTypeless voiceHandle;
    Common::SonicContextPlayVoice(voiceHandle, m_tripleKickCount == 2 ? 3002031 : 3002032, 10 + m_tripleKickCount);
    Common::SonicContextPlaySound(soundHandle, 80041029, 1);

    static SharedPtrTypeless tripleKickPfx[3];
    auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("Root");
    Common::fCGlitterCreate(*PLAYER_CONTEXT, tripleKickPfx[m_tripleKickCount], &attachBone, ("ef_ch_sh_spinattack0" + std::to_string(m_tripleKickCount)).c_str(), 0);
}

HOOK(int*, __fastcall, NextGenShadow_CSonicStateSquatKickBegin, 0x12526D0, hh::fnd::CStateMachineBase::CStateBase* This)
{
    // Don't allow direction change for squat kick
    WRITE_MEMORY(0x11D943D, uint8_t, 0xEB);

    // Get initial brake flip direction
    Eigen::Vector3f playerPosition;
    Eigen::Quaternionf playerRotation;
    if (Common::GetPlayerTransform(playerPosition, playerRotation))
    {
        NextGenShadow::m_brakeFlipDir = playerRotation * Eigen::Vector3f::UnitZ();
    }

    // Get current speed so we can keep it
    Eigen::Vector3f playerVelocity;
    if (Common::GetPlayerVelocity(playerVelocity))
    {
        playerVelocity.y() = 0.0f;
        NextGenShadow::m_squatKickSpeed = playerVelocity.norm();
    }

    NextGenShadow::m_tripleKickCount = -1;
    if (NextGenShadow::m_squatKickSpeed == 0.0f)
    {
        // start triple kick
        auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
        NextGenShadow::NextTripleKick(context);
        Common::SonicContextSetCollision(SonicCollision::TypeSonicSquatKick, true);

        /*{
            auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("RightToeBase");
            Common::fCGlitterCreate(*PLAYER_CONTEXT, tripleKickPfxR, &attachBone, "ef_ch_sh_spinattack_kickR", 0);
        }
        {
            auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("LeftToeBase");
            Common::fCGlitterCreate(*PLAYER_CONTEXT, tripleKickPfxL, &attachBone, "ef_ch_sh_spinattack_kickL", 0);
        }*/

        // Stop moving
        Common::GetSonicStateFlags()->OutOfControl = true;
        return nullptr;
    }
    else
    {
        // Play squat kick sfx
        static SharedPtrTypeless soundHandle;
        Common::SonicContextPlaySound(soundHandle, 80041030, 1);

        NextGenShadow::m_isSquatKick = true;
        return originalNextGenShadow_CSonicStateSquatKickBegin(This);
    }
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateSquatKickAdvance, 0x1252810, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    if (NextGenShadow::m_tripleKickCount >= 0)
    {
        // triple kick
        if (Common::IsPlayerAnimationFinished(context->m_pPlayer))
        {
            if (!context->m_Grounded)
            {
                StateManager::ChangeState(StateAction::Fall, *PLAYER_CONTEXT);
            }
            else if (NextGenShadow::m_tripleKickCount == 2)
            {
                // finished
                StateManager::ChangeState(StateAction::Stand, *PLAYER_CONTEXT);
            }
            else if (NextGenShadow::m_tripleKickBuffered)
            {
                NextGenShadow::NextTripleKick(context);
                This->m_Time = 0.0f;
            }
            else
            {
                // no input for next attack
                StateManager::ChangeState(StateAction::SlidingEnd, *PLAYER_CONTEXT);
            }
        }
        else
        {
            if (!NextGenShadow::m_tripleKickShockWaveSpawned && This->m_Time >= cShadow_tripleKickShockWaveSpawn)
            {
                NextGenShadow::m_tripleKickShockWaveSpawned = true;

                Hedgehog::Math::CVector pos = context->m_spMatrixNode->m_Transform.m_Position;
                pos.y() += 0.1f;
                Common::CreatePlayerSupportShockWave(pos, cShadow_tripleKickShockWaveHeight, cShadow_tripleKickShockWaveRadius + NextGenShadow::m_tripleKickCount, cShadow_tripleKickShockWaveDuration);
            }

            if (!NextGenShadow::m_tripleKickBuffered)
            {
                Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
                NextGenShadow::m_tripleKickBuffered = padState->IsTapped(Sonic::EKeyState::eKeyState_X);
            }
        }
    }
    else
    {
        // sweep kick
        originalNextGenShadow_CSonicStateSquatKickAdvance(This);
    }

    // Lock squat kick's rotation if not moving
    if (NextGenShadow::m_squatKickSpeed == 0.0f)
    {
        alignas(16) float dir[4] = { NextGenShadow::m_brakeFlipDir.x(), NextGenShadow::m_brakeFlipDir.y(), NextGenShadow::m_brakeFlipDir.z(), 0 };
        NextGenPhysics::applyCSonicRotationAdvance(This, dir);
    }
}

bool __fastcall NextGenShadow_CSonicStateSquatKickAdvanceTransitionOutImpl(char const* name)
{
    if (strcmp(name, "Stand") == 0 || strcmp(name, "Walk") == 0)
    {
        CSonicStateFlags* flags = Common::GetSonicStateFlags();
        if (!flags->KeepRunning)
        {
            Eigen::Vector3f inputDirection;
            if (Common::GetWorldInputDirection(inputDirection) && inputDirection.isZero())
            {
                // Stops Sonic completely if no stick input
                slidingEndWasSliding_Shadow = NextGenShadow::m_isSliding;
                StateManager::ChangeState(StateAction::SlidingEnd, *PLAYER_CONTEXT);
                return true;
            }
        }
    }

    return false;
}

void __declspec(naked) NextGenShadow_CSonicStateSquatKickAdvanceTransitionOut()
{
    static uint32_t returnAddress = 0x1252924;
    static uint32_t sub_E4FF30 = 0xE4FF30;
    __asm
    {
        push    eax
        push    ecx

        mov     ecx, [eax]
        call    NextGenShadow_CSonicStateSquatKickAdvanceTransitionOutImpl
        mov     bl, al

        pop     ecx
        pop     eax

        test    bl, bl
        jnz     jump
        call    [sub_E4FF30]

        jump:
        jmp     [returnAddress]
    }
}

HOOK(int*, __fastcall, NextGenShadow_CSonicStateSquatKickEnd, 0x12527B0, void* This)
{
    // Unlock direction change for sliding/spindash
    WRITE_MEMORY(0x11D943D, uint8_t, 0x74);
    
    if (NextGenShadow::m_tripleKickCount >= 0)
    {
        Common::SonicContextSetCollision(SonicCollision::TypeSonicSquatKick, false);
        Common::GetSonicStateFlags()->OutOfControl = false;
        if (NextGenShadow::m_tripleKickCount == 2)
        {
            NextGenShadow::m_tripleKickCount = -1;
        }

        //Common::fCGlitterEnd(*PLAYER_CONTEXT, tripleKickPfxR, true);
        //Common::fCGlitterEnd(*PLAYER_CONTEXT, tripleKickPfxL, true);

        // m_tripleKickCount gets reset in SlidingEnd state
        return nullptr;
    }
    else
    {
        NextGenShadow::m_isSquatKick = false;
        return originalNextGenShadow_CSonicStateSquatKickEnd(This);
    }
}

//---------------------------------------------------
// CSonicStateSliding
//---------------------------------------------------
HOOK(int, __fastcall, NextGenShadow_CSonicStateSlidingBegin, 0x11D7110, void* This)
{
    if (NextGenShadow::m_isSpindash)
    {
        // Spin animation over slide
        WRITE_MEMORY(0x11D7124, char*, AnimationSetPatcher::SpinFast);
        WRITE_MEMORY(0x11D6E6A, char*, AnimationSetPatcher::SpinFast);
        WRITE_MEMORY(0x11D6EDB, char*, AnimationSetPatcher::SpinFast);

        // Disable sliding sfx and voice
        WRITE_MEMORY(0x11D722C, int, -1);
        WRITE_MEMORY(0x11D72DC, int, -1);

        // Use spindash pfx
        static char const* ef_ch_sng_spindash = "ef_ch_sng_spindash";
        WRITE_MEMORY(0x11D6A59, uint8_t, 0x30);
        WRITE_MEMORY(0x11D6A0A, char**, &ef_ch_sng_spindash);
        WRITE_MEMORY(0x11D6A80, char**, &ef_ch_sng_spindash);

        // Play trail effect
        Common::SonicContextRequestLocusEffect();
    }
    else
    {
        // Original sliding animation
        WRITE_MEMORY(0x11D7124, uint32_t, 0x15E69CC);
        WRITE_MEMORY(0x11D6E6A, uint32_t, 0x15E69C4);
        WRITE_MEMORY(0x11D6EDB, uint32_t, 0x15E69CC);

        // Sliding sfx and voice
        WRITE_MEMORY(0x11D722C, uint32_t, 2002032);
        WRITE_MEMORY(0x11D72DC, uint32_t, 3002016);

        // Original sliding pfx
        WRITE_MEMORY(0x11D6A59, uint8_t, 0x18);
        WRITE_MEMORY(0x11D6A0A, uint32_t, 0x1E61E00);
        WRITE_MEMORY(0x11D6A80, uint32_t, 0x1E61DA8);

        NextGenShadow::m_isSliding = true;

        // Get current sliding speed
        Eigen::Vector3f playerVelocity(0, 0, 0);
        if (Common::GetPlayerVelocity(playerVelocity))
        {
            NextGenShadow::m_slidingSpeed = playerVelocity.norm();
            NextGenShadow::m_slidingSpeed = max(NextGenShadow::m_slidingSpeed, cShadow_slidingSpeedMin);
            NextGenShadow::m_slidingSpeed = min(NextGenShadow::m_slidingSpeed, cShadow_slidingSpeedMax);
        }
    }

    NextGenShadow::m_slidingTime = NextGenShadow::m_isSpindash ? cShadow_spindashTime : cShadow_slidingTime;
    return originalNextGenShadow_CSonicStateSlidingBegin(This);
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateSlidingAdvance, 0x11D69A0, hh::fnd::CStateMachineBase::CStateBase* This)
{
    originalNextGenShadow_CSonicStateSlidingAdvance(This);

    if (NextGenShadow::CheckChaosBoost())
    {
        return;
    }

    bool bDown, bPressed, bReleased;
    NextGenPhysics::getActionButtonStates(bDown, bPressed, bReleased);
    if (bPressed || NextGenShadow::m_slidingTime - This->m_Time <= 0.0f)
    {
        if (bPressed && NextGenPhysics::checkUseLightSpeedDash())
        {
            // Pressed X button and use light speed dash
            return;
        }

        if (NextGenShadow::m_isSpindash)
        {
            // Cancel spindash, this will still do sweep kick and will also allow jumping before it
            StateManager::ChangeState(StateAction::Walk, *PLAYER_CONTEXT);
            return;
        }
        else
        {
            // Cancel sliding
            slidingEndWasSliding_Shadow = NextGenShadow::m_isSliding;
            StateManager::ChangeState(StateAction::SlidingEnd, *PLAYER_CONTEXT);

            // Set out of control to prevent immediate squat kick
            FUNCTION_PTR(int, __stdcall, SetOutOfControl, 0xE5AC00, CSonicContext * context, float duration);
            SetOutOfControl(*pModernSonicContext, 0.1f);
            return;
        }
    }

    // For 2D slide/spindash, there's one frame delay before Sonic can goto max speed, lower the minSpeed
    float minSpeed = (NextGenShadow::m_isSpindash ? cShadow_spindashSpeed : cShadow_slidingSpeedMin) - 5.0f;
    minSpeed = Common::IsPlayerIn2D() ? 2.0f : minSpeed;

    Eigen::Vector3f playerVelocity;
    bool result = Common::IsPlayerIn2D() ? Common::GetPlayerTargetVelocity(playerVelocity) : Common::GetPlayerVelocity(playerVelocity);
    if (!result || playerVelocity.norm() <= minSpeed)
    {
        if (StateManager::isCurrentAction(StateAction::Sliding))
        {
            slidingEndWasSliding_Shadow = NextGenShadow::m_isSliding;
            StateManager::ChangeState(StateAction::SlidingEnd, *PLAYER_CONTEXT);
            return;
        }
    }
}

void __declspec(naked) NextGenShadow_CSonicStateSlidingEnd()
{
    static uint32_t returnAddress = 0x11D702D;
    __asm
    {
        // Original function
        mov     eax, [edi + 534h]

        // Set spindash/sliding state
        mov     NextGenShadow::m_isSliding, 0
        mov     NextGenShadow::m_isSpindash, 0
        jmp     [returnAddress]
    }
}

void __declspec(naked) NextGenShadow_slidingHorizontalTargetVel2D()
{
    static uint32_t returnAddress = 0x11D98AC;
    __asm
    {
        // Get overrided target velocity
        mov     ecx, esi
        call    NextGenShadow::applySlidingHorizontalTargetVel
        pop     edi
        pop     esi
        pop     ebx
        mov     esp, ebp
        jmp     [returnAddress]
    }
}

void __declspec(naked) NextGenShadow_slidingHorizontalTargetVel3D()
{
    static uint32_t returnAddress = 0x11D953E;
    __asm
    {
        // Get overrided target velocity
        mov     ecx, ebx
        call    NextGenShadow::applySlidingHorizontalTargetVel
        test    al, al
        jnz     jump

        // Original target velocity fallback code
        movaps  xmm0, [esp + 10h]
        movaps  xmmword ptr[ebx + 2A0h], xmm0

        jump:
        jmp     [returnAddress]
    }
}

//---------------------------------------------------
// Spindashing
//---------------------------------------------------
void __declspec(naked) NextGenShadow_startSpindash()
{
    static uint32_t returnAddress = 0x1230C39;
    __asm
    {
        // Original function
        mov     byte ptr[ebx + 5E8h], 1
        mov[ebx + 5E9h], al

        // Set spindash state
        mov     NextGenShadow::m_isSpindash, 1

        // Give Sonic the initial nudge
        mov     ecx, ebx
        call    NextGenShadow::applySpindashImpulse

        jmp     [returnAddress]
    }
}

bool __fastcall NextGenShadow::applySpindashImpulse(void* context)
{
    // This function is necessary for 2D otherwise Sonic will stop immediately
    Eigen::Vector3f playerPosition;
    Eigen::Quaternionf playerRotation;
    if (!Common::GetPlayerTransform(playerPosition, playerRotation)) return false;

    alignas(16) MsgApplyImpulse message {};
    message.m_position = playerPosition;
    message.m_impulse = playerRotation * Eigen::Vector3f::UnitZ();
    message.m_impulseType = ImpulseType::None;
    message.m_outOfControl = 0.0f;
    message.m_notRelative = true;
    message.m_snapPosition = false;
    message.m_pathInterpolate = false;
    message.m_alwaysMinusOne = -1.0f;

    if (m_isSpindash)
    {
        message.m_impulse *= cShadow_spindashSpeed;
    }
    else
    {
        message.m_impulse *= m_slidingSpeed;
    }

    Common::ApplyPlayerApplyImpulse(message);

    return true;
}

bool __fastcall NextGenShadow::applySlidingHorizontalTargetVel(void* context)
{
    Eigen::Vector3f playerPosition;
    Eigen::Quaternionf playerRotation;
    if (!Common::GetPlayerTransform(playerPosition, playerRotation)) return false;
    Eigen::Vector3f playerDir = playerRotation * Eigen::Vector3f::UnitZ();

    // If not moving, don't update brake flip direction
    if (m_isBrakeFlip || (m_isSquatKick && m_squatKickSpeed == 0.0f))
    {
        // Stop Sonic immediately
        float* horizontalVel = (float*)((uint32_t)context + 0x290);
        float* horizontalTargetVel = (float*)((uint32_t)context + 0x2A0);
        horizontalVel[0] = 0;
        horizontalVel[2] = 0;
        horizontalTargetVel[0] = 0;
        horizontalTargetVel[2] = 0;

        return true;
    }

    m_brakeFlipDir = playerDir;
    bool superForm = Common::IsPlayerSuper();
    if (m_isSquatKick)
    {
        // Keep velocity with squat kick
        playerDir *= m_squatKickSpeed;
    }
    else if (m_isSpindash)
    {
        playerDir *= cShadow_spindashSpeed;

        // Double speed for super and normal physics
        if (superForm || !Configuration::m_physics)
        {
            playerDir *= Common::IsPlayerIn2D() ? 1.5f : 2.0f;
        }
    }
    else
    {
        // Use default sliding behavior for super and normal physics
        if (superForm || !Configuration::m_physics)
        {
            return false;
        }

        playerDir *= m_slidingSpeed;
    }

    // For 2D we have to override the actual velocity (+0x290)
    // For 3D we have to override target velocity (+0x2A0)
    float* horizontalVel = (float*)((uint32_t)context + (Common::IsPlayerIn2D() ? 0x290 : 0x2A0));
    horizontalVel[0] = playerDir.x();
    if (Common::IsPlayerIn2D() && !m_isSquatKick)
    {
        horizontalVel[1] = playerDir.y();
    }
    horizontalVel[2] = playerDir.z();

    return true;
}

//---------------------------------------------------
// CSonicStateSquat
//---------------------------------------------------
SharedPtrTypeless spinDashSoundHandle_Shadow;
SharedPtrTypeless spinDashPfxHandle_Shadow;
HOOK(int*, __fastcall, NextGenShadow_CSonicStateSquatBegin, 0x1230A30, void* This)
{
    // Play spindash pfx
    void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x30);
    Common::fCGlitterCreate(*PLAYER_CONTEXT, spinDashPfxHandle_Shadow, matrixNode, "ef_ch_sng_spincharge", 1);

    // Play spindash charge sfx
    Common::SonicContextPlaySound(spinDashSoundHandle_Shadow, 2002042, 1);
    return originalNextGenShadow_CSonicStateSquatBegin(This);
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateSquatAdvance, 0x1230B60, void* This)
{
    originalNextGenShadow_CSonicStateSquatAdvance(This);
    Eigen::Vector3f worldDirection;
    if (!Common::GetPlayerWorldDirection(worldDirection, true)) return;

    // Allow changing Sonic's rotation when charging spindash
    alignas(16) float dir[4] = { worldDirection.x(), worldDirection.y(), worldDirection.z(), 0 };
    NextGenPhysics::applyCSonicRotationAdvance(This, dir);
}

HOOK(int*, __fastcall, NextGenShadow_CSonicStateSquatEnd, 0x12309A0, void* This)
{
    // Stop spindash pfx
    Common::fCGlitterEnd(*PLAYER_CONTEXT, spinDashPfxHandle_Shadow, false);

    spinDashSoundHandle_Shadow.reset();
    if (NextGenShadow::m_isSpindash)
    {
        // Play spindash launch sfx
        static SharedPtrTypeless soundHandle;
        Common::SonicContextPlaySound(soundHandle, 80041024, 1);
    }
    return originalNextGenShadow_CSonicStateSquatEnd(This);
}

HOOK(void, __fastcall, NextGenShadow_CRASH, 0x8CD0F0, void* This, void* Edx, uint16_t* a2, int a3, void* a4)
{
    if (*a2 == 0xA)
    {
        printf("\n\n\n\n*********************************\nCRASH DETECTED\n*********************************\n\n\n\n");
        *a2 = 0;
    }
    originalNextGenShadow_CRASH(This, Edx, a2, a3, a4);
}

//---------------------------------------------------
// Super Shadow
//---------------------------------------------------
HOOK(void, __fastcall, NextGenShadow_CPlayerSpeedStateTransformSpAdvance, 0xE425B0, float* This)
{
    isChaosControl = false;

    // Play super transform longer
    if (This[4] > 2.0f)
    {
        originalNextGenShadow_CPlayerSpeedStateTransformSpAdvance(This);
    }
}

HOOK(int, __fastcall, NextGenShadow_CSonicStatePluginSuperSonicBegin, 0x11D6840, uint32_t This)
{
    int result = originalNextGenShadow_CSonicStatePluginSuperSonicBegin(This);

    // max out chaos boost
    NextGenShadow::SetChaosBoostLevel(3, true);
    return result;
}

HOOK(int, __fastcall, NextGenShadow_CSonicStatePluginSuperSonicEnd, 0x11D6720, uint32_t This)
{
    // reset chaos boost
    NextGenShadow::SetChaosBoostLevel(0, true);
    return originalNextGenShadow_CSonicStatePluginSuperSonicEnd(This);
}

HOOK(void, __fastcall, NextGenShadow_CStateWalkBegin, 0x111C070, uint32_t This)
{
    if (Common::IsPlayerSuper())
    {
        // replace Walk with same animation so Boost Aura doesn't move around
        WRITE_MEMORY(0x111C19F, char*, AnimationSetPatcher::FloatingBoost);
    }
    else
    {
        WRITE_MEMORY(0x111C19F, uint32_t, 0x15F8438);
    }
    originalNextGenShadow_CStateWalkBegin(This);
}

//---------------------------------------------------
// Start animations
//---------------------------------------------------
bool m_startModelHide = false;
void PlayStartTeleport()
{
    if (m_startModelHide)
    {
        // warp start pfx
        SharedPtrTypeless warpHandle;
        void* matrixNode = (void*)((uint32_t)*PLAYER_CONTEXT + 0x10);
        Common::fCGlitterCreate(*PLAYER_CONTEXT, warpHandle, matrixNode, "ef_ch_sha_warp_e", 1);

        SharedPtrTypeless soundHandle;
        Common::SonicContextPlaySound(soundHandle, 80041038, 1);

        m_startModelHide = false;
        NextGenShadow::SetChaosBoostModelVisible(false);
    }
}

void StartTeleportAdvance(hh::fnd::CStateMachineBase::CStateBase* This)
{
    if (This->m_Time >= cShadow_startTeleportTime)
    {
        PlayStartTeleport();
    }
    else
    {
        m_startModelHide = true;
        NextGenShadow::SetChaosBoostModelVisible(false, true);
    }
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateStartDashAdvance, 0xDEEC00, hh::fnd::CStateMachineBase::CStateBase* This)
{
    StartTeleportAdvance(This);
    originalNextGenShadow_CSonicStateStartDashAdvance(This);
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateStartDashEnd, 0xDEEBD0, hh::fnd::CStateMachineBase::CStateBase* This)
{
    PlayStartTeleport();
    originalNextGenShadow_CSonicStateStartDashEnd(This);
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateStartEventAdvance, 0xDEE070, hh::fnd::CStateMachineBase::CStateBase* This)
{
    StartTeleportAdvance(This);
    originalNextGenShadow_CSonicStateStartEventAdvance(This);
}

HOOK(bool, __fastcall, NextGenShadow_CSonicStateStartEventEnd, 0xDEDC30, hh::fnd::CStateMachineBase::CStateBase* This)
{
    PlayStartTeleport();
    return originalNextGenShadow_CSonicStateStartEventEnd(This);
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateStartCrouchingAdvance, 0xDEF180, hh::fnd::CStateMachineBase::CStateBase* This)
{
    StartTeleportAdvance(This);
    originalNextGenShadow_CSonicStateStartCrouchingAdvance(This);
}

HOOK(bool, __fastcall, NextGenShadow_CSonicStateStartCrouchingEnd, 0xDEF0A0, hh::fnd::CStateMachineBase::CStateBase* This)
{
    PlayStartTeleport();
    return originalNextGenShadow_CSonicStateStartCrouchingEnd(This);
}

//---------------------------------------------------
// Main Apply Patches
//---------------------------------------------------
void NextGenShadow::applyPatches()
{
    // Fix character creation
    WRITE_MEMORY(0xE25998, char*, "RightToeBase"); // Toe_R

    // Homing trail texture
    WRITE_MEMORY(0xE5FE61, char*, "homing_sd");

    // Fix fire/electric damage bone location
    WRITE_MEMORY(0x1ABD060, char*, "TopHair1"); // Needle_U_C
    WRITE_MEMORY(0x1ABD064, char*, "LeftHand"); // Hand_L
    WRITE_MEMORY(0x1ABD068, char*, "RightHand"); // Hand_R
    WRITE_MEMORY(0x1ABD06C, char*, "LeftToeBase"); // Toe_L
    WRITE_MEMORY(0x1ABD070, char*, "RightToeBase"); // Toe_R
    WRITE_MEMORY(0x1203CA3, uint32_t, 0x1ABD05C); // electic damage use fire damage offsets
    WRITE_MEMORY(0x1203D7C, uint32_t, 0x1ABD074);

    // Fix Super Shadow pfx bone location
    static float const spineAuraOffset = -50.0f;
    WRITE_MEMORY(0xDA2689, float*, &spineAuraOffset);
    WRITE_MEMORY(0xDA26EA, char*, "TopHair1"); // Needle_U_C
    WRITE_MEMORY(0xDA273B, char*, "LeftHand"); // Hand_L
    WRITE_MEMORY(0xDA278C, char*, "RightHand"); // Hand_R
    WRITE_MEMORY(0xDA27DD, char*, "LeftFoot"); // Foot_L
    WRITE_MEMORY(0xDA285E, char*, "RightFoot"); // Foot_R

    // Super Shadow
    INSTALL_HOOK(NextGenShadow_CPlayerSpeedStateTransformSpAdvance);
    INSTALL_HOOK(NextGenShadow_CSonicSpRenderableSsnUpdate);
    INSTALL_HOOK(NextGenShadow_CSonicStatePluginSuperSonicBegin);
    INSTALL_HOOK(NextGenShadow_CSonicStatePluginSuperSonicEnd);
    INSTALL_HOOK(NextGenShadow_CStateWalkBegin);

    // Always disable stomp voice and sfx
    WRITE_MEMORY(0x1254E04, int, -1);
    WRITE_MEMORY(0x1254F23, int, -1);

    // Don't change to ball model during drift
    WRITE_NOP(0xDF30AB, 0xD);
    WRITE_MEMORY(0xDF2B24, char*, AnimationSetPatcher::SpinFast);

    // Force no change JumpBall hitting dash panel
    WRITE_JUMP(0xDED512, (void*)0xDED63B);

    // Handle model hide/unhide, jet effect
    INSTALL_HOOK(NextGenShadow_MsgRestartStage);
    INSTALL_HOOK(NextGenShadow_CSonicUpdate);
    INSTALL_HOOK(NextGenShadow_AssignFootstepFloorCues);

    // HACK: CRASH PREVENTION
    INSTALL_HOOK(NextGenShadow_CRASH);

    if (!Configuration::m_characterMoveset) return;

    // Disable force landing after 1 second in air
    WRITE_MEMORY(0xE33AC7, uint8_t, 0xEB);

    // Disable stomping
    WRITE_MEMORY(0xDFDDB3, uint8_t, 0xEB);

    // Allow pushing down CObjCscLavaRide without stomp
    WRITE_NOP(0xF1D7F4, 6);
    WRITE_MEMORY(0xF1D7FE, uint16_t, 0x1C0);

    // Start teleport animations
    INSTALL_HOOK(NextGenShadow_CSonicStateStartDashAdvance);
    INSTALL_HOOK(NextGenShadow_CSonicStateStartDashEnd);
    INSTALL_HOOK(NextGenShadow_CSonicStateStartEventAdvance);
    INSTALL_HOOK(NextGenShadow_CSonicStateStartEventEnd);
    INSTALL_HOOK(NextGenShadow_CSonicStateStartCrouchingAdvance);
    INSTALL_HOOK(NextGenShadow_CSonicStateStartCrouchingEnd);

    //-------------------------------------------------------
    // Chaos Attack/Chaos Snap
    //-------------------------------------------------------
    INSTALL_HOOK(NextGenShadow_CSonicStateHomingAttackBegin);
    INSTALL_HOOK(NextGenShadow_CSonicStateHomingAttackAdvance);
    INSTALL_HOOK(NextGenShadow_CSonicStateHomingAttackEnd);

    // Handle homing attack voice ourselves
    WRITE_MEMORY(0x11184E4, int, -1);
    WRITE_MEMORY(0x1118512, int, -1);

    INSTALL_HOOK(NextGenShadow_CSonicStateHomingAttackAfterBegin);
    INSTALL_HOOK(NextGenShadow_CSonicStateHomingAttackAfterAdvance);
    INSTALL_HOOK(NextGenShadow_CSonicStateHomingAttackAfterEnd);

    //-------------------------------------------------------
    // Chaos Spear/Chaos Blast/Chaos Boost
    //-------------------------------------------------------
    INSTALL_HOOK(NextGenShadow_AirAction);
    WRITE_JUMP(0xDFFEA3, NextGenShadow_AirActionAfterFlag);
    INSTALL_HOOK(NextGenShadow_CSonicStateTrickAttackBegin);
    INSTALL_HOOK(NextGenShadow_CSonicStateTrickAttackAdvance);
    INSTALL_HOOK(NextGenShadow_CSonicStateTrickAttackEnd);
    EnemyShock::applyPatches();

    //-------------------------------------------------------
    // Chaos Control
    //-------------------------------------------------------
    // skip MsgChangeGameSpeed and sound change
    WRITE_JUMP(0x111AF35, (void*)0x111AF71); 
    WRITE_NOP(0x111AF7B, 3);

    INSTALL_HOOK(NextGenShadow_CPlayerSpeedStatePluginTimeBreakBegin);
    INSTALL_HOOK(NextGenShadow_CPlayerSpeedStatePluginTimeBreakAdvance);
    INSTALL_HOOK(NextGenShadow_CPlayerSpeedStatePluginTimeBreakEnd);

    //INSTALL_HOOK(NextGenShadow_CParticleManagerAdvance);
    INSTALL_HOOK(NextGenShadow_GlitterCHandleAdvance);

    INSTALL_HOOK(NextGenShadow_CSonicStateDivingDiveAdvance);
    INSTALL_HOOK(NextGenShadow_CSonicStateDivingFloatAdvance);
    INSTALL_HOOK(NextGenShadow_CSonicStateFloatingBoostAdvance);
    INSTALL_HOOK(NextGenShadow_CSonicStateGrindBegin);

    // Situation that stops Chaos Control
    INSTALL_HOOK(NextGenShadow_MsgPlayerGoal);
    INSTALL_HOOK(NextGenShadow_MsgStartHangOn);
    INSTALL_HOOK(NextGenShadow_CObjCannonMsgNotifyObjectEvent);
    INSTALL_HOOK(NextGenShadow_CObjSelectCanonMsgNotifyObjectEvent);
    INSTALL_HOOK(NextGenShadow_CObjJumpSelectorMsgNotifyObjectEvent);
    INSTALL_HOOK(NextGenShadow_CObjStopPeopleMsgNotifyObjectEvent);
    INSTALL_HOOK(NextGenShadow_CRivalShadowMsgDamage);

    //-------------------------------------------------------
    // X-Action State handling
    //-------------------------------------------------------
    // Return 0 for Squat and Sliding, handle them ourselves
    WRITE_MEMORY(0xDFF8D5, uint8_t, 0xEB, 0x05);
    WRITE_MEMORY(0xDFF856, uint8_t, 0xE9, 0x81, 0x00, 0x00, 0x00);
    INSTALL_HOOK(NextGenShadow_BActionHandler);
    INSTALL_HOOK(NextGenShadow_MsgNotifyObjectEvent);

    //-------------------------------------------------------
    // Brake Flip
    //-------------------------------------------------------
    INSTALL_HOOK(NextGenShadow_CSonicStateSlidingEndBegin);
    INSTALL_HOOK(NextGenShadow_CSonicStateSlidingEndAdvance);
    INSTALL_HOOK(NextGenShadow_CSonicStateSlidingEndEnd);

    //-------------------------------------------------------
    // Sweep Kick
    //-------------------------------------------------------
    INSTALL_HOOK(NextGenShadow_CSonicStateSquatKickBegin);
    INSTALL_HOOK(NextGenShadow_CSonicStateSquatKickAdvance);
    WRITE_JUMP(0x125291F, NextGenShadow_CSonicStateSquatKickAdvanceTransitionOut);
    INSTALL_HOOK(NextGenShadow_CSonicStateSquatKickEnd);

    // Don't transition out to Stand, only Walk and Fall
    WRITE_MEMORY(0x1252905, uint32_t, 0x15F4FE8);

    // Enable sweep kick attack collision immediately
    static double const c_sweepKickActivateTime = 0.0;
    WRITE_MEMORY(0x125299E, double*, &c_sweepKickActivateTime);

    // Change SquatKick's collision the same as sliding
    WRITE_MEMORY(0xDFCD6D, uint8_t, 0x5); // switch 6 cases
    static uint32_t const collisionSwitchTable[6] =
    {
        0xDFCD7B, // normal
        0xDFCDC0, // slide
        0xDFCD7B, // boost
        0xDFCD7B,
        0xDFCDFA, // unused
        0xDFCDC0  // squat kick
    };
    WRITE_MEMORY(0xDFCD77, uint32_t*, collisionSwitchTable);

    // Change "Stomping" type object physics to "Normal"
    WRITE_MEMORY(0xE9FFC9, uint32_t, 5);

    // Play chaos attack voice
    WRITE_MEMORY(0x1252740, uint32_t, 3002032);

    //-------------------------------------------------------
    // Anti-Gravity
    //-------------------------------------------------------
    // Change slide to hit enemy as if you're squat kicking (TypeSonicSquatKick)
    WRITE_MEMORY(0x11D72F3, uint32_t, SonicCollision::TypeSonicSquatKick);
    WRITE_MEMORY(0x11D7090, uint32_t, SonicCollision::TypeSonicSquatKick);

    // Disable all Sliding transition out, handle them outselves
    WRITE_MEMORY(0x11D6B7D, uint8_t, 0xEB);
    WRITE_MEMORY(0x11D6CA2, uint8_t, 0xEB);
    WRITE_MEMORY(0x11D6F82, uint8_t, 0x90, 0xE9);

    // Change sliding animation if we are spindashing, handle transition out
    INSTALL_HOOK(NextGenShadow_CSonicStateSlidingBegin);
    INSTALL_HOOK(NextGenShadow_CSonicStateSlidingAdvance);
    WRITE_JUMP(0x11D7027, NextGenShadow_CSonicStateSlidingEnd);

    // Set constant sliding speed
    WRITE_JUMP(0x11D989B, NextGenShadow_slidingHorizontalTargetVel2D);
    WRITE_JUMP(0x11D98A7, NextGenShadow_slidingHorizontalTargetVel2D);
    WRITE_JUMP(0x11D9532, NextGenShadow_slidingHorizontalTargetVel3D);

    // Do not use sliding fail pfx
    WRITE_NOP(0x11D6A6D, 2);

    // Increase turn rate for Spindash and Anti-Gravity
    static float slideTurnRate = 100.0f;
    WRITE_MEMORY(0x11D9441, float*, &slideTurnRate);

    //-------------------------------------------------------
    // Spindashing
    //-------------------------------------------------------
    // Spin animation for Squat
    WRITE_MEMORY(0x1230A85, char*, AnimationSetPatcher::SpinFast); // slide begin animation
    WRITE_MEMORY(0x1230A9F, char*, AnimationSetPatcher::SpinFast); // slide begin animation
    WRITE_MEMORY(0x1230D74, char*, AnimationSetPatcher::SpinFast); // slide hold animation

    // Use spindash when release button
    WRITE_JUMP(0x1230BDB, NextGenShadow_startSpindash);
    WRITE_MEMORY(0x1230C3A, uint32_t, 0x15F5108); // change state to sliding

    // If in tight spaces, still allow Sonic to unduck (aka use spindash)
    WRITE_NOP(0x1230BCB, 0x2);

    // Don't allow stick move start sliding from squat
    WRITE_MEMORY(0x1230D62, uint8_t, 0xEB);
    WRITE_MEMORY(0x1230DA9, uint8_t, 0xE9, 0xA8, 0x00, 0x00, 0x00, 0x90);

    // Play spindash sfx
    INSTALL_HOOK(NextGenShadow_CSonicStateSquatBegin);
    INSTALL_HOOK(NextGenShadow_CSonicStateSquatAdvance);
    INSTALL_HOOK(NextGenShadow_CSonicStateSquatEnd);
}
