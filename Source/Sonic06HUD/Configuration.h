#pragma once

class Configuration
{
public:
    static bool load(const std::string& rootPath);

    static bool m_usingSTH2006Project;
};
