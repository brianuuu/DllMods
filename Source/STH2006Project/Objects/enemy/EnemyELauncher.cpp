#include "EnemyELauncher.h"

void __declspec(naked) EnemyELauncher_HideMissile()
{
    static uint32_t returnAddress = 0xB81491;
    __asm
    {
        mov     dword ptr [esp + 184h], 0xC61C4000 // -10000y
        movss   [esp + 190h], xmm0 // original
        jmp     [returnAddress]
    }
}


void EnemyELauncher::applyPatches()
{	
    // Hide ELauncher missile respawn
    WRITE_JUMP(0xB81488, (void*)EnemyELauncher_HideMissile);

    // ELauncher
    WRITE_MEMORY(0xB820C0, uint32_t, 0x1574644); // rigidbody radius -> 1.0
    WRITE_MEMORY(0xB8207F, uint32_t, 0x156460C); // attached rigidbody to Spine

	// fix chaos energy amount
    WRITE_MEMORY(0x16FB1FC + 0xC8, void*, AddCallback);
}

void __fastcall EnemyELauncher::AddCallback
(
	EnemyELauncher* This, void*, void*
)
{
	// modify chaos energy amount
	This->m_energyAmount = 3;
}
