#include "NextGenShadow.h"
#include "Configuration.h"
#include "Application.h"
#include "AnimationSetPatcher.h"
#include "VoiceOver.h"

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

int NextGenShadow::m_chaosAttackCount = -1;
float const cShadow_chaosAttackWaitTime = 0.2f;

float NextGenShadow::m_xHeldTimer = 0.0f;
bool NextGenShadow::m_enableAutoRunAction = true;

// Chaos Boost
uint8_t NextGenShadow::m_chaosBoostLevel = 0u;
float NextGenShadow::m_chaosMaturity = 0.0f;

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
        "AirBoost"
    };

    alignas(16) MsgGetAnimationInfo message {};
    Common::SonicContextGetAnimationInfo(message);

    return !Common::IsPlayerSuper() && c_jetAnimations.count(message.m_name);
}

SharedPtrTypeless jetRightFront;
SharedPtrTypeless jetRightBack;
SharedPtrTypeless jetLeftFront;
SharedPtrTypeless jetLeftBack;
void NextGenShadow::SetJetEffectVisible(bool visible)
{
    auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
    if (visible)
    {
        {
            auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("RightFoot");
            Common::fCGlitterCreate(*PLAYER_CONTEXT, jetRightBack, &attachBone, "ef_bo_sha_yh2_jet_back", 1);
        }
        {
            auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("RightToeBase");
            Common::fCGlitterCreate(*PLAYER_CONTEXT, jetRightFront, &attachBone, "ef_bo_sha_yh2_jet_front", 1);
        }
        {
            auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("LeftFoot");
            Common::fCGlitterCreate(*PLAYER_CONTEXT, jetLeftBack, &attachBone, "ef_bo_sha_yh2_jet_back", 1);
        }
        {
            auto attachBone = context->m_pPlayer->m_spCharacterModel->GetNode("LeftToeBase");
            Common::fCGlitterCreate(*PLAYER_CONTEXT, jetLeftFront, &attachBone, "ef_bo_sha_yh2_jet_front", 1);
        }
    }
    else if (jetRightFront)
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

HOOK(void, __fastcall, NextGenShadow_CSonicUpdateJetEffect, 0xE6BF20, Sonic::Player::CPlayerSpeed* This, void* Edx, float* dt)
{
    if (*pModernSonicContext)
    {
        if (NextGenShadow::ShouldPlayJetEffect())
        {
            if (!jetRightFront)
            {
                NextGenShadow::SetJetEffectVisible(true);
            }
        }
        else if (jetRightFront)
        {
            NextGenShadow::SetJetEffectVisible(false);
        }
    }

    originalNextGenShadow_CSonicUpdateJetEffect(This, Edx, dt);
}

//---------------------------------------------------
// Chaos Attack
//---------------------------------------------------
HOOK(int, __fastcall, NextGenShadow_CSonicStateHomingAttackBegin, 0x1232040, hh::fnd::CStateMachineBase::CStateBase* This)
{
    return originalNextGenShadow_CSonicStateHomingAttackBegin(This);
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateHomingAttackAdvance, 0x1231C60, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();

    Eigen::Vector3f velocity;
    Common::GetPlayerVelocity(velocity);

    if (!context->m_HomingAttackTargetActorID)
    {
        if (velocity.norm() < 10.0f)
        {
            // hit a wall, unable to keep velocity
            StateManager::ChangeState(StateAction::Fall, *PLAYER_CONTEXT);
        }
        else
        {
            // No homing attack target, keep constant forward velocity
            velocity.y() = 0.0f;
            velocity = velocity.normalized() * context->m_spParameter->Get<float>(Sonic::Player::ePlayerSpeedParameter_HomingSpeedOfDummy);
            Common::SetPlayerVelocity(velocity);
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
    return originalNextGenShadow_CSonicStateHomingAttackEnd(This);
}

HOOK(int32_t*, __fastcall, NextGenShadow_CSonicStateHomingAttackAfterBegin, 0x1118300, hh::fnd::CStateMachineBase::CStateBase* This)
{
    // run the original function as some context members need to be set
    int32_t* result = originalNextGenShadow_CSonicStateHomingAttackAfterBegin(This);

    // handle Shadow stopping for chaos attack
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    if (!context->StateFlag(eStateFlag_EnableHomingAttackOnDiving))
    {
        NextGenShadow::m_chaosAttackCount = 0;

        Common::SonicContextChangeAnimation(AnimationSetPatcher::ChaosAttackWait);
        Common::SetPlayerVelocity(Eigen::Vector3f::Zero());
    }

    return result;
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateHomingAttackAfterAdvance, 0x1118600, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    if (NextGenShadow::m_chaosAttackCount >= 0)
    {
        Common::SetPlayerVelocity(Eigen::Vector3f::Zero());

        if (This->m_Time > cShadow_chaosAttackWaitTime)
        {
            // timeout, resume original homing attack after
            This->m_Time = 0.0f;
            NextGenShadow::m_chaosAttackCount = -1;

            // apply up velocity
            Eigen::Vector3f velocity(0, 0, 0);
            velocity.y() = context->m_spParameter->Get<float>(Sonic::Player::ePlayerSpeedParameter_HomingAttackAfterUpVelocity)
                         * context->m_spParameter->Get<float>(Sonic::Player::ePlayerSpeedParameter_AttackAfterImpluseVelocityCoeff);
            Common::SetPlayerVelocity(velocity);

            // play jump sfx, vfx
            static SharedPtrTypeless soundHandle;
            Common::SonicContextPlaySound(soundHandle, 2002027, 1);
            VoiceOver::playJumpVoice();

            // play random homing attack after animation
            Common::SonicContextChangeAnimation((const char*)*((uint32_t*)0x1E75E18 + rand() % 6));

            // make sure it's in air again
            Common::SonicContextSetInAirData(context);
            return;
        }
    }
    else
    {
        originalNextGenShadow_CSonicStateHomingAttackAfterAdvance(This);
    }
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateHomingAttackAfterEnd, 0x11182F0)
{
    // resume ground detection
    WRITE_MEMORY(0xE63A31, uint8_t, 0x74);

    NextGenShadow::m_chaosAttackCount = -1;
}

//---------------------------------------------------
// Chaos Boost
//---------------------------------------------------
HOOK(int, __fastcall, NextGenShadow_MsgRestartStage, 0xE76810, Sonic::Player::CPlayer* player, void* Edx, void* message)
{
    int result = originalNextGenShadow_MsgRestartStage(player, Edx, message);

    // HUD reset is handled by 06 HUD
    NextGenShadow::SetChaosBoostLevel(0, false);
    NextGenShadow::SetJetEffectVisible(false);

    // Re-enable auto run actions (squat kick)
    NextGenShadow::m_enableAutoRunAction = true;

    return result;
}

void NextGenShadow::SetChaosBoostModelVisible(bool visible)
{
    auto const& model = Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer->m_spCharacterModel;
    model->m_spModel->m_NodeGroupModels[0]->m_Visible = !visible;
    model->m_spModel->m_NodeGroupModels[1]->m_Visible = visible;
}

void NextGenShadow::SetChaosBoostLevel(uint8_t level, bool notifyHUD)
{
    m_chaosBoostLevel = level;
    m_chaosMaturity = 0.0f; // change level always resets maturity
    SetChaosBoostModelVisible(level > 0);

    if (notifyHUD)
    {
        S06HUD_API::SetShadowChaosLevel(level, 0.0f);
    }
}

void NextGenShadow::AddChaosMaturity(float amount)
{
    if (Configuration::m_model != Configuration::ModelType::Shadow) return;

    float maturity = m_chaosMaturity;
    m_chaosMaturity = min(100.0f, maturity + amount);
    S06HUD_API::SetShadowChaosLevel(m_chaosBoostLevel, m_chaosMaturity);

    if (m_chaosMaturity == 100.0f && maturity < 100.0f)
    {
        // TODO: play sfx
    }
}

//-------------------------------------------------------
// Chaos Spear/Chaos Blast
//-------------------------------------------------------
void __declspec(naked) NextGenShadow_AirAction()
{
    static uint32_t successAddress = 0xDFFEBA;
    static uint32_t returnAddress = 0xDFFEAD;
    static uint32_t fnButtonPress = 0xD97E00;
    __asm
    {
        // original function
        original:
        mov     eax, [esi + 11Ch]
        jmp     [returnAddress]

        jump:
        jmp     [successAddress]
    }
}

class CObjChaosSpear : public Sonic::CGameObject3D
{
private:
    uint32_t m_OwnerActorID; // same for CSkateBoardAttack

    Hedgehog::Math::CVector m_Position;
    Hedgehog::Math::CVector m_Velocity;
    float m_LifeTime;

    Sonic::CGlitterPlayer* m_pGlitterPlayer;
    boost::shared_ptr<Sonic::CMatrixNodeTransform> m_spNodeEventCollision;

public:
    CObjChaosSpear
    (
        uint32_t _OwnerActorID,
        Hedgehog::Math::CVector const& _Position,
        Hedgehog::Math::CVector const& _TargetPosition
    )
        : m_OwnerActorID(_OwnerActorID)
        , m_Position(_Position)
        , m_Velocity((_TargetPosition - _Position).normalized() * 40.0f) // TODO
        , m_LifeTime(0.0f)
    {

    }

    ~CObjChaosSpear()
    {
        delete m_pGlitterPlayer;
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

        // Set initial rotation
        Hedgehog::Math::CVector const dir = m_Velocity.normalized();
        Hedgehog::Math::CVector dirXZ = dir; dirXZ.y() = 0.0f;
        Hedgehog::Math::CQuaternion rotYaw = Hedgehog::Math::CQuaternion::FromTwoVectors(Hedgehog::Math::CVector::UnitZ(), dirXZ.head<3>());
        Hedgehog::Math::CQuaternion rotPitch = Hedgehog::Math::CQuaternion::FromTwoVectors(dirXZ.head<3>(), dir.head<3>());
        m_spMatrixNodeTransform->m_Transform.SetRotationAndPosition(rotPitch * rotYaw, m_Position);
        m_spMatrixNodeTransform->NotifyChanged();

        // play pfx
        m_pGlitterPlayer = Sonic::CGlitterPlayer::Make(pGameDocument);
        m_pGlitterPlayer->PlayContinuous(Sonic::CGameDocument::GetInstance(), m_spMatrixNodeTransform, "ef_bo_sha_yh2_spear", 1.0f, 10, 0);
        m_pGlitterPlayer->PlayContinuous(Sonic::CGameDocument::GetInstance(), m_spMatrixNodeTransform, "ef_bo_sha_yh2_spear_tail", 1.0f, 10, 0);
    
        // set up collision with enemy
        m_spNodeEventCollision = boost::make_shared<Sonic::CMatrixNodeTransform>();
        m_spNodeEventCollision->m_Transform.SetPosition(hh::math::CVector(0.0f, 0.0f, 0.0f));
        m_spNodeEventCollision->NotifyChanged();
        m_spNodeEventCollision->SetParent(m_spMatrixNodeTransform.get());

        hk2010_2_0::hkpSphereShape* shapeEventTrigger1 = new hk2010_2_0::hkpSphereShape(0.3f);
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
                FUNCTION_PTR(void, __thiscall, CSkateBoardAttack_MsgHitEventCollision, 0xE2A750, void* This, Hedgehog::Universe::Message& message);
                CSkateBoardAttack_MsgHitEventCollision(this, message);
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
        Hedgehog::Math::CVector const posPrev = m_Position;
        UpdatePosition(m_Position + m_Velocity * updateInfo.DeltaTime);

        Eigen::Vector4f const rayStartPos(posPrev.x(), posPrev.y(), posPrev.z(), 1.0f);
        Eigen::Vector4f const rayEndPos(m_Position.x(), m_Position.y(), m_Position.z(), 1.0f);
        Eigen::Vector4f outPos;
        Eigen::Vector4f outNormal;
        if (Common::fRaycast(rayStartPos, rayEndPos, outPos, outNormal, *(uint32_t*)0x1E0AFB4))
        {
            // hit terrain
            Kill();
        }
        else
        {
            // time out
            m_LifeTime += updateInfo.DeltaTime;
            if (m_LifeTime >= 5.0f)
            {
                Kill();
            }
        }
    }

    void UpdatePosition(Hedgehog::Math::CVector const& _Position)
    {
        m_Position = _Position;
        m_spMatrixNodeTransform->m_Transform.SetPosition(m_Position);
        m_spMatrixNodeTransform->NotifyChanged();
    }

    void Kill()
    {
        SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
    }
};

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

    CSonicStateFlags* flags = Common::GetSonicStateFlags();
    if (flags->KeepRunning && !m_enableAutoRunAction)
    {
        // Cannot use any action during auto run
        return false;
    }

    bool moving = playerVelocity.norm() > 0.2f;
    bool canUseSpindash = !moving || (Configuration::Shadow::m_rapidSpindash && !flags->KeepRunning);

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
                if (Configuration::Shadow::m_rapidSpindash)
                {
                    // Disable anti-gravity if rapid spindash is enabled
                }
                else
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
    if (Common::GetWorldInputDirection(inputDirection) && inputDirection.isZero())
    {
        // Do brake flip animation
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
    }
    return result;
}

HOOK(int*, __fastcall, NextGenShadow_CSonicStateSlidingEndEnd, 0x1230E60, void* This)
{
    NextGenShadow::m_isBrakeFlip = false;
    return originalNextGenShadow_CSonicStateSlidingEndEnd(This);
}

//---------------------------------------------------
// CSonicStateSquatKick
//---------------------------------------------------
HOOK(int*, __fastcall, NextGenShadow_CSonicStateSquatKickBegin, 0x12526D0, void* This)
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

    // TODO: Play squat kick sfx and pfx
    static SharedPtrTypeless soundHandle;
    //Common::SonicContextPlaySound(soundHandle, 80041021, 1); 

    NextGenShadow::m_isSquatKick = true;
    return originalNextGenShadow_CSonicStateSquatKickBegin(This);
}

HOOK(void, __fastcall, NextGenShadow_CSonicStateSquatKickAdvance, 0x1252810, void* This)
{
    originalNextGenShadow_CSonicStateSquatKickAdvance(This);

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

    NextGenShadow::m_isSquatKick = false;
    return originalNextGenShadow_CSonicStateSquatKickEnd(This);
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
        slidingEndWasSliding_Shadow = NextGenShadow::m_isSliding;
        StateManager::ChangeState(StateAction::SlidingEnd, *PLAYER_CONTEXT);
        return;
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

    // Always disable stomp voice and sfx
    WRITE_MEMORY(0x1254E04, int, -1);
    WRITE_MEMORY(0x1254F23, int, -1);

    // Handle model hide/unhide, jet effect
    INSTALL_HOOK(NextGenShadow_MsgRestartStage);
    INSTALL_HOOK(NextGenShadow_CSonicUpdateJetEffect);

    if (!Configuration::m_characterMoveset) return;

    //-------------------------------------------------------
    // Chaos Attack
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
    // Chaos Boost
    //-------------------------------------------------------

    //-------------------------------------------------------
    // Chaos Spear/Chaos Blast
    //-------------------------------------------------------
    WRITE_JUMP(0xDFFEA3, NextGenShadow_AirAction);

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
