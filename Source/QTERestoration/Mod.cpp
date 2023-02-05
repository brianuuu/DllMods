#include "ArchiveTreePatcher.h"
#include "Configuration.h"
#include "QTEJumpBoard.h"
#include "QTEReactionPlate.h"

extern "C" __declspec(dllexport) void Init(ModInfo * modInfo)
{
	Configuration::Read();

	// -------------Patches--------------
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	// Load extra archives
	ArchiveTreePatcher::applyPatches();

	// Restore QTEs
	QTEJumpBoard::applyPatches();
	QTEReactionPlate::applyPatches();
}

extern "C" __declspec(dllexport) void PostInit()
{

}

extern "C" void __declspec(dllexport) OnFrame()
{

}