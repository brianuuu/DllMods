#pragma once
#include "Configuration.h"
#include "NextGenShadow.h"

extern "C" __declspec(dllexport) Configuration::ModelType API_GetModelType()
{
	return Configuration::m_model;
}

extern "C" __declspec(dllexport) bool API_IsUsingCharacterMoveset()
{
	return Configuration::m_characterMoveset;
}

extern "C" __declspec(dllexport) int API_GetChaosBoostLevel()
{
	return (int)NextGenShadow::m_chaosBoostLevel;
}
