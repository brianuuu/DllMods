#include "SuperSonic.h"

#include "Configuration.h"

uint32_t isSuperSonic = 0;
void checkIsSuperSonic()
{
    isSuperSonic = false;

    // Not Modern Sonic
    void* pModernSonicContext = *(void**)0x1E5E2F8;
    if (!pModernSonicContext) return;

    // Not in super form
    uint32_t superSonicAddress = (uint32_t)(pModernSonicContext)+0x1A0;
    if (!*(void**)superSonicAddress) return;

    isSuperSonic = true;
}

uint32_t const SuperSonicSpeedASMReturnAddress = 0xDFE118;
void __declspec(naked) SuperSonicSpeedASM()
{
    __asm
    {
        movss   [esp + 0x04], xmm0 // This will be speed from Sonic.prm.xml
        call    checkIsSuperSonic
        cmp     isSuperSonic, 0
        je      jump

        mov     dword ptr [esp + 0x04], 0x428C0000 // 70.0

        jump:
        jmp     [SuperSonicSpeedASMReturnAddress]
    }
}

void SuperSonic::applyPatches()
{
    // Revert Super Sonic speed for 06 physics
    if (Configuration::m_physics)
    {
        WRITE_JUMP(0xDFE112, SuperSonicSpeedASM);
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
