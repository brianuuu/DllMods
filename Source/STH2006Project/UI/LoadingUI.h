/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Draws 06 loading screen with ImGUI
/*----------------------------------------------------------*/

#pragma once
class LoadingUI
{

public:
	static void applyPatches();

	static float m_startCountdown;
	static void startNowLoading(float countdown = 0.0f, bool ignoreOthers = false);
	static void stopNowLoading(float fadeInTime = 0.4f, bool forceStop = false);
	static bool IsEnabled() { return m_drawEnabled; }

	// ImGui
	static uint32_t m_stagePrevious;
	static std::string m_bottomText;
	static bool m_drawEnabled;
	static float m_fadeInTime;
	static int m_frame;
	static bool m_showNowLoading;
	static void draw();

	// Textures
	static bool m_init;
	static bool initTextures();
	static IUnknown* m_backgroundTexture;
	static IUnknown* m_stageTexture;
	static IUnknown* m_tipsTexture;
	static IUnknown* m_nowLoadingTexture;
	static IUnknown* m_arrowTexture;
	~LoadingUI()
	{
		if (m_backgroundTexture)	m_backgroundTexture->Release();
		if (m_stageTexture)			m_stageTexture->Release();
		if (m_tipsTexture)			m_tipsTexture->Release();
		if (m_nowLoadingTexture)	m_nowLoadingTexture->Release();
		if (m_arrowTexture)			m_arrowTexture->Release();
	}
};

