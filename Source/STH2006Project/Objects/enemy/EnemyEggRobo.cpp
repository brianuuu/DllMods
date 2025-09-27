#include "EnemyEggRobo.h"

HOOK(int*, __fastcall, EnemyEggRobo_SpawnBrk, 0xBAAEC0, uint32_t This)
{
    int* result = originalEnemyEggRobo_SpawnBrk(This);

    static Hedgehog::Base::CSharedString enm_eggroboA_brk = "enm_eggroboA_brk";
    static Hedgehog::Base::CSharedString enm_eggroboB_brk = "enm_eggroboB_brk";
    static Hedgehog::Base::CSharedString cmn_eggroboA_brk_ = "cmn_eggroboA_brk_";
    static Hedgehog::Base::CSharedString cmn_eggroboB_brk_ = "cmn_eggroboB_brk_";
    if (*(bool*)(This + 416))
    {
        WRITE_MEMORY(0x1E77828, char*, enm_eggroboB_brk.get());
        WRITE_MEMORY(0x1E7782C, char*, cmn_eggroboB_brk_.get());
    }
    else
    {
        WRITE_MEMORY(0x1E77828, char*, enm_eggroboA_brk.get());
        WRITE_MEMORY(0x1E7782C, char*, cmn_eggroboA_brk_.get());
    }

    return result;
}

void __declspec(naked) EnemyEggRobo_TrackBeam()
{
    static uint32_t returnAddress = 0x601CF0;
    __asm
    {
        // original
        movss   [esp + 0xB4], xmm0

        // TStateGetContext
        mov     ecx, ebx
        mov     eax, [ecx + 8]

        // use missile track values
        movss   xmm0, dword ptr [eax + 0x3E0] // homing velocity
        movss   [esp + 0xA8], xmm0
        movss   xmm0, dword ptr [eax + 0x3E4] // homing start time
        movss   [esp + 0xAC], xmm0
        movss   xmm0, ds:0x1A40984 // 1.5 homing time
        movss   [esp + 0xB0], xmm0
        movss   xmm0, dword ptr [eax + 0x3EC] // life time
        movss   dword ptr [esp + 0xB8], xmm0

        jmp     [returnAddress]
    }
}

void EnemyEggRobo::applyPatches()
{
    // Fix EggRoboB using wrong brk object
    INSTALL_HOOK(EnemyEggRobo_SpawnBrk);

    // Change EggRoboA beam data 
    WRITE_MEMORY(0x601C5C, uint32_t, 0x1A416CC); // 0.5 length
    WRITE_MEMORY(0x601CBF, uint32_t, 0x1A3F16C); // 12 shot velocity
    WRITE_MEMORY(0x601CD5, uint32_t, 0x156A23C); // 10 life time

    // Set EggRoboA beam to use EggRoboB missile track values
    WRITE_JUMP(0x601CE7, (void*)EnemyEggRobo_TrackBeam);
    WRITE_JUMP(0x601E54, (void*)0x601E7E);

    // fix chaos energy amount
    WRITE_MEMORY(0x16F7C9C + 0xC8, void*, AddCallback);
}

void EnemyEggRobo::AddCallback
(
    EnemyEggRobo* This, void*, void*
)
{
    // modify chaos energy amount
    if (!This->m_isStinger)
    {
        This->m_energyAmount = 2;
    }
}
