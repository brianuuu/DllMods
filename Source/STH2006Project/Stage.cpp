#include "Stage.h"

HOOK(int32_t*, __fastcall, MsgHitGrindPath, 0xE25680, void* This, void* Edx, uint32_t a2)
{
    // If at Kingdom Valley, change sfx to wind
    if (Common::CheckCurrentStage("euc200"))
    {
        WRITE_MEMORY(0xE4FC73, uint8_t, 0);
        WRITE_MEMORY(0xE4FC82, uint32_t, 4042004);
        WRITE_MEMORY(0xE4FCD4, uint8_t, 0);
        WRITE_MEMORY(0xE4FCD6, uint32_t, 4042005);
    }
    else
    {
        WRITE_MEMORY(0xE4FC73, uint8_t, 1);
        WRITE_MEMORY(0xE4FC82, uint32_t, 2002038);
        WRITE_MEMORY(0xE4FCD4, uint8_t, 1);
        WRITE_MEMORY(0xE4FCD6, uint32_t, 2002037);
    }

    return originalMsgHitGrindPath(This, Edx, a2);
}

HOOK(int, __fastcall, CObjSpringSFX, 0x1038DA0, void* This)
{
    // If at Kingdom Valley, change sfx to wind
    if (Common::CheckCurrentStage("euc200"))
    {
        WRITE_MEMORY(0x1A6B7D8, uint32_t, 8000);
        WRITE_MEMORY(0x1A6B7EC, uint32_t, 8000);
    }
    else
    {
        WRITE_MEMORY(0x1A6B7D8, uint32_t, 4001015);
        WRITE_MEMORY(0x1A6B7EC, uint32_t, 4001015);
    }

    return originalCObjSpringSFX(This);
}

void Stage::applyPatches()
{
    // Play robe sfx in Kingdom Valley
    INSTALL_HOOK(CObjSpringSFX);

    // Play wind rail sfx for Kingdom Valley
    INSTALL_HOOK(MsgHitGrindPath);
}
