#include "Configuration.h"
#include "AnimationSetPatcher.h"

float const c_swimUpMaxVelocity = 4.0f;
float const c_flyUpMaxVelocity = 3.0f;
float const c_flyUpAcceleration = 20.0f;
float const c_flyUpMaxTime = 8.0f;
float const c_tapMaxTime = 0.5f;
float const c_flightGravity = 8.0f;

bool m_isFlight = false;
bool m_isTired = false;
bool m_isInWater = false;
bool m_isCollisionEnabled = false;
float m_flightTime = 0.0f;
float m_tapTime = 0.0f;

HOOK(bool, __fastcall, CSonicClassicStateJumpBallBegin, 0x1114F30, hh::fnd::CStateMachineBase::CStateBase* This)
{
    m_isFlight = false;
    m_isTired = false;
    m_isCollisionEnabled = true;
    m_flightTime = 0.0f;

    return originalCSonicClassicStateJumpBallBegin(This);
}

SharedPtrTypeless flySfxHandle;
HOOK(void, __fastcall, CSonicClassicStateJumpBallAdvance, 0x1114FD0, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    
    // handle tapping A
    Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
    if (padState->IsTapped(Sonic::EKeyState::eKeyState_A))
    {
        if (!m_isFlight)
        {
            if (context->StateFlag(eStateFlag_OnWater))
            {
                m_isInWater = true;
                context->ChangeAnimation(AnimationSetPatcher::SwimLoop);
            }
            else
            {
                m_isInWater = false;
                context->ChangeAnimation(AnimationSetPatcher::FlyLoop);

                context->PlaySound(80041021, 1);
                flySfxHandle = context->PlaySound(80041022, 1);
            }

            context->StateFlag(eStateFlag_UpdateYawOnAir)++;
            m_isFlight = true;

            // set low gravity
            if (!m_isInWater)
            {
                context->m_spParameter->m_scpNode->m_ValueMap[Sonic::Player::ePlayerSpeedParameter_Gravity] = c_flightGravity;
            }
        }
        else
        {
            // only register 2nd tap for flying up
            m_tapTime = c_tapMaxTime;
        }
    }

    if (m_isFlight)
    {
        // flight time
        m_flightTime += This->m_pStateMachine->m_UpdateInfo.DeltaTime;

        // enable collision when going up
        Eigen::Vector3f velocity;
        Common::GetPlayerVelocity(velocity);
        if (velocity.y() > 0.0f ^ m_isCollisionEnabled)
        {
            m_isCollisionEnabled = !m_isCollisionEnabled;
            Common::SonicContextSetCollision(SonicCollision::TypeSonicSpin, m_isCollisionEnabled);
        }

        if (m_flightTime > c_flyUpMaxTime && !m_isTired)
        {
            m_isTired = true;
            if (context->StateFlag(eStateFlag_OnWater))
            {
                m_isInWater = true;
                context->ChangeAnimation(AnimationSetPatcher::SwimTired);
            }
            else
            {
                m_isInWater = false;
                context->ChangeAnimation(AnimationSetPatcher::FlyTired);

                flySfxHandle.reset();
                context->PlaySound(80041023, 1);
            }
        }

        // Switch between water & air
        if (!m_isInWater && context->StateFlag(eStateFlag_OnWater))
        {
            m_isInWater = true;
            context->ChangeAnimation(m_isTired ? AnimationSetPatcher::SwimTiredLoop : AnimationSetPatcher::SwimLoop);

            // low gravity in air
            context->m_spParameter->m_scpNode->m_ValueMap.erase(Sonic::Player::ePlayerSpeedParameter_Gravity);

            flySfxHandle.reset();

        }
        else if (m_isInWater && !context->StateFlag(eStateFlag_OnWater))
        {
            m_isInWater = false;
            context->ChangeAnimation(m_isTired ? AnimationSetPatcher::FlyTiredLoop : AnimationSetPatcher::FlyLoop);

            // use same gravity in water
            context->m_spParameter->m_scpNode->m_ValueMap[Sonic::Player::ePlayerSpeedParameter_Gravity] = c_flightGravity;

            if (!m_isTired)
            {
                flySfxHandle = context->PlaySound(80041022, 1);
            }
        }
    }

    originalCSonicClassicStateJumpBallAdvance(This);
}

