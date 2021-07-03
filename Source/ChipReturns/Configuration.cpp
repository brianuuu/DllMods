#include "Configuration.h"

bool Configuration::m_followHub = true;
bool Configuration::m_followStage = false;
//int Configuration::m_injectLayer = 0;
int Configuration::m_modelType = 0;

bool Configuration::load(const std::string& rootPath)
{
    const INIReader reader(rootPath + "mod.ini");
    if (reader.ParseError() != 0)
    {
        return false;
    }

    m_followHub = reader.GetBoolean("Config", "bFollowHub", true);
    m_followStage = reader.GetBoolean("Config", "bFollowStage", false);
    //m_injectLayer = reader.GetInteger("Config", "nLayer", 0);
    m_modelType = reader.GetInteger("Config", "nModel", 0);

    return true;
}
