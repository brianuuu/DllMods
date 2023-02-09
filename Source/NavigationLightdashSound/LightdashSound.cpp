#include "LightdashSound.h"
#include "Configuration.h"

float LightdashSound::m_lightdashSoundTimer = 0.0f;
SharedPtrTypeless lightDashGlitterHandle;

HOOK(int, __fastcall, LightDashSound_CSonicStateLightSpeedDashBegin, 0x1231890, hh::fnd::CStateMachineBase::CStateBase* This)
{
    LightdashSound::m_lightdashSoundTimer = 0.0f;

    if (Configuration::m_enableLightdashEffect)
    {
        void* context = This->GetContextBase();
        void* matrixNode = (void*)((uint32_t)context + 0x30);
        Common::fCGlitterCreate(context, lightDashGlitterHandle, matrixNode, *(Hedgehog::Base::CSharedString*)0x1E61DC4, 1);
    }

    return originalLightDashSound_CSonicStateLightSpeedDashBegin(This);
}

HOOK(void, __fastcall, LightDashSound_CSonicStateLightSpeedDashAdvance, 0x1231810, hh::fnd::CStateMachineBase::CStateBase* This)
{
    if (Configuration::m_enableLightdashSound)
    {
        LightdashSound::m_lightdashSoundTimer = max(0.0f, LightdashSound::m_lightdashSoundTimer - This->m_pStateMachine->m_UpdateInfo.DeltaTime);
        if (LightdashSound::m_lightdashSoundTimer <= 0.0f)
        {
            LightdashSound::m_lightdashSoundTimer = LightdashSound::m_lightdashSoundLength;

            static SharedPtrTypeless soundHandle;
            Common::SonicContextPlaySound(soundHandle, 3000812985, 0);
        }
    }

    originalLightDashSound_CSonicStateLightSpeedDashAdvance(This);
}

HOOK(int, __fastcall, LightDashSound_CSonicStateLightSpeedDashEnd, 0x12319D0, hh::fnd::CStateMachineBase::CStateBase* This)
{
    if (Configuration::m_enableLightdashEffect)
    {
        Common::fCGlitterEnd(This->GetContextBase(), lightDashGlitterHandle, true);
    }
    return originalLightDashSound_CSonicStateLightSpeedDashEnd(This);
}

void LightdashSound::applyPatches()
{
    INSTALL_HOOK(LightDashSound_CSonicStateLightSpeedDashBegin);
    INSTALL_HOOK(LightDashSound_CSonicStateLightSpeedDashAdvance);
    INSTALL_HOOK(LightDashSound_CSonicStateLightSpeedDashEnd);
}