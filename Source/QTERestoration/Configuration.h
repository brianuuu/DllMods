#pragma once

#define INI_FILE "Config.ini"

class Configuration
{
public:
	static void Read();
	static enum class ButtonType : int
	{
		X360,
		XSX,
		PS3,
		Switch
	};

	static ButtonType buttonType;
};
