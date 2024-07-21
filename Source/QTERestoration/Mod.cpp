#include "ArchiveTreePatcher.h"
#include "AnimationSetPatcher.h"
#include "Configuration.h"
#include "QTEJumpBoard.h"
#include "QTEReactionPlate.h"
#include "QTEReactionPlate.h"
#include "TrickJumper.h"

extern "C" __declspec(dllexport) void Init(ModInfo * modInfo)
{
	Configuration::Read();

	// -------------Patches--------------
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	// Load extra archives
	ArchiveTreePatcher::applyPatches();

	// Load extra animations
	AnimationSetPatcher::applyPatches();

	// Restore QTEs
	QTEJumpBoard::applyPatches();
	QTEReactionPlate::applyPatches();

	TrickJumper::registerObject();
	TrickJumper::applyPatches();
}

extern "C" __declspec(dllexport) void PostInit()
{

}

extern "C" void __declspec(dllexport) OnFrame()
{

}