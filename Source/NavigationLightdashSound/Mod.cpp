#include "Configuration.h"
#include "ArchiveTreePatcher.h"
#include "NavigationSound.h"
#include "LightdashSound.h"

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
    LightdashSound::applyPatches();
    NavigationSound::applyPatches();

    if (Configuration::m_enableLightdashRings)
    {
        // Patch "All Rings Can Be Light Dashed" by "Skyth"
        WRITE_MEMORY(0x105334D, uint32_t, 0x10C47C6);
        WRITE_NOP(0x1053351, 16);

        // Patch "Disable Light Dash Particles" by "Hyper"
        WRITE_MEMORY(0x10538EB, uint8_t, 0xE9, 0x8F, 0x00, 0x00, 0x00, 0x90);
    }

    if (Configuration::m_enableBumperRailSwitch)
    {
        // Stick move nav to bumper nav, case 1->0
        WRITE_MEMORY(0x1224500, uint32_t, 0x1223BFA);
        WRITE_MEMORY(0x12230E4, uint32_t, 0x1222CF4);

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