#pragma once

class ScoreUI
{
private:
	static void applyPatches();
	static void applyUIPatches();

public:
	// Bonus GUI
	static int m_rainbowRingScore;
	static int m_physicsScore;
	static float m_bonusDrawTimer;
	static void draw();

	// Textures
	static bool initTextures();
	static PDIRECT3DTEXTURE9 m_bonus_Great;
	~ScoreUI()
	{
		if (m_bonus_Great)   m_bonus_Great->Release();
	}
};

