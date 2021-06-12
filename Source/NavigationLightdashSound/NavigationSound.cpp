#include "NavigationSound.h"
#include "Configuration.h"

bool NavigationSound::m_playedSoundThisFrame = false;
float NavigationSound::m_lightdashTimer = 0.0f;
uint32_t NavigationSound::m_esp = 0;

// This may need to be updated if PlayNavigationHintSound() is changed...
// Check ESP at 0x528A2B
#if _DEBUG
uint32_t const LightDashStackPtr = 0x19F898;
#else
uint32_t const LightDashStackPtr = 0x19FA68;
#endif

void PlayNavigationHintSound()
{
    if (NavigationSound::m_playedSoundThisFrame)
    {
        printf("Navigation Sound already played this frame, ignoring...\n");
        return;
    }

    if (NavigationSound::m_esp == LightDashStackPtr)
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

HOOK(void*, __fastcall, UpdateApplication, 0xE7BED0, void* This, void* Edx, float elapsedTime, uint8_t a3)
{
    if (NavigationSound::m_lightdashTimer > 0.0f)
    {
        NavigationSound::m_lightdashTimer -= elapsedTime;
    }
    return originalUpdateApplication(This, Edx, elapsedTime, a3);
}

#define NAVIGATION_HINT_SOUND_ASM(hintName, returnAddress) \
    uint32_t const hintName##ReturnAddress = returnAddress; \
    void __declspec(naked) hintName##PlaySound() \
    { \
        __asm \
        { \
            __asm mov     NavigationSound::m_esp, esp \
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
    INSTALL_HOOK(UpdateApplication);

    WRITE_JUMP_NAVIGATION(0x5287AE, boost);
    WRITE_JUMP_NAVIGATION(0x5288DE, quickstep);
    WRITE_JUMP_NAVIGATION(0x528A2B, common);
    WRITE_JUMP_NAVIGATION(0x528B8E, direction);
    WRITE_JUMP_NAVIGATION(0x528CCE, slide);
    WRITE_JUMP_NAVIGATION(0x528DFE, leftright);
}