HOOK(bool, __fastcall, CSonicClassicStateJumpBallEnd, 0x1114F00, hh::fnd::CStateMachineBase::CStateBase* This)
{
    auto* context = (Sonic::Player::CPlayerSpeedContext*)This->GetContextBase();
    if (m_isFlight)
    {
        context->StateFlag(eStateFlag_UpdateYawOnAir)--;
    }

    m_isFlight = false;
    m_isCollisionEnabled = false;
    flySfxHandle.reset();

    // reset gravity
    if (!m_isInWater)
    {
        context->m_spParameter->m_scpNode->m_ValueMap.erase(Sonic::Player::ePlayerSpeedParameter_Gravity);
    }

    return originalCSonicClassicStateJumpBallEnd(This);
}

HOOK(void, __fastcall, CPlayerSpeedPosture2DAir_CStateNormalAdvance, 0xE3CD50, hh::fnd::CStateMachineBase::CStateBase* This)
{
    originalCPlayerSpeedPosture2DAir_CStateNormalAdvance(This);

    if (m_isFlight && !m_isTired)
    {
        Eigen::Vector3f velocity;
        Common::GetPlayerVelocity(velocity);

        if (m_tapTime > 0.0f)
        {
            float const maxUpvelocity = m_isInWater ? c_swimUpMaxVelocity : c_flyUpMaxVelocity;
            float const dt = This->m_pStateMachine->m_UpdateInfo.DeltaTime;
            m_tapTime = max(m_tapTime - dt, 0.0f);

            if (velocity.y() < maxUpvelocity)
            {
                velocity.y() = min(maxUpvelocity, velocity.y() + c_flyUpAcceleration * dt);
                Common::SetPlayerVelocity(velocity);

                if (m_tapTime == 0.0f && velocity.y() < maxUpvelocity)
                {
                    // keep running until velocity is reached
                    m_tapTime = 0.001f;
                }
            }
        }
    }
}

void __declspec(naked) DisableFallAtFlight()
{
    static uint32_t returnAddress = 0x123558F;
    static uint32_t skipAddress = 0x1235612;
    __asm
    {
        cmp     m_isFlight, 0
        jne     jump
        cmp     byte ptr[ebx + 6Ch], 0
        jz      jump
        jmp     [returnAddress]

        jump:
        jmp     [skipAddress]
    }
}

extern "C" __declspec(dllexport) void Init(ModInfo_t * modInfo)
{
    std::string dir = modInfo->CurrentMod->Path;

    size_t pos = dir.find_last_of("\\/");
    if (pos != std::string::npos)
    {
        dir.erase(pos + 1);
    }

    if (!Configuration::load(dir))
    {
        MessageBox(NULL, L"Failed to parse mod.ini", NULL, MB_ICONERROR);
    }

    AnimationSetPatcher::applyPatches();

    INSTALL_HOOK(CSonicClassicStateJumpBallBegin);
    INSTALL_HOOK(CSonicClassicStateJumpBallAdvance);
    INSTALL_HOOK(CSonicClassicStateJumpBallEnd);
    INSTALL_HOOK(CPlayerSpeedPosture2DAir_CStateNormalAdvance);

    //WRITE_JUMP(0x1235585, DisableFallAtFlight);

    // Always disable JumpBall->Fall transition
    WRITE_JUMP(0x123557F, (void*)0x1235612);
}

extern "C" __declspec(dllexport) void PostInit(ModInfo_t * modInfo)
{
	
}

extern "C" void __declspec(dllexport) OnFrame()
{

}
