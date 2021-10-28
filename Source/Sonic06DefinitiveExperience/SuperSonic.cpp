#include "SuperSonic.h"
#include "Configuration.h"

HOOK(void, __fastcall, SuperSonic_CSonicSetMaxSpeedBasis, 0xDFBCA0, int* This)
{
    originalSuperSonic_CSonicSetMaxSpeedBasis(This);

    if (*pModernSonicContext && Common::IsPlayerSuper())
    {
        *Common::GetPlayerMaxSpeed() = 70.0f;
    }
}

void SuperSonic::applyPatches()
{
    // Revert Super Sonic speed for 06 physics
    if (Configuration::m_physics)
    {
        INSTALL_HOOK(SuperSonic_CSonicSetMaxSpeedBasis);
        WRITE_NOP(0xDFE117, 1);
    }

    // Patch "Disable Super Sonic Floating Boost (Ground Boost)" by "Skyth"
    WRITE_MEMORY(0xDFF34C, uint8_t, 0xEB);

    // Patch "Closer Super Sonic FOV" by "brianuuu"
    WRITE_MEMORY(0x11D9F32, uint8_t, 0xC7, 0x80, 0x70, 0x01, 0x00, 0x00);
    WRITE_MEMORY(0x11D9F38, float, 1.308996916f);
    WRITE_NOP(0x11D9F3C, 2);
    WRITE_MEMORY(0x11D9EFB, uint8_t, 0x8B, 0x16, 0xC7, 0x82, 0x6C, 0x01, 0x00, 0x00);
    WRITE_MEMORY(0x11D9F03, float, 0.8f);
    WRITE_NOP(0x11D9F07, 8);
    WRITE_MEMORY(0x10E7B96, uint8_t, 0xEB, 0x0A);
}
