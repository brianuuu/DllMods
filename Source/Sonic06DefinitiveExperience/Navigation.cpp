#include "Navigation.h"

HOOK(void, __fastcall, MsgStartCommonButtonSign, 0x5289A0, void* This, void* Edx, uint32_t a2)
{
    // Disable Y button prompt
    uint32_t buttonType = *(uint32_t*)(a2 + 16);
    if (buttonType == 3)
    {
        return;
    }

    originalMsgStartCommonButtonSign(This, Edx, a2);
}

void Navigation::applyPatches()
{
    INSTALL_HOOK(MsgStartCommonButtonSign);
}
