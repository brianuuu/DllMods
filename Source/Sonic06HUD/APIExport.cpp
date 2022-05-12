#pragma once
#include "CustomHUD.h"

extern "C" __declspec(dllexport) bool API_IsYesNoWindowDrawing()
{
	return CustomHUD::IsYesNoWindowDrawing();
}

extern "C" __declspec(dllexport) CustomHUD::SonicGemType API_ScrollSonicGem(bool toRight, bool ignoreNone)
{
	CustomHUD::ScrollSonicGem(toRight, ignoreNone);
	return CustomHUD::m_sonicGemType;
}

extern "C" __declspec(dllexport) CustomHUD::SonicGemType API_GetSonicGem()
{
	return CustomHUD::m_sonicGemType;
}

extern "C" __declspec(dllexport) void API_SetSonicGemEnabled(CustomHUD::SonicGemType type, bool enabled)
{
	CustomHUD::m_sonicGemEnabled[type] = enabled;
}
