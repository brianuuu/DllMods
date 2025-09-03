#pragma once
#include "APIHelper.h"

class STH2006_API
{
private:
	LIB_FUNCTION(std::string, "STH2006Project.dll", API_GetVersion);

public:
    static STH2006_API* GetInstance()
    {
        static STH2006_API* instance = nullptr;
        return instance != nullptr ? instance : instance = new STH2006_API();
    }

	static std::string GetVersion()
	{
		GENERIC_EXPORT(std::string, API_GetVersion);
	}
};
