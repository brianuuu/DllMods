#include "Configuration.h"

bool Configuration::m_using06ScoreSystem = true;
bool Configuration::m_using06HUD = false;
bool Configuration::m_usingCustomWindow = true;
Configuration::TitleLogoType Configuration::m_titleLogo = Configuration::TitleLogoType::TLT_Original;
Configuration::TitleMusicType Configuration::m_titleMusic = Configuration::TitleMusicType::TMT_Original;

bool Configuration::load(const std::string& rootPath)
{
    const INIReader reader(rootPath + "mod.ini");
    if (reader.ParseError() != 0)
    {
        return false;
    }

    m_using06ScoreSystem = reader.GetBoolean("Main", "ScoreSystem", m_using06ScoreSystem);
    m_usingCustomWindow = reader.GetBoolean("Appearance", "CustomWindow", m_usingCustomWindow);
    m_titleLogo = (TitleLogoType)reader.GetInteger("Appearance", "TitleLogo", 0);
    m_titleMusic = (TitleMusicType)reader.GetInteger("Music", "TitleMusic", 0);

    return true;
}
