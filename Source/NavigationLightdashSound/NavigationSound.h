#pragma once
class NavigationSound
{
public:
	static void update();
	static void applyPatches();

	static bool m_playedSoundThisFrame;
	
	static float constexpr m_lightdashDelay = 2.0f;
	static float m_lightdashTimer;
	static uint32_t m_esp;
};

