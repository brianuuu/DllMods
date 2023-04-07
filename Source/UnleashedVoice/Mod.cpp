#include "RankQuote.h"
#include "UnleashedVoice.h"

extern "C" void __declspec(dllexport) OnFrame()
{

}

extern "C" __declspec(dllexport) void Init(ModInfo * modInfo)
{
    RankQuote::applyPatches();
    UnleashedVoice::applyPatches();
}