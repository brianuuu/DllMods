#include "NextGenShadow.h"
#include "Configuration.h"
#include "Application.h"
#include "AnimationSetPatcher.h"

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
float NextGenShadow::m_xHeldTimer = 0.0f;

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
        "Boost", "BoostL", "BoostR", "BoostWallL", "BoostWallR"
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
    // No jet effect for classic Sonic
    if (!*pModernSonicContext)
    {
        originalNextGenShadow_CSonicUpdateJetEffect(This, Edx, dt);
        return;
    }

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

    originalNextGenShadow_CSonicUpdateJetEffect(This, Edx, dt);
}

//---------------------------------------------------
// Chaos Boost
//---------------------------------------------------
HOOK(int, __fastcall, NextGenShadow_MsgRestartStage, 0xE76810, Sonic::Player::CPlayer* player, void* Edx, void* message)
{
    // HUD reset is handled by 06 HUD
    NextGenShadow::SetChaosBoostLevel(0, false);
    NextGenShadow::SetJetEffectVisible(false);

    return originalNextGenShadow_MsgRestartStage(player, Edx, message);
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

bool NextGenShadow::bActionHandlerImpl()
{
    bool bDown, bPressed, bReleased;
    NextGenPhysics::getActionButtonStates(bDown, bPressed, bReleased);
    if (bDown)
    {
        // Remember how long we held X, always run in 60fps
        m_xHeldTimer += 1.0f / 60.0f;
    }
    else
    {
        if (bReleased)
        {
            auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
            Hedgehog::Math::CVector position = context->m_spMatrixNode->m_Transform.m_Position + Hedgehog::Math::CVector::UnitY() * 0.5f;
            Hedgehog::Math::CVector targetPosition = position + context->m_spMatrixNode->m_Transform.m_Rotation * Hedgehog::Math::CVector::UnitZ();

            if (context->m_HomingAttackTargetActorID)
            {
                context->m_pPlayer->SendMessageImm(context->m_HomingAttackTargetActorID, boost::make_shared<Sonic::Message::MsgGetHomingAttackPosition>(&targetPosition));
            }

            boost::shared_ptr<Sonic::CGameObject> spChaosSpear = boost::make_shared<CObjChaosSpear>(0u, position, targetPosition);
            Sonic::CGameDocument::GetInstance()->AddGameObject(spChaosSpear);
        }

        m_xHeldTimer = 0.0f;
    }

    return false;
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

    // Handle model hide/unhide, jet effect
    INSTALL_HOOK(NextGenShadow_MsgRestartStage);
    INSTALL_HOOK(NextGenShadow_CSonicUpdateJetEffect);

    if (!Configuration::m_characterMoveset) return;

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
}
