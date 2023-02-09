#include "QTEReactionPlate.h"

float const c_qteReactionMaxScore = 1500.0f;
float const c_qteReactionMinScore = 1000.0f;

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

void ProcMsgHitReactionPlate(Sonic::Player::CPlayerSpeed* This, const Sonic::Message::MsgHitReactionPlate& message)
{
    const float minVelocity = message.m_JumpMinVelocity <= 0 ? 40 : message.m_JumpMinVelocity;
    const float maxVelocity = message.m_JumpMaxVelocity <= 0 ? 60 : message.m_JumpMaxVelocity;

    if (message.m_Type != 0) // 0 == Begin
    {
        if (message.m_Type != 5 && This->m_StateMachine.GetCurrentState()->GetStateName() == "ReactionJump") // 5 == End
        {
            const auto pState = This->GetContext()->ChangeState<Sonic::Player::CPlayerSpeedStateReactionLand>();
            pState->m_TargetActorID = message.m_TargetActorID;
            pState->m_JumpMinVelocity = minVelocity;
            pState->m_JumpMaxVelocity = maxVelocity;

            hh::math::CVector targetPosition;
            This->SendMessageImm(message.m_TargetActorID, boost::make_shared<Sonic::Message::MsgGetPosition>(targetPosition));

            const auto rightDirection = message.m_Direction.cross(hh::math::CVector::UnitY()).normalized();
            const auto targetDirection = (targetPosition - message.m_Position).normalized();
            const bool facesLeft = rightDirection.dot(targetDirection) < 0;

            This->GetContext()->ChangeAnimation(facesLeft ? "ReactionLandL" : "ReactionLandR");
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
            This->GetContext()->m_ModelUpDirection = (rotation * hh::math::CVector::UnitY()).normalized();

            This->GetContext()->m_spMatrixNode->m_Transform.SetRotationAndPosition(rotation * This->GetContext()->m_HorizontalRotation, message.m_Position + message.m_Direction * 0.25f);
            This->GetContext()->m_spMatrixNode->NotifyChanged();
        }
    }
    else if (This->m_StateMachine.GetCurrentState()->GetStateName() != "ReactionLand")
    {
        fE5CD90
        (
            message.m_TargetActorID,
            This->m_spContext.get(),
            &message.m_Position,
            Sonic::Player::CPlayerSpeedStateReactionLand::eAnimationType_JumpSpring,
            minVelocity,
            maxVelocity,
            1000,
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

int m_qteReactionPlateScore = 0;
HOOK(bool, __stdcall, QTEReactionPlate_PlayUIEffect, 0xE6F3E0, float* This, void* a2)
{
    bool result = originalQTEReactionPlate_PlayUIEffect(This, a2);
    if (result)
    {
        float inputTime = This[12];
        float maxTime = This[9];
        m_qteReactionPlateScore = max(0, (int)(c_qteReactionMinScore + (c_qteReactionMaxScore - c_qteReactionMinScore) * (1.0f - inputTime / maxTime)));
    }

    return result;
}

HOOK(int, __fastcall, QTEReactionPlate_CPlayerSpeedStateReactionLandEnd, 0x124B7D0, hh::fnd::CStateMachineBase::CStateBase* This)
{
    uint32_t context = (uint32_t)This->GetContextBase();
    uint32_t v3 = *(uint32_t*)(context + 2036);
    bool success = *(bool*)(v3 + 57);
    if (m_qteReactionPlateScore > 0 && success)
    {
        ScoreGenerationsAPI::AddScore(m_qteReactionPlateScore);
        UnleashedHUD_API::AddTrickScore(m_qteReactionPlateScore);
    }
    m_qteReactionPlateScore = 0;

    return originalQTEReactionPlate_CPlayerSpeedStateReactionLandEnd(This);
}

void QTEReactionPlate_ReactionJumpPlaySfx()
{
    Sonic::Player::CPlayerSpeedContext::GetInstance()->PlaySound(4002047, 0);
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
    INSTALL_HOOK(QTEReactionPlate_CPlayerSpeedStateReactionLandEnd);
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
