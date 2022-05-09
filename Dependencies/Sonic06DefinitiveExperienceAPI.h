#pragma once
#include "APIHelper.h"

class S06DE_API
{
public:
    enum ModelType
    {
        Sonic = 0,
        SonicElise,
        Blaze,
    };

private:
    LIB_FUNCTION(ModelType, "Sonic06DefinitiveExperience.dll", API_GetModelType);
    LIB_FUNCTION(bool, "Sonic06DefinitiveExperience.dll", API_IsUsingCharacterMoveset);

public:
    static S06DE_API* GetInstance()
    {
        static S06DE_API* instance = nullptr;
        return instance != nullptr ? instance : instance = new S06DE_API();
    }

    static ModelType GetModelType()
	{
		ENUM_EXPORT(S06DE_API::ModelType, API_GetModelType);
	}
	
    static bool IsUsingCharacterMoveset()
	{
		BOOL_EXPORT(API_IsUsingCharacterMoveset);
	}
};