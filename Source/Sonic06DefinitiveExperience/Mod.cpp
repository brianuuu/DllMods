#include "Configuration.h"

#include "Application.h"
#include "RankQuote.h"
#include "RankRunAnimation.h"
#include "SuperSonic.h"
#include "Navigation.h"
#include "ExpToSonic.h"
#include "RailPhysics.h"
#include "ArchiveTreePatcher.h"
#include "Itembox.h"
#include "NextGenPhysics.h"

extern "C" void __declspec(dllexport) OnFrame()
{
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

    // -------------Patches--------------
    // General application pathces
    Application::applyPatches();

    // Allow rank comments
    RankQuote::applyPatches();

    // Allow running goal animation for some stages
    RankRunAnimation::applyPatches();

    // Changer Super Sonic's properties
    SuperSonic::applyPatches();

    // Disable lightdash hints
    Navigation::applyPatches();

    // Make Chaos Energy goes to Sonic
    ExpToSonic::applyPatches();

    // Allow 1up and 10ring to be locked-on
    ArchiveTreePatcher::applyPatches();
    Itembox::applyPatches();

    // Apply various 06 rail physics
    RailPhysics::applyPatches();

    // Replicate 06 physics
    NextGenPhysics::applyPatches();

    // -------------Mandatory codes--------------
    // Patch "Disable Boost Button Prompt" by "Hyper"
    WRITE_MEMORY(0x109BC7C, uint8_t, 0xE9, 0x71, 0x01, 0x00, 0x00);

    // Patch "No Boosting Animation When Grinding" by "Skyth"
    WRITE_MEMORY(0xDF2380, uint16_t, 0xA4E9);
    WRITE_MEMORY(0xDF2382, uint8_t, 0x0);
    WRITE_NOP(0xDF2385, 1);
}
