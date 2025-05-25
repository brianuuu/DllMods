#pragma once
#include "APIHelper.h"

class S06HUD_API
{
public:
	// Must match API
	enum SonicGemType
	{
		SGT_Blue,
		SGT_Red,
		SGT_Green,
		SGT_Purple,
		SGT_Sky,
		SGT_White,
		SGT_Yellow,
		SGT_None
	};

private:
	LIB_FUNCTION(bool, "Sonic06HUD.dll", API_IsYesNoWindowDrawing);
	LIB_FUNCTION(SonicGemType, "Sonic06HUD.dll", API_ScrollSonicGem, bool toRight, bool ignoreNone);
	LIB_FUNCTION(SonicGemType, "Sonic06HUD.dll", API_GetSonicGem);
	LIB_FUNCTION(void, "Sonic06HUD.dll", API_SetSonicGemEnabled, SonicGemType type, bool enabled);
	LIB_FUNCTION(void, "Sonic06HUD.dll", API_CloseCaptionWindow);
	LIB_FUNCTION(void, "Sonic06HUD.dll", API_SetShadowChaosLevel, uint8_t level, float maturity);
	LIB_FUNCTION(void, "Sonic06HUD.dll", API_SetGadgetMaxCount, int count);
	LIB_FUNCTION(void, "Sonic06HUD.dll", API_SetGadgetCount, int count, int maxCount);
	LIB_FUNCTION(void, "Sonic06HUD.dll", API_SetGadgetHP, float hp);

public:
    static S06HUD_API* GetInstance()
    {
        static S06HUD_API* instance = nullptr;
        return instance != nullptr ? instance : instance = new S06HUD_API();
    }

	static bool IsYesNoWindowDrawing()
	{
		BOOL_EXPORT(API_IsYesNoWindowDrawing);
	}

    static SonicGemType ScrollSonicGem(bool toRight, bool ignoreNone)
	{
		ENUM_EXPORT(SonicGemType, API_ScrollSonicGem, toRight, ignoreNone);
	}

    static SonicGemType GetSonicGem()
	{
		ENUM_EXPORT(SonicGemType, API_GetSonicGem);
	}

    static void SetSonicGemEnabled(SonicGemType type, bool enabled)
	{
		VOID_EXPORT(API_SetSonicGemEnabled, type, enabled);
	}

    static void CloseCaptionWindow()
	{
		VOID_EXPORT(API_CloseCaptionWindow);
	}

	static void SetShadowChaosLevel(uint8_t level, float maturity)
	{
		VOID_EXPORT(API_SetShadowChaosLevel, level, maturity);
	}

	static void SetGadgetMaxCount(int count)
	{
		VOID_EXPORT(API_SetGadgetMaxCount, count);
	}

	static void SetGadgetCount(int count, int maxCount)
	{
		VOID_EXPORT(API_SetGadgetCount, count, maxCount);
	}

	static void SetGadgetHP(float hp)
	{
		VOID_EXPORT(API_SetGadgetHP, hp);
	}
};
