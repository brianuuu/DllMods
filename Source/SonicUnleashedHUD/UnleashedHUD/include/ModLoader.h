#pragma once
// Used for backwards compatibility

static const int ModLoaderVer = 2;
static const int GameVer = 0;

struct ModInfo
{
	int LoaderVersion;
	int GameVersion;
};