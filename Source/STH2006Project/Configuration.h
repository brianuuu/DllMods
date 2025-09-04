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

	static std::string GetVersion() { return "SHC2025"; }
    static bool load(const std::string& rootPath);

	static bool m_using06ScoreSystem;
	static bool m_using06HUD;
	static bool m_usingCustomWindow;
	static TitleLogoType m_titleLogo;
	static TitleMusicType m_titleMusic;
};
