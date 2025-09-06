#include "QTEReactionPlate.h"

class alignas(16) MsgGetReactionPlateInfo : public Hedgehog::Universe::MessageTypeGet
{
public:
    HH_FND_MSG_MAKE_TYPE(0x1680DE8);

    uint32_t m_Field10;
    uint32_t m_Field14;
    uint32_t m_Field18;
    uint32_t m_Field1C;
    uint32_t m_Field20;
    uint32_t m_Field24;
    uint32_t m_Field28;
    uint32_t m_Field2C;
    float m_Field30;
    uint32_t m_Field34;
    float m_Field38;
    float m_Field3C;
};

void fE5CD90
(
    uint32_t targetActorID,
    Sonic::Player::CPlayerContext* pPlayerContext,
    const hh::math::CVector* pPosition,
    Sonic::Player::CPlayerSpeedStateReactionLand::EAnimationType animationType,
    float minVelocity,
    float maxVelocity,
    int score,
    int difficulty
)
{
    static uint32_t sub_E5CD90 = 0xE5CD90;
    __asm
    {
        push    difficulty
        push    score
        push    maxVelocity
        push    minVelocity
        push    animationType
        push    pPosition
        push    pPlayerContext
        mov     eax, targetActorID
        call    [sub_E5CD90]
    }
}

void fE6F900
(
    void* a1,
    uint32_t a2,
    float a3,
    float a4,
    float a5
)
{
    static uint32_t sub_E6F900 = 0xE6F900;
    __asm
    {
        push a5
        push a4
        push a3
        push a2
        mov edi, a1
        call[sub_E6F900]
    }
}

int m_qteReactionPlateBaseScore = 0;
int m_qteReactionPlateAddScore = 0;
SharedPtrTypeless reactionPlatePfxHandle;
void ProcMsgHitReactionPlate(Sonic::Player::CPlayerSpeed* This, const Sonic::Message::MsgHitReactionPlate& message)
{
    const float minVelocity = message.m_JumpMinVelocity <= 0 ? This->GetContext()->m_spParameter->Get<float>(
        Sonic::Player::ePlayerSpeedParameter_ReactionJumpMinVelocity) : message.m_JumpMinVelocity;

    const float maxVelocity = message.m_JumpMaxVelocity <= 0 ? This->GetContext()->m_spParameter->Get<float>(
        Sonic::Player::ePlayerSpeedParameter_ReactionJumpMaxVelocity) : message.m_JumpMaxVelocity;

    if (message.m_Type != 0) // 0 == Begin
    {
        if (message.m_Type != 5) // 5 == End
        {
            const auto pState = This->GetContext()->ChangeState<Sonic::Player::CPlayerSpeedStateReactionLand>();

            const auto reactionJumpQTE = This->GetContext()->m_spReactionJumpQTE_HUDPtr.get();

            if (!*(reinterpret_cast<bool*>(reactionJumpQTE) + 0x3A))
            {
                const auto reactionPlateInfo = boost::make_shared<MsgGetReactionPlateInfo>();
                This->SendMessageImm(message.m_SenderActorID, reactionPlateInfo);

                fE6F900(
                    reactionJumpQTE,
                    reactionPlateInfo->m_Field18,
                    reactionPlateInfo->m_Field30,
                    reactionPlateInfo->m_Field38,
                    reactionPlateInfo->m_Field3C);
            }

            *(reinterpret_cast<float*>(reactionJumpQTE) + 0x10) = *(reinterpret_cast<float*>(reactionJumpQTE) + 0xC);
            *(reinterpret_cast<bool*>(reactionJumpQTE) + 0x3C) = true;

            pState->m_TargetActorID = message.m_TargetActorID;
            pState->m_JumpMinVelocity = minVelocity;
            pState->m_JumpMaxVelocity = maxVelocity;
            *(reinterpret_cast<float*>(pState) + 0x1E) = *(reinterpret_cast<const float*>(&message) + 0x14);
            *(reinterpret_cast<float*>(pState) + 0x20) = *(reinterpret_cast<const float*>(&message) + 0x16);

            hh::math::CVector targetPosition;
            This->SendMessageImm(message.m_TargetActorID, boost::make_shared<Sonic::Message::MsgGetPosition>(targetPosition));

            const auto rightDirection = message.m_Direction.cross(hh::math::CVector::UnitY()).normalized();
            const auto targetDirection = (targetPosition - message.m_Position).normalized();
            const bool facesLeft = rightDirection.dot(targetDirection) < 0;

            bool const isUnleashedSonic = This->GetContext()->m_pPlayer->m_spCharacterModel->GetNode("SonicRoot") != nullptr;
            This->GetContext()->ChangeAnimation(isUnleashedSonic ? (facesLeft ? "ReactionLandLSWA" : "ReactionLandRSWA") : (facesLeft ? "ReactionLandL" : "ReactionLandR"));
            pState->m_AnimationType = facesLeft ?
                Sonic::Player::CPlayerSpeedStateReactionLand::eAnimationType_ReactionJumpL :
                Sonic::Player::CPlayerSpeedStateReactionLand::eAnimationType_ReactionJumpR;

            This->GetContext()->PlaySound(4002046, false);

            const auto yawDirection = hh::math::CVector(message.m_Direction.x(), 0, message.m_Direction.z()).normalized();
            const float yaw = atan2(yawDirection.x(), yawDirection.z());
            const float pitch = asin(-message.m_Direction.y());
            const auto rotation = Eigen::AngleAxisf(pitch, Eigen::Vector3f::UnitX()) * Eigen::AngleAxisf(yaw, Eigen::Vector3f::UnitY());

            This->GetContext()->m_VerticalRotation = rotation;
            This->GetContext()->m_HorizontalRotation.setIdentity();
            This->GetContext()->m_ModelUpDirection = message.m_Direction;

            This->GetContext()->m_spMatrixNode->m_Transform.SetRotationAndPosition(rotation * This->GetContext()->m_HorizontalRotation, message.m_Position + message.m_Direction * 0.25f);
            This->GetContext()->m_spMatrixNode->NotifyChanged();

            m_qteReactionPlateBaseScore = max(0, message.m_Score);
        }
    }
    else
    {
        // TODO: is Wii config
        // Spring sfx for Wii
        This->GetContext()->PlaySound(4002013, false);

        fE5CD90
        (
            message.m_TargetActorID,
            This->m_spContext.get(),
            &message.m_Position,
            Sonic::Player::CPlayerSpeedStateReactionLand::eAnimationType_JumpSpring,
            minVelocity,
            maxVelocity,
            message.m_Score,
            1
        );
    }
}

