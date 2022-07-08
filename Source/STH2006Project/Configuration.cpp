#include "Configuration.h"

bool Configuration::m_using06HUD = false;
bool Configuration::m_usingCustomWindow = true;
bool Configuration::m_titleLogo = Configuration::TitleLogoType::TLT_Original;

bool Configuration::load(const std::string& rootPath)
{
    const INIReader reader(rootPath + "mod.ini");
    if (reader.ParseError() != 0)
    {
        return false;
    }

    m_usingCustomWindow = reader.GetBoolean("Config", "CustomWindow", m_usingCustomWindow);
    m_titleLogo = (TitleLogoType)reader.GetInteger("Config", "TitleLogo", 0);

    return true;
}
