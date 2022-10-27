#pragma once

class ScoreUI
{
private:
	static void applyPatches();

public:
	// Rainbow ring bonus
	static uint32_t m_rainbowRingChain;
	static int calculateRainbowRingChainBonus();
	static void __fastcall addRainbowScore();

	// Bonus GUI
	static int m_bonus;
	static float m_bonusTimer;
	static float m_bonusDrawTimer;
	static IUnknown** m_bonusTexture;
	static void draw();
	static void clearDraw();

	// Textures
	static bool initTextures();
	static IUnknown* m_bonus_Great;
	~ScoreUI()
	{
		if (m_bonus_Great)   m_bonus_Great->Release();
	}
};

