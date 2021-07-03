#pragma once

class Configuration
{
public:
    static bool load(const std::string& rootPath);

    static bool m_followHub;
    static bool m_followStage;
    //static int m_injectLayer;
    static int m_modelType;
};
