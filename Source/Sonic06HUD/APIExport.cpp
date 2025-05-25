#pragma once
#include "CustomHUD.h"
#include "SubtitleUI.h"

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

extern "C" __declspec(dllexport) void API_CloseCaptionWindow()
{
	SubtitleUI::closeCaptionWindow();
}

extern "C" __declspec(dllexport) void API_SetShadowChaosLevel(uint8_t level, float maturity)
{
	CustomHUD::SetShadowChaosLevel(level, maturity);
}

extern "C" __declspec(dllexport) void API_SetGadgetMaxCount(int count)
{
	CustomHUD::SetGadgetMaxCount(count);
}

extern "C" __declspec(dllexport) void API_SetGadgetCount(int count, int maxCount)
{
	CustomHUD::SetGadgetCount(count, maxCount);
}

extern "C" __declspec(dllexport) void API_SetGadgetHP(float hp)
{
	CustomHUD::SetGadgetHP(hp);
}