HOOK(bool, __fastcall, QTEReactionPlate_CPlayerSpeedProcessMessage, 0xE6E000, hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message, bool flag)
{
    if (flag && message.Is<Sonic::Message::MsgHitReactionPlate>())
    {
        ProcMsgHitReactionPlate(static_cast<Sonic::Player::CPlayerSpeed*>(This),
            static_cast<Sonic::Message::MsgHitReactionPlate&>(message));
        return true;
    }

    return originalQTEReactionPlate_CPlayerSpeedProcessMessage(This, Edx, message, flag);
}

HOOK(bool, __stdcall, QTEReactionPlate_PlayUIEffect, 0xE6F3E0, float* This, void* a2)
{
    bool result = originalQTEReactionPlate_PlayUIEffect(This, a2);
    if (result)
    {
        float inputTime = This[12];
        float maxTime = This[9];
        m_qteReactionPlateAddScore = max(0, (int)((maxTime - 0.5f) * 500.f * (maxTime - inputTime) / maxTime));

        // TODO: is Wii config
        // Wii OK sfx
        SharedPtrTypeless soundHandle;
        Common::SonicContextPlaySound(soundHandle, 3000812995, 0);
    }

    return result;
}

HOOK(void, __fastcall, QTEReactionPlate_CPlayerSpeedStateReactionJumpAdvance, 0x119F6A0, uint32_t This)
{
    bool const hasUI = *(bool*)(This + 176);
    originalQTEReactionPlate_CPlayerSpeedStateReactionJumpAdvance(This);
    if (!hasUI && *(bool*)(This + 176))
    {
        // TODO: is Wii config
        // Wii button appear sfx
        SharedPtrTypeless soundHandle;
        Common::SonicContextPlaySound(soundHandle, 3000813003, 0);
    }
}

