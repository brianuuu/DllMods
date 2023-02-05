#include "ArchiveTreePatcher.h"
#include "QTEJumpBoard.h"
#include "QTEReactionPlate.h"

extern "C" __declspec(dllexport) void Init(ModInfo * modInfo)
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	ArchiveTreePatcher::applyPatches();
	QTEJumpBoard::applyPatches();
	QTEReactionPlate::applyPatches();
}

extern "C" __declspec(dllexport) void PostInit()
{

}

extern "C" void __declspec(dllexport) OnFrame()
{

}