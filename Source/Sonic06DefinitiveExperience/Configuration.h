#pragma once

class Configuration
{

public:
    enum ModelType
    {
        Sonic = 0,
        SonicElise,
    };

    enum LanguageType
    {
        English = 0,
        Japanese,
    };

    enum RunResultType
    {
        Disable = 0,
        EnableAll,
        STH2006,
        Custom
    };

    static ModelType m_model;
    static LanguageType m_language;

    static RunResultType m_run;
    static vector<string> m_runStages;

    static bool m_physics;
    static bool m_xButtonAction;

    static bool load(const std::string& rootPath);
};
