#include "Configuration.h"
#include "EnemyTrigger.h"
#include "Navigation.h"
#include "Omochao.h"
#include "ExpToSonic.h"
#include "ArchiveTreePatcher.h"
#include "Itembox.h"
#include "Stage.h"
#include "SoleannaNPC.h"

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
    // Enable enemy event and triggers
    EnemyTrigger::applyPatches();

    // Disable lightdash hints
    Navigation::applyPatches();

    // Character specific dialogs
    Omochao::applyPatches();

    // Make Chaos Energy goes to Sonic (mainly for hub worlds since boost HUD is offscreen)
    ExpToSonic::applyPatches();

    // Allow 1up and 10ring to be locked-on
    ArchiveTreePatcher::applyPatches();
    Itembox::applyPatches();

    // Stage specific patches
    Stage::applyPatches();

    // Animate Soleanna NPCs
    SoleannaNPC::applyPatches();

    // -------------Mandatory codes--------------
    // Patch "Red Rings Appear On New Game" by "brianuuu"
    WRITE_NOP(0x11A9ECB, 2);

    // Move EnemyEggRobo (Lancer/Stinger) lock-on
    static float const c_eggRoboLockScale = 0.3f;
    WRITE_MEMORY(0xBAAFBD, uint8_t, 0xD);
    WRITE_MEMORY(0xBAAFBE, float*, &c_eggRoboLockScale);
    WRITE_NOP(0xBAAFC2, 0x1);

    // Change slide and jumpball to hit enemy as if you're boosting
    // This will make targeted enemies don't trip or explode early and get launched to targets always
    WRITE_MEMORY(0x11D72F3, uint32_t, 0x1E61B90); // slide start
    WRITE_MEMORY(0x11D7090, uint32_t, 0x1E61B90); // slide end
    WRITE_MEMORY(0x11BCC43, uint32_t, 0x1E61B90); // jumpball start
    WRITE_MEMORY(0x11BCBB2, uint32_t, 0x1E61B90); // jumpball end
}
