#include "Configuration.h"
#include "ArchiveTreePatcher.h"
#include "NavigationSound.h"
#include "LightdashSound.h"

#include <Hedgehog.h>

bool m_changedToQS = false;
HOOK(bool, __fastcall, NavigationMessages, 0x11BC880, void** This, void* Edx, uint32_t* a2, uint32_t* a3)
{
    // Change Stick Move into Quick Step Bumpers
    if (a2[0] == 0x16B0E00) // MsgStartCommonButtonSign
    {
        // Stick Move: a2[4] == 0xFFFFFFFF, a2[5] = 0xA, a2[6] = type (2,1,0) why the fuck is the type flipped
        if (a2[4] == 0xFFFFFFFF && a2[5] == 0xA) // Not drift & Stick Move
        {
            // QS: a2[4] = 0, a2[8] = 0, a2[12] = type (0,1,2)
            a2[0] = 0x16AA0C8; // MsgStartQuickStepSign
            a2[4] = 0;
            a2[8] = 0;
            a2[12] = 2 - a2[6];

            m_changedToQS = true;
        }
    }

    return originalNavigationMessages(This, Edx, a2, a3);
}

HOOK(void, __fastcall, MsgEndCommonButtonSign, 0xE68870, void* This, void* Edx, uint32_t* a2)
{
    if (m_changedToQS)
    {
        // Call for Quick Step Bumpers
        a2[0] = 0x16AA0D8; // MsgEndQuickStepSign
        originalMsgEndCommonButtonSign(This, Edx, a2);

        m_changedToQS = false;
    }
    else
    {
        // Call original for common button
        originalMsgEndCommonButtonSign(This, Edx, a2);
    }
}

static void* const pCGlitterCreate = (void*)0xE73890;
static void* fCGlitterCreate
(
    void* pContext,
    SharedPtrTypeless& handle,
    void* pMatrixTransformNode,
    Hedgehog::Base::CSharedString const* name,
    uint32_t flag
)
{
    __asm
    {
        push    flag
        push    name
        push    pMatrixTransformNode
        mov     eax, pContext
        mov     esi, handle
        call[pCGlitterCreate]
    }
}

static void* const pCGlitterEnd = (void*)0xE72650;
static void* const pCGlitterKill = (void*)0xE72570;
static void fCGlitterEnd
(
    void* pContext,
    SharedPtrTypeless& handle,
    bool instantStop
)
{
    __asm
    {
        mov     eax, [handle]
        mov     ebx, [eax + 4]
        push    ebx
        mov     ebx, [eax]
        push    ebx
        mov     eax, pContext
        cmp     instantStop, 0
        jnz     jump
        call[pCGlitterEnd]
        jmp     end

        jump :
        call[pCGlitterKill]

            end :
    }
}

SharedPtrTypeless lightDashGlitterHandle;

void playLightDashGlitter()
{
    void* context = GetCurrentSonicContext();
    void* matrixNode = (void*)((uint32_t)context + 0x30);
    fCGlitterCreate(context, lightDashGlitterHandle, matrixNode, (Hedgehog::Base::CSharedString const*)0x1E61DC4, 1);
}

void stopLightDashGlitter()
{
    void* context = GetCurrentSonicContext();
    fCGlitterEnd(context, lightDashGlitterHandle, true);
}

uint32_t CSonicStateLightSpeedDashBeginReturnAddress = 0x12318AA;
void __declspec(naked) CSonicStateLightSpeedDashBegin()
{
    __asm
    {
        push    eax
        call    playLightDashGlitter
        pop     eax
        push    [0x15E6AA4] // LightSpeedDash
        jmp     [CSonicStateLightSpeedDashBeginReturnAddress]
    }
}

uint32_t CSonicStateLightSpeedDashEndReturnAddress = 0x12319E7;
void __declspec(naked) CSonicStateLightSpeedDashEnd()
{
    __asm
    {
        push    eax
        call    stopLightDashGlitter
        pop     eax
        push    [0x15E613C] // LightSpeedDash
        jmp     [CSonicStateLightSpeedDashEndReturnAddress]
    }
}

extern "C" __declspec(dllexport) void Init(ModInfo * modInfo)
{
    std::string dir = modInfo->CurrentMod->Path;

    size_t pos = dir.find_last_of("\\/");
    if (pos != std::string::npos)
    {
        dir.erase(pos + 1);
    }
    
    if (!Configuration::load(dir))
    {
        MessageBox(NULL, L"Failed to parse Config.ini", NULL, MB_ICONERROR);
    }

    ArchiveTreePatcher::applyPatches();

    if (Configuration::m_enableLightdashSound)
    {
        LightdashSound::applyPatches();
    }

    if (Configuration::m_enableLightdashEffect)
    {
        WRITE_JUMP(0x12318A5, CSonicStateLightSpeedDashBegin);
        WRITE_JUMP(0x12319E2, CSonicStateLightSpeedDashEnd);
    }

    if (Configuration::m_enableLightdashRings)
    {
        // Patch "All Rings Can Be Light Dashed" by "Skyth"
        WRITE_MEMORY(0x105334D, uint32_t, 0x10C47C6);
        WRITE_NOP(0x1053351, 16);

        // Patch "Disable Light Dash Particles" by "Hyper"
        WRITE_MEMORY(0x10538EB, uint8_t, 0xE9, 0x8F, 0x00, 0x00, 0x00, 0x90);
    }

    if (Configuration::m_enableNavigationSound)
    {
        NavigationSound::applyPatches();
    }

    if (Configuration::m_enableBumperRailSwitch)
    {
        INSTALL_HOOK(NavigationMessages);
        INSTALL_HOOK(MsgEndCommonButtonSign);

        // Patch "Use Bumpers to Switch Grind Rails" by "Skyth"
        WRITE_MEMORY(0xDFCC92, uint32_t, 0x10244C8B);
        WRITE_NOP(0xDFCC96, 2);
        WRITE_MEMORY(0xDFCC99, uint16_t, 0xE1BA);
        WRITE_MEMORY(0xDFCC9B, uint8_t, 0xD);
        WRITE_NOP(0xDFCC9C, 3);
        WRITE_MEMORY(0xDFCC9F, uint8_t, 0x73);
        WRITE_MEMORY(0xDFCCA8, uint32_t, 0xCE1BA0F);
        WRITE_NOP(0xDFCCAC, 7);
        WRITE_MEMORY(0xDFCCB3, uint8_t, 0x73);
    }
}

extern "C" void __declspec(dllexport) OnFrame()
{
    if (Configuration::m_enableNavigationSound)
    {
        NavigationSound::update();
    }
}