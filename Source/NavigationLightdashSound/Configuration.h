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
    static bool m_enableNavigationSound;
    static SoundType m_navigationSoundType;
};
