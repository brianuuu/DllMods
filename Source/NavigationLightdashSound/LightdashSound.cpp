#include "LightdashSound.h"

float LightdashSound::m_lightdashSoundTimer = 0.0f;

HOOK(void, __fastcall, CLightDashUpdate, 0xDF6320, void* This, void* Edx)
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

    return originalCLightDashUpdate(This, Edx);
}

HOOK(void*, __fastcall, UpdateApplication2, 0xE7BED0, void* This, void* Edx, float elapsedTime, uint8_t a3)
{
    if (LightdashSound::m_lightdashSoundTimer > 0.0f)
    {
        LightdashSound::m_lightdashSoundTimer -= elapsedTime;
    }
    return originalUpdateApplication2(This, Edx, elapsedTime, a3);
}

void LightdashSound::applyPatches()
{
    INSTALL_HOOK(UpdateApplication2);
    INSTALL_HOOK(CLightDashUpdate);
}