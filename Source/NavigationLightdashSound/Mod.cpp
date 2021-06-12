#include "Configuration.h"
#include "ArchiveTreePatcher.h"
#include "NavigationSound.h"
#include "LightdashSound.h"

#include <Hedgehog.h>

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
}

extern "C" void __declspec(dllexport) OnFrame()
{
    if (Configuration::m_enableNavigationSound)
    {
        NavigationSound::update();
    }
}