#include "Configuration.h"

#include "Application.h"
#include "AnimationSetPatcher.h"
#include "VoiceOver.h"
#include "RankQuote.h"
#include "RankRunAnimation.h"
#include "SuperSonic.h"
#include "Navigation.h"
#include "RailPhysics.h"
#include "ArchiveTreePatcher.h"
#include "Itembox.h"
#include "NextGenPhysics.h"
#include "ChaosEnergy.h"
#include "CustomCamera.h"

extern "C" void __declspec(dllexport) OnFrame()
{
    // Detect whether custom camera was used last frame
    CustomCamera::advance();
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
    // General application patches
    Application::applyPatches();
    
    // Inject new animations
    AnimationSetPatcher::applyPatches();

    // Handle voice modifications
    VoiceOver::applyPatches();

    // Allow rank comments
    RankQuote::applyPatches();

    // Allow running goal animation for some stages
    RankRunAnimation::applyPatches();

    // Changer Super Sonic's properties
    SuperSonic::applyPatches();

    // Disable lightdash hints
    Navigation::applyPatches();

    // Allow 1up and 10ring to be locked-on
    ArchiveTreePatcher::applyPatches();
    Itembox::applyPatches();

    // Apply various 06 rail physics
    RailPhysics::applyPatches();

    // Replicate 06 physics
    NextGenPhysics::applyPatches();

    // Changes how Chaos Enemgy awards boost
    ChaosEnergy::applyPatches();

    // Implement custom camera
    CustomCamera::applyPatches();

    // For checking current state
    WRITE_JUMP(0xE4FF30, StateManager::ChangeStateHOOK);

    // -------------Mandatory codes--------------
    // Patch "Disable Boost Button Prompt" by "Hyper"
    WRITE_MEMORY(0x109BC7C, uint8_t, 0xE9, 0x71, 0x01, 0x00, 0x00);

    // Patch "No Boosting Animation When Grinding" by "Skyth"
    WRITE_MEMORY(0xDF2380, uint16_t, 0xA4E9);
    WRITE_MEMORY(0xDF2382, uint8_t, 0x0);
    WRITE_NOP(0xDF2385, 1);

    // Patch "Longer Blue Trail" by "N69 & Nekit"
    WRITE_MEMORY(0xE5FB17, uint16_t, 0x9090);
    WRITE_MEMORY(0xE5FB19, uint16_t, 0x9090);
    WRITE_MEMORY(0xE5FB1B, uint16_t, 0x9090);
    WRITE_MEMORY(0xE5FE10, uint8_t, 0x48);
    WRITE_MEMORY(0xE5FE70, uint8_t, 0x48);
}
