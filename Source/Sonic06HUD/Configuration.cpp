#include "Configuration.h"

bool Configuration::m_usingS06DE = false;
bool Configuration::m_usingSTH2006Project = false;
int Configuration::m_characterIcon = 0;
bool Configuration::m_uiColor = false;

bool Configuration::load(const std::string& rootPath)
{
    const INIReader reader(rootPath + "Sonic06HUD.ini");
    if (reader.ParseError() != 0)
    {
        return false;
    }

    m_characterIcon = reader.GetInteger("Main", "nCharacterIcon", 0);
    m_uiColor = reader.GetBoolean("Main", "bUIColor", false);

    return true;
}
