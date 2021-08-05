#pragma once

enum SoundType
{
    Unleashed = 0,
    Generations3DS,
};

class Configuration
{
public:
    static bool load(const std::string& rootPath);

    static bool m_enableLightdashSound;
    static bool m_enableLightdashPrompt;
    static bool m_enableLightdashEffect;
    static bool m_enableLightdashRings;

    static bool m_enableNavigationSound;
    static SoundType m_navigationSoundType;
    static bool m_enableBumperRailSwitch;
};
