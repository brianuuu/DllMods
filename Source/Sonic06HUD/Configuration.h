#pragma once

class Configuration
{
public:
    enum ButtonType
    {
        BT_Xbox,
        BT_PS3,
    };

    static std::string GetVersion() { return "SHC2025"; }
    static bool load(const std::string& rootPath);

    static bool m_usingS06DE;
    static bool m_usingSTH2006Project;

    static int m_characterIcon;
    static bool m_uiColor;
    static ButtonType m_buttonType;
};
