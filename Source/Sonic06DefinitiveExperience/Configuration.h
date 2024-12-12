#pragma once

class Configuration
{

public:
    enum ModelType
    {
        Sonic = 0,
        SonicElise,
        Blaze,
        Shadow,
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

    // General
    static bool m_usingSTH2006Project;
    static ModelType m_model;

    // Camera
    static bool m_camera;
    static bool m_cameraInvertX;
    static bool m_cameraInvertY;

    // Physics
    static bool m_physics;
    static bool m_characterMoveset;
    static bool m_noCursor;
    static bool m_noTrick;

    // Sonic
    struct Sonic
    {
        static bool m_rapidSpindash;
        static RunResultType m_run;
        static std::vector<std::string> m_runStages;
        static bool m_unlimitedGauge;
        static bool m_gemsEnabled;
    };

    // Shadow
    struct Shadow
    {
        static bool m_chaosBlastCamera;
        static bool m_chaosSnapAll;
        static bool m_antiGravity;
        static bool m_floatBoost;
    };

    static bool load(const std::string& rootPath);
    static bool loadPostInit();
};
