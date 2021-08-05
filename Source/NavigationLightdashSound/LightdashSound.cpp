#include "LightdashSound.h"

float LightdashSound::m_lightdashSoundTimer = 0.0f;

HOOK(void, __fastcall, CLightDashUpdate, 0xDF6320, void* This)
{
    if (LightdashSound::m_lightdashSoundTimer <= 0.0f)
    {
        LightdashSound::m_lightdashSoundTimer = LightdashSound::m_lightdashSoundLength;

        static SharedPtrTypeless soundHandle;
        if (soundHandle)
        {
            soundHandle.reset();
        }
        PlaySound(GetCurrentSonicContext(), soundHandle, 3000812985);
        printf("Played LightDash Sound!\n");
    }
    else
    {
        printf("%.4f\n", LightdashSound::m_lightdashSoundTimer);
    }

    return originalCLightDashUpdate(This);
}

HOOK(void, __fastcall, LightDashSound_CSonicUpdate, 0xE6BF20, void* This, void* Edx, float* dt)
{
    LightdashSound::m_lightdashSoundTimer = max(0.0f, LightdashSound::m_lightdashSoundTimer - *dt);
    originalLightDashSound_CSonicUpdate(This, Edx, dt);
}

void LightdashSound::applyPatches()
{
    INSTALL_HOOK(LightDashSound_CSonicUpdate);
    INSTALL_HOOK(CLightDashUpdate);
}