#pragma once

#define INI_FILE "Config.ini"

class Configuration
{
public:
	static void Read();
	enum class ButtonType : int
	{
		X360,
		XSX,
		PS3,
		Switch
	};

	// Appearance
	static ButtonType m_buttonType;

	// Settings
	static bool m_hackAdlibTrickJump;
	static bool m_wiiSFX;
};

