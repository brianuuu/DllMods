#pragma once
#include "APIHelper.h"

class UnleashedHUD_API
{
public:

private:
	LIB_FUNCTION(void, "UnleashedHUD.dll", API_AddRingScore, int score);
	LIB_FUNCTION(void, "UnleashedHUD.dll", API_AddSpeedScore, int score);
	LIB_FUNCTION(void, "UnleashedHUD.dll", API_AddEnemyScore, int score);
	LIB_FUNCTION(void, "UnleashedHUD.dll", API_AddTrickScore, int score);

public:
    static UnleashedHUD_API* GetInstance()
    {
        static UnleashedHUD_API* instance = nullptr;
        return instance != nullptr ? instance : instance = new UnleashedHUD_API();
    }

    static void AddRingScore(int score)
	{
		VOID_EXPORT(API_AddRingScore, score);
	}

    static void AddSpeedScore(int score)
	{
		VOID_EXPORT(API_AddSpeedScore, score);
	}

    static void AddEnemyScore(int score)
	{
		VOID_EXPORT(API_AddEnemyScore, score);
	}

    static void AddTrickScore(int score)
	{
		VOID_EXPORT(API_AddTrickScore, score);
	}
};
