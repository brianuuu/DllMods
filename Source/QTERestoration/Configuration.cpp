#include "Configuration.h"
Configuration::ButtonType Configuration::m_buttonType = ButtonType::X360;
bool Configuration::m_hackAdlibTrickJump = false;
bool Configuration::m_wiiSFX = false;

void Configuration::Read()
{
	INIReader reader(INI_FILE);

	// Appearance
	m_buttonType = (ButtonType)reader.GetInteger("Appearance", "buttonType", (int)m_buttonType);

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
			m_buttonType = (ButtonType)configReader.GetInteger("Appearance", "buttonType", (int)m_buttonType); // old version compatibility
			m_buttonType = (ButtonType)configReader.GetInteger("HUD", "buttonType", (int)m_buttonType);
			break;
		}
	}

	// Settings
	m_hackAdlibTrickJump = reader.GetBoolean("Settings", "hackAdlibTrickJump", m_hackAdlibTrickJump);
	m_hackAdlibTrickJump = reader.GetBoolean("Settings", "wiiSFX", m_wiiSFX);
}
