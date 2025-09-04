#pragma once
#include "Configuration.h"

extern "C" __declspec(dllexport) std::string API_GetVersion()
{
	return Configuration::GetVersion();
}
