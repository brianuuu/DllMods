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

	// ImGui
	static std::string m_stagePrevious;
	static std::string m_bottomText;
	static void draw();

	// Textures
	static bool m_init;
	static bool m_drawEnabled;
	static float m_fadeInTime;
	static bool initTextures();
	static PDIRECT3DTEXTURE9 m_backgroundTexture;
	static PDIRECT3DTEXTURE9 m_stageTexture;
	~LoadingUI()
	{
		if (m_backgroundTexture)	m_backgroundTexture->Release();
		if (m_stageTexture)			m_stageTexture->Release();
	}
};

