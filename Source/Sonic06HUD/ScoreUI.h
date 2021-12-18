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
	static PDIRECT3DTEXTURE9* m_bonusTexture;
	static void draw();

	// Textures
	static bool initTextures();
	static PDIRECT3DTEXTURE9 m_bonus_Great;
	~ScoreUI()
	{
		if (m_bonus_Great)   m_bonus_Great->Release();
	}
};

