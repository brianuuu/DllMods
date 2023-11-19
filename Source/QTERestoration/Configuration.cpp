#include "Configuration.h"
Configuration::ButtonType Configuration::buttonType = ButtonType::X360;

void Configuration::Read()
{
	INIReader reader(INI_FILE);

	// Appearance
	buttonType = (ButtonType)reader.GetInteger("Appearance", "buttonType", (int)buttonType);

	// Override by Unleashed HUD
	std::vector<std::string> modIniList;
	Common::GetModIniList(modIniList);
	for (int i = modIniList.size() - 1; i >= 0; i--)
	{
		std::string const& config = modIniList[i];
		std::string unleashedHUDConfig = config.substr(0, config.find_last_of("\\")) + "\\UnleashedHUD.ini";
		if (!unleashedHUDConfig.empty() && Common::IsFileExist(unleashedHUDConfig))
		{
			INIReader configReader(unleashedHUDConfig);
			buttonType = (ButtonType)configReader.GetInteger("Appearance", "buttonType", (int)buttonType); // old version compatibility
			buttonType = (ButtonType)configReader.GetInteger("HUD", "buttonType", (int)buttonType);
			break;
		}
	}
}
