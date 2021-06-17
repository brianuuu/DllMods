#include "Configuration.h"

bool Configuration::m_enableLightdashSound = true;
bool Configuration::m_enableNavigationSound = true;
SoundType Configuration::m_navigationSoundType = SoundType::Unleashed;
bool Configuration::m_enableLightdashPrompt = true;
bool Configuration::m_enableBumperRailSwitch = false;

bool Configuration::load(const std::string& rootPath)
{
    const INIReader reader(rootPath + "mod.ini");
    if (reader.ParseError() != 0)
    {
        return false;
    }

    m_enableLightdashSound = reader.GetBoolean("Config", "bLightdash", true);
    m_enableNavigationSound = reader.GetBoolean("Config", "bNavigation", true);
    m_navigationSoundType = (SoundType)reader.GetInteger("Config", "nNavSoundType", 0);
    m_enableLightdashPrompt = reader.GetBoolean("Config", "bLightdashPrompt", true);
    m_enableBumperRailSwitch = reader.GetBoolean("Config", "bBumperRailSwitch", false);
    return true;
}
