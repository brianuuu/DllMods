#include "Configuration.h"
#include "EnemyTrigger.h"

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
    EnemyTrigger::applyPatches();

    // -------------Mandatory codes--------------
    // Patch "Disable Light Dash Hints" by "Hyper"
    WRITE_MEMORY(0x528A08, uint8_t, 0xE9, 0xC8, 0x00, 0x00, 0x00);

    // Patch "Disable Boost Button Prompt" by "Hyper"
    WRITE_MEMORY(0x109BC7C, uint8_t, 0xE9, 0x71, 0x01, 0x00, 0x00);

    //Patch "Red Rings Appear On New Game" by "brianuuu"
    WRITE_NOP(0x11A9ECB, 2);
}
