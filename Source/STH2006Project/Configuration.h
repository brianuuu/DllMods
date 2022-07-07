#pragma once

class Configuration
{
public:
	enum TitleLogoType
	{
		TLT_Original,
		TLT_STH2006Project,
	};

    static bool load(const std::string& rootPath);

	static bool m_using06HUD;
	static bool m_titleLogo;
};
