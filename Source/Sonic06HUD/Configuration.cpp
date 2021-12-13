#include "Configuration.h"

bool Configuration::m_usingSTH2006Project = false;

bool Configuration::load(const std::string& rootPath)
{
    const INIReader reader(rootPath + "mod.ini");
    if (reader.ParseError() != 0)
    {
        return false;
    }

    if (GetModuleHandle(TEXT("STH2006Project.dll")) != nullptr)
    {
        m_usingSTH2006Project = true;
    }

    return true;
}
