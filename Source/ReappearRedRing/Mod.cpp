#include "ArchiveTreePatcher.h"
#include "ReappearingRedRing.h"

extern "C" __declspec(dllexport) void Init(ModInfo * modInfo)
{
    ArchiveTreePatcher::applyPatches();
    ReappearingRedRing::applyPatches();
}