#include <fstream>

#include "DataHelper.h"
#include "include\INIReader.h"
#include "include\ModLoader.h"

#define INI_FILE "UnleashedHUD.ini"

int ReadIni()
{
	INIReader configReader(INI_FILE);
	int buttons = configReader.GetInteger("HUD", "buttons", 0);

	return buttons;
}

extern "C" __declspec(dllexport) void Init(const char* path)
{
	int buttonType = ReadIni();
	WriteData(buttonType);
	HookFunctions();
}

__declspec(dllexport) ModInfo GensModInfo = { ModLoaderVer, GameVer };
