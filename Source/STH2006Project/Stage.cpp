#include "Stage.h"
#include "Application.h"
#include "Configuration.h"
#include "UIContext.h"

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

bool Stage::m_waterRunning = false; 
static SharedPtrTypeless waterSoundHandle;
HOOK(char, __stdcall, Stage_CSonicStateGrounded, 0xDFF660, int* a1, bool a2)
{
    if (Common::CheckCurrentStage("ghz200"))
    {
        alignas(16) MsgGetAnimationInfo message {};
        Common::SonicContextGetAnimationInfo(message);

        CSonicStateFlags* flags = Common::GetSonicStateFlags();
        if (flags->KeepRunning && flags->OnWater)
        {
            // Initial start, play sfx
            if (!Stage::m_waterRunning)
            {
                Common::SonicContextPlaySound(waterSoundHandle, 2002059, 1);
            }

            // Change animation
            Stage::m_waterRunning = true;
            if (!message.IsAnimation("Sliding"))
            {
                Common::SonicContextChangeAnimation("Sliding");
            }
        }
        else
        {
            // Auto-run finished
            Stage::m_waterRunning = false;
            waterSoundHandle.reset();
            if (message.IsAnimation("Sliding"))
            {
                Common::SonicContextChangeAnimation("Walk");
            }
        }
    }

    return originalStage_CSonicStateGrounded(a1, a2);
}

uint32_t playWaterPfxSuccessAddress = 0x11DD1B9;
uint32_t playWaterPfxReturnAddress = 0x11DD240;
void __declspec(naked) playWaterPfx()
{
    __asm
    {
        mov     edx, [ecx + 4]

        // check boost
        cmp     byte ptr[edx + 10h], 0
        jnz     jump

        // check auto-run
        cmp     byte ptr[edx + 2Dh], 0
        jnz     jump

        jmp     [playWaterPfxReturnAddress]

        jump:
        jmp     [playWaterPfxSuccessAddress]
    }
}

HOOK(void, __stdcall, Stage_SonicChangeAnimation, 0xCDFC80, void* a1, int a2, const hh::base::CSharedString& name)
{
    if (Stage::m_waterRunning)
    {
        // if still water running, do not use walk animation (boost)
        if (strcmp(name.m_pStr, "Walk") == 0)
        {
            originalStage_SonicChangeAnimation(a1, a2, "Sliding");
            return;
        }

        alignas(16) MsgGetAnimationInfo message {};
        Common::SonicContextGetAnimationInfo(message);

        if (message.IsAnimation("Sliding"))
        {
            Stage::m_waterRunning = false;
            waterSoundHandle.reset();
        }
    }

    originalStage_SonicChangeAnimation(a1, a2, name);
}

std::string Stage::m_lapTimeStr;
float Stage::m_checkpointTimer = 0.0f;
HOOK(void, __fastcall, Stage_MsgNotifyLapTimeHud, 0x1097640, void* This, void* Edx, uint32_t a2)
{
    if (Configuration::m_using06HUD)
    {
        float lapTime = *(float*)(a2 + 20);
        int minute = (int)(lapTime / 60.0f);
        int seconds = (int)(lapTime - 60.0f * (float)minute);
        int milliseconds = (int)(lapTime * 1000.0f) % 1000;

        char buffer[20];
        sprintf(buffer, "%02d'%02d\"%03d", minute, seconds, milliseconds);

        Stage::m_lapTimeStr = std::string(buffer);
        Stage::m_checkpointTimer = 3.0f;

        return;
    }
    
    originalStage_MsgNotifyLapTimeHud(This, Edx, a2);
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

    // Do slide animation on water running in Wave Ocean
    INSTALL_HOOK(Stage_CSonicStateGrounded);
    INSTALL_HOOK(Stage_SonicChangeAnimation);
    WRITE_JUMP(0x11DD1AC, playWaterPfx);

    // For 06 HUD, do checkpoint time here so we can draw on top of itembox
    INSTALL_HOOK(Stage_MsgNotifyLapTimeHud);
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

void Stage::draw()
{
    // At loading screen, clear all
    if (Common::IsAtLoadingScreen())
    {
        m_checkpointTimer = 0.0f;
        return;
    }

    if (m_checkpointTimer > 0.0f)
    {
        static bool visible = true;
        ImGui::Begin("Checkpoint", &visible, UIContext::m_hudFlags);
        {
            ImVec2 size = ImGui::CalcTextSize(m_lapTimeStr.c_str());
            ImGui::Text(m_lapTimeStr.c_str());
            ImGui::SetWindowFocus();
            ImGui::SetWindowPos(ImVec2((float)*BACKBUFFER_WIDTH * 0.5f - size.x * 0.5f, (float)*BACKBUFFER_HEIGHT * 0.882f - size.y * 0.5f));
        }
        ImGui::End();

        m_checkpointTimer -= Application::getHudDeltaTime();
    }
}
