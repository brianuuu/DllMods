#include "ReappearingRedRing.h"

uint32_t m_isRedRingCollected = 0;

uint32_t RedRingCollectedCheckReturnAddress = 0x11A9EAF;
void __declspec(naked) RedRingCollectedCheck()
{
    __asm
    {
        mov     m_isRedRingCollected, 0
        jz      jump

        // collected
        mov     m_isRedRingCollected, 1

        jump:
        jmp     [RedRingCollectedCheckReturnAddress]
    }
}

const char* volatile const RedRingOriginalModel = "cmn_obj_spcialring_HD";
const char* volatile const RedRingTransparentModel = "cmn_obj_spcialring_HD_hide";
uint32_t RedRingModelChangeReturnAddress = 0x11A9F2C;
void __declspec(naked) RedRingModelChange()
{
    __asm
    {
        cmp     m_isRedRingCollected, 1
        je      jump

        push    RedRingOriginalModel
        jmp     [RedRingModelChangeReturnAddress]

        jump:
        push    RedRingTransparentModel
        jmp     [RedRingModelChangeReturnAddress]
    }
}

void ReappearingRedRing::applyPatches()
{
	WRITE_JUMP(0x11A9EA9, RedRingCollectedCheck);
    WRITE_JUMP(0x11A9F27, RedRingModelChange);
}
