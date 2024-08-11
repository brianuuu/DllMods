#include "NextGenShadow.h"
#include "Configuration.h"
#include "Application.h"
#include "AnimationSetPatcher.h"

//---------------------------------------------------
// Animation
//---------------------------------------------------
void NextGenShadow::setAnimationSpeed_Shadow(NextGenAnimation& data)
{
    if (Configuration::m_physics)
    {
        
    }
    else
    {

    }
}

//---------------------------------------------------
// Main Variables
//---------------------------------------------------
float NextGenShadow::m_xHeldTimer = 0.0f;

//-------------------------------------------------------
// Chaos Spear/Chaos Blast
//-------------------------------------------------------
void __declspec(naked) NextGenShadow_AirAction()
{
    static uint32_t returnAddress = 0xDFFEE5;
    static uint32_t fnButtonPress = 0xD97E00;
    __asm
    {
        call    NextGenShadow::bActionHandlerImpl

        jump:
        jmp     [returnAddress]
    }
}

//---------------------------------------------------
// Main Variables
//---------------------------------------------------
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


    if (!Configuration::m_characterMoveset) return;

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
