#pragma once

class Configuration
{
public:
	enum TitleLogoType
	{
		TLT_Original,
		TLT_STH2006Project,
	};

	enum TitleMusicType
	{
		TMT_Original,
		TMT_E3,
	};

	enum ButtonType
	{
		BT_Xbox,
		BT_PS3,
	};

	static std::string GetVersion() { return "SHC2025"; }
    static bool load(const std::string& rootPath);

	// Apperance
	static bool m_usingCustomWindow;
	static TitleLogoType m_titleLogo;

	// Gameplay
	static bool m_using06HUD;
	static bool m_using06ScoreSystem;
	static ButtonType m_buttonType;

	// Music
	static TitleMusicType m_titleMusic;
};
