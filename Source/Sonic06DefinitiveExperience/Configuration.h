#pragma once

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

class Configuration
{
public:
    static bool load(const std::string& rootPath);
};