HOOK(int, __fastcall, QTEReactionPlate_CPlayerSpeedStateReactionLandEnd, 0x124B7D0, hh::fnd::CStateMachineBase::CStateBase* This)
{
    uint32_t context = (uint32_t)This->GetContextBase();
    uint32_t v3 = *(uint32_t*)(context + 2036);
    bool success = *(bool*)(v3 + 57);

    int score = m_qteReactionPlateBaseScore + m_qteReactionPlateAddScore;
    if (score > 0 && success)
    {
        ScoreGenerationsAPI::AddScore(score);
        UnleashedHUD_API::AddTrickScore(score);
    }
    m_qteReactionPlateBaseScore = 0;
    m_qteReactionPlateAddScore = 0;

    return originalQTEReactionPlate_CPlayerSpeedStateReactionLandEnd(This);
}

HOOK(void, __fastcall, QTEReactionPlate_MsgHitEventCollision, 0x1017020, uint32_t* This, void* Edx, void* message)
{
    auto* player = Sonic::Player::CPlayerSpeedContext::GetInstance()->m_pPlayer;
    uint32_t type = This[75];
    if (type > 0 && type < 5 && player->m_StateMachine.GetCurrentState()->GetStateName() == "ReactionJump")
    {
        Common::ObjectCGlitterPlayerOneShot(This, "ef_cmn_reaction");
    }
    originalQTEReactionPlate_MsgHitEventCollision(This, Edx, message);
}

void QTEReactionPlate_ReactionJumpPlaySfx()
{
    // TODO: not Wii config
    //Sonic::Player::CPlayerSpeedContext::GetInstance()->PlaySound(4002047, 0);
}

void QTEReactionPlate_ReactionJumpSetAnim()
{
    Sonic::Player::CPlayerSpeedContext::GetInstance()->ChangeAnimation("ReactionJumpU");
}

void __declspec(naked) QTEReactionPlate_ReactionJumpPlaySfxTrampoline()
{
    __asm
    {
        call    QTEReactionPlate_ReactionJumpPlaySfx
        pop     edi
        pop     esi
        pop     ebx
        mov     esp, ebp
        pop     ebp
        retn    0x1C
    }
}

void __declspec(naked) QTEReactionPlate_ReactionJumpSetAnimTrampoline()
{
    __asm
    {
        call    QTEReactionPlate_ReactionJumpSetAnim
        pop     edi
        pop     esi
        pop     ebx
        mov     esp, ebp
        pop     ebp
        retn
    }
}

void QTEReactionPlate::applyPatches()
{
    // set correct models
    WRITE_MEMORY(0xEBA0DB, char*, "ReactionPlate");
    WRITE_MEMORY(0x1A47288, char*,
        "cmn_obj_kt3_springblueM_000",
        "cmn_obj_km_BreactionpanelM_000",
        "cmn_obj_km_AreactionpanelM_000",
        "cmn_obj_km_XreactionpanelM_000",
        "cmn_obj_km_YreactionpanelM_000",
        "RP_ButtonE",
        "sea_obj_km_reactionpanelQ_000");

    // main hooks
    INSTALL_HOOK(QTEReactionPlate_CPlayerSpeedProcessMessage);
    INSTALL_HOOK(QTEReactionPlate_PlayUIEffect);
    INSTALL_HOOK(QTEReactionPlate_CPlayerSpeedStateReactionJumpAdvance);
    INSTALL_HOOK(QTEReactionPlate_CPlayerSpeedStateReactionLandEnd);
    INSTALL_HOOK(QTEReactionPlate_MsgHitEventCollision);
    WRITE_JUMP(0xE5D03E, QTEReactionPlate_ReactionJumpPlaySfxTrampoline);
    WRITE_JUMP(0xE5CDB3, QTEReactionPlate_ReactionJumpPlaySfxTrampoline);
    WRITE_JUMP(0x124B915, QTEReactionPlate_ReactionJumpSetAnimTrampoline);

    // fix animations
    WRITE_MEMORY(0x1274C03 + 6, char*, "sn_wall_fly00_loop");
    WRITE_MEMORY(0x1274C6A + 6, char*, "sn_wall_fly00_loop");

    // use custom xncp
    WRITE_STRING(0x1604AEC, "ui_qte_swa");

    // use stumble voice instead of damage02
    WRITE_MEMORY(0xE5CBF9, uint32_t, 3002002);
}
