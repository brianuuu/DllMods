#include "Configuration.h"
#include "ArchiveTreePatcher.h"
#include "NavigationSound.h"
#include "LightdashSound.h"

#include <Hedgehog.h>

HOOK(bool, __fastcall, NavigationMessages, 0x11BC880, void** This, void* Edx, uint32_t* a2, uint32_t* a3)
{
    // Chamge Stick Move into Quick Step Bumpers
    if (a2[0] == 0x16B0E00 && a2[5] == 0xA) // MsgStartCommonButtonSign && Stick Move
    {
        // Stick Move: a2[5] = 0xA, a2[6] = type (2,1,0) why the fuck is the type flipped
        // QS: a2[4] = 0, a2[8] = 0, a2[12] = type (0,1,2)
        a2[0] = 0x16AA0C8; // MsgStartQuickStepSign
        a2[4] = 0;
        a2[8] = 0;
        a2[12] = 2 - a2[6];
    }

    return originalNavigationMessages(This, Edx, a2, a3);
}

HOOK(void, __fastcall, MsgEndCommonButtonSign, 0xE68870, void* This, void* Edx, uint32_t* a2)
{
    // Call original for common button
    originalMsgEndCommonButtonSign(This, Edx, a2);

    // Call again for Quick Step Bumpers
    a2[0] = 0x16AA0D8; // MsgEndQuickStepSign
    originalMsgEndCommonButtonSign(This, Edx, a2);
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