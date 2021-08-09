#include "Stage.h"
#include "Application.h"

HOOK(int32_t*, __fastcall, Stage_MsgHitGrindPath, 0xE25680, void* This, void* Edx, uint32_t a2)
{
    // If at Kingdom Valley, change sfx to wind
    // There are normal rails too, when x > 210
    Eigen::Vector3f playerPosition;
    Eigen::Quaternionf playerRotation;
    if (Common::CheckCurrentStage("euc200") 
     && Common::GetPlayerTransform(playerPosition, playerRotation)
     && playerPosition.x() < 210.0f)
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

    return originalStage_MsgHitGrindPath(This, Edx, a2);
}

HOOK(int, __fastcall, Stage_CObjSpringSFX, 0x1038DA0, void* This)
{
    // If at Kingdom Valley, change sfx to rope
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

    return originalStage_CObjSpringSFX(This);
}

bool Stage::m_wallJumpStart = false;
float Stage::m_wallJumpTime = 0.0f;
float const c_wallJumpSpinTime = 0.6f;
HOOK(bool, __fastcall, Stage_CSonicStateFallAdvance, 0x1118C50, void* This)
{
    bool result = originalStage_CSonicStateFallAdvance(This);

    if (Stage::m_wallJumpStart)
    {
        Common::SonicContextChangeAnimation("SpinAttack");
        Stage::m_wallJumpStart = false;
        Stage::m_wallJumpTime = c_wallJumpSpinTime;
    }
    else if (Stage::m_wallJumpTime > 0.0f)
    {
        Stage::m_wallJumpTime -= Application::getDeltaTime();
        if (Stage::m_wallJumpTime <= 0.0f)
        {
            Common::SonicContextChangeAnimation("FallFast");
        }
    }

    return result;
}

HOOK(int, __fastcall, Stage_CSonicStateFallEnd, 0x1118F20, void* This)
{
    Stage::m_wallJumpStart = false;
    Stage::m_wallJumpTime = 0.0f;
    return originalStage_CSonicStateFallEnd(This);
}

uint32_t getIsWallJumpReturnAddress = 0xE6D5AF;
void __declspec(naked) getIsWallJump()
{
    __asm
    {
        push    esi
        push    ebx
        lea     ecx, [esi + 30h]
        call    Stage::getIsWallJumpImpl
        pop     ebx
        pop     esi

        push    [0x15F4FE8] // Walk
        jmp     [getIsWallJumpReturnAddress]
    }
}

void Stage::applyPatches()
{
    // Play robe sfx in Kingdom Valley
    INSTALL_HOOK(Stage_CObjSpringSFX);

    // Play wind rail sfx for Kingdom Valley
    INSTALL_HOOK(Stage_MsgHitGrindPath);

    // Do SpinAttack animation for walljumps (required negative out of control time)
    WRITE_JUMP(0xE6D5AA, getIsWallJump);
    INSTALL_HOOK(Stage_CSonicStateFallAdvance);
    INSTALL_HOOK(Stage_CSonicStateFallEnd);
}

void __fastcall Stage::getIsWallJumpImpl(float* outOfControl)
{
    if (*outOfControl < 0.0f)
    {
        m_wallJumpStart = true;
        *outOfControl = -*outOfControl;
    }
    else
    {
        m_wallJumpStart = false;
    }
}
