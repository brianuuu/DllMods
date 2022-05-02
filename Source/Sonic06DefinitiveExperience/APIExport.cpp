#pragma once
#include "Configuration.h"

extern "C" __declspec(dllexport) Configuration::ModelType API_GetModelType()
{
	return Configuration::m_model;
}

extern "C" __declspec(dllexport) bool API_IsUsingCharacterMoveset()
{
	return Configuration::m_characterMoveset;
}
