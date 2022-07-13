#include "Configuration.h"

bool Configuration::m_blazeSupport = true;

bool Configuration::load(const std::string& rootPath)
{
    const INIReader reader(rootPath + "mod.ini");
    if (reader.ParseError() != 0)
    {
        return false;
    }

    m_blazeSupport = reader.Get("Main", "IncludeDir1", "") == "blaze_support";

    return true;
}
