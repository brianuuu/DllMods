#pragma once
#include "APIHelper.h"

class S06DE_API
{
public:
    enum ModelType
    {
        None = -1,
        Sonic,
        SonicElise,
        Blaze,
        Shadow,
    };

private:
    LIB_FUNCTION(ModelType, "Sonic06DefinitiveExperience.dll", API_GetModelType);
    LIB_FUNCTION(bool, "Sonic06DefinitiveExperience.dll", API_IsUsingCharacterMoveset);
    LIB_FUNCTION(void, "Sonic06DefinitiveExperience.dll", API_SetChaosEnergyRewardOverride, float amount);
    LIB_FUNCTION(int, "Sonic06DefinitiveExperience.dll", API_GetChaosBoostLevel);
    LIB_FUNCTION(void, "Sonic06DefinitiveExperience.dll", API_SetChaosAttackForced, bool forced);

public:
    static S06DE_API* GetInstance()
    {
        static S06DE_API* instance = nullptr;
        return instance != nullptr ? instance : instance = new S06DE_API();
    }

    static ModelType GetModelType()
	{
		ENUM_EXPORT(ModelType, API_GetModelType);
	}
	
    static bool IsUsingCharacterMoveset()
	{
		BOOL_EXPORT(API_IsUsingCharacterMoveset);
	}
	
    static void SetChaosEnergyRewardOverride(float amount = 0.0f)
	{
		VOID_EXPORT(API_SetChaosEnergyRewardOverride, amount);
	}
	
    static int GetChaosBoostLevel()
	{
		INT_EXPORT(API_GetChaosBoostLevel);
	}
	
    static void SetChaosAttackForced(bool forced)
	{
		VOID_EXPORT(API_SetChaosAttackForced, forced);
	}
};
