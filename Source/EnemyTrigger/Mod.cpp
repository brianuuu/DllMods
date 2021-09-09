#include "EnemyTrigger.h"

extern "C" void __declspec(dllexport) OnFrame()
{

}

extern "C" __declspec(dllexport) void Init(ModInfo * modInfo)
{
	EnemyTrigger::applyPatches();
}