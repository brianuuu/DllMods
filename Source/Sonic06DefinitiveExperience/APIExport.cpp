#pragma once
#include "Configuration.h"
#include "ChaosEnergy.h"
#include "NextGenShadow.h"

extern "C" __declspec(dllexport) Configuration::ModelType API_GetModelType()
{
	return Configuration::m_model;
}

extern "C" __declspec(dllexport) bool API_IsUsingCharacterMoveset()
{
	return Configuration::m_characterMoveset;
}

extern "C" __declspec(dllexport) void API_SetChaosEnergyRewardOverride(float amount = 0.0f)
{
	ChaosEnergy::setChaosEnergyRewardOverride(amount);
}

//---------------------------------------------------
// Shadow
//---------------------------------------------------
extern "C" __declspec(dllexport) int API_GetChaosBoostLevel()
{
	return (int)NextGenShadow::m_chaosBoostLevel;
}

extern "C" __declspec(dllexport) void API_SetChaosAttackForced(bool forced)
{
	NextGenShadow::SetChaosAttackForced(forced);
}

extern "C" __declspec(dllexport) void API_ToggleStartTeleport(bool enable)
{
	NextGenShadow::ToggleStartTeleport(enable);
}
