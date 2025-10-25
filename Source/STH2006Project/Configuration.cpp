#include "Configuration.h"

bool Configuration::m_usingCustomWindow = true;
Configuration::TitleLogoType Configuration::m_titleLogo = Configuration::TitleLogoType::TLT_Original;

Configuration::ButtonType Configuration::m_buttonType = Configuration::ButtonType::BT_Xbox;
bool Configuration::m_gliderInvert = false;
bool Configuration::m_using06HUD = false;
bool Configuration::m_using06ScoreSystem = true;

Configuration::TitleMusicType Configuration::m_titleMusic = Configuration::TitleMusicType::TMT_Original;

bool Configuration::load(const std::string& rootPath)
{
    const INIReader reader(rootPath + "mod.ini");
    if (reader.ParseError() != 0)
    {
        return false;
    }

    // Appearance
    m_usingCustomWindow = reader.GetBoolean("Appearance", "bCustomWindow", m_usingCustomWindow);
    m_titleLogo = (TitleLogoType)reader.GetInteger("Appearance", "nTitleLogo", m_titleLogo);

    // Gameplay
    m_buttonType = (ButtonType)reader.GetInteger("Gameplay", "nButtonType", m_buttonType);
    m_gliderInvert = reader.GetBoolean("Gameplay", "bGliderInvert", m_gliderInvert);

    // Music
    m_titleMusic = (TitleMusicType)reader.GetInteger("Music", "nTitleMusic", m_titleMusic);

    return true;
}
