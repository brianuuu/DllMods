#include "NavigationSound.h"
#include "Configuration.h"

bool NavigationSound::m_playedSoundThisFrame = false;
float NavigationSound::m_lightdashTimer = 0.0f;

HOOK(void, __fastcall, NavigationSound_CSonicUpdate, 0xE6BF20, void* This, void* Edx, float* dt)
{
    NavigationSound::m_lightdashTimer = max(0.0f, NavigationSound::m_lightdashTimer - *dt);
    originalNavigationSound_CSonicUpdate(This, Edx, dt);
}

uint32_t m_buttonType = 0;
HOOK(void, __fastcall, MsgStartCommonButtonSign, 0x5289A0, void* This, void* Edx, uint32_t a2)
{
    // Disable Y button prompt?
    m_buttonType = *(uint32_t*)(a2 + 16);
    if (m_buttonType == 3 && !Configuration::m_enableLightdashPrompt)
    {
        return;
    }

    originalMsgStartCommonButtonSign(This, Edx, a2);
}

void PlayNavigationHintSound()
{
    if (NavigationSound::m_playedSoundThisFrame)
    {
        printf("Navigation Sound already played this frame, ignoring...\n");
        return;
    }

    if (m_buttonType == 3) // Y button
    {
        if (NavigationSound::m_lightdashTimer > 0.0f)
        {
            printf("Preventing light dash prompt spam\n");
            return;
        }
        else
        {
            NavigationSound::m_lightdashTimer = NavigationSound::m_lightdashDelay;
        }
    }

    static SharedPtrTypeless soundHandle;
    if (soundHandle)
    {
        soundHandle.reset();
    }
    PlaySound(GetCurrentSonicContext(), soundHandle, Configuration::m_navigationSoundType == 0 ? 3000812983 : 3000812984);

    NavigationSound::m_playedSoundThisFrame = true;
    printf("Played Navigation Sound!\n");
}

#define NAVIGATION_HINT_SOUND_ASM(hintName, returnAddress) \
    uint32_t const hintName##ReturnAddress = returnAddress; \
    void __declspec(naked) hintName##PlaySound() \
    { \
        __asm \
        { \
            __asm mov     ecx, [ebp+8h] \
            __asm mov     ecx, [ecx+10h] \
            __asm mov     m_buttonType, ecx \
            __asm call    PlayNavigationHintSound \
            __asm lea     ecx, [esp+38h-24h] \
            __asm push    0 \
            __asm jmp     [hintName##ReturnAddress] \
        } \
    } \

NAVIGATION_HINT_SOUND_ASM(boost,        0x5287B4);
NAVIGATION_HINT_SOUND_ASM(quickstep,    0x5288E4);
NAVIGATION_HINT_SOUND_ASM(common,       0x528A31);
NAVIGATION_HINT_SOUND_ASM(direction,    0x528B94);
NAVIGATION_HINT_SOUND_ASM(slide,        0x528CD4);
NAVIGATION_HINT_SOUND_ASM(leftright,    0x528E04);

#define WRITE_JUMP_NAVIGATION(address, hintName) \
    WRITE_JUMP(address, hintName##PlaySound); \
    WRITE_NOP(address + 0x5, 1);

void NavigationSound::update()
{
    m_playedSoundThisFrame = false;
}

void NavigationSound::applyPatches()
{
    INSTALL_HOOK(NavigationSound_CSonicUpdate);
    INSTALL_HOOK(MsgStartCommonButtonSign);

    WRITE_JUMP_NAVIGATION(0x5287AE, boost);
    WRITE_JUMP_NAVIGATION(0x5288DE, quickstep);
    WRITE_JUMP_NAVIGATION(0x528A2B, common);
    WRITE_JUMP_NAVIGATION(0x528B8E, direction);
    WRITE_JUMP_NAVIGATION(0x528CCE, slide);
    WRITE_JUMP_NAVIGATION(0x528DFE, leftright);
}
