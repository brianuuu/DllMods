#include "Configuration.h"
#include "ArchiveTreePatcher.h"
#include "AutodrawPatcher.h"
#include "SetdataPatcher.h"
#include "Chip.h"

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

    std::srand((unsigned int)time(NULL));
    ArchiveTreePatcher::applyPatches();
    AutodrawPatcher::applyPatches();
    SetdataPatcher::applyPatches();
    Chip::applyPatches();
}