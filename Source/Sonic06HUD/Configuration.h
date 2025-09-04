#pragma once

class Configuration
{
public:
    static std::string GetVersion() { return "SHC2025"; }
    static bool load(const std::string& rootPath);

    static bool m_usingS06DE;
    static bool m_usingSTH2006Project;
    static int m_characterIcon;
    static bool m_uiColor;
};
