#include "Configuration.h"

#include "Application.h"

bool Configuration::m_usingS06DE = false;
bool Configuration::m_usingSTH2006Project = false;

int Configuration::m_characterIcon = 0;
bool Configuration::m_uiColor = false;
Configuration::ButtonType Configuration::m_buttonType = Configuration::ButtonType::BT_Xbox;

bool Configuration::load(const std::string& rootPath)
{
    const INIReader reader(rootPath + "Sonic06HUD.ini");
    if (reader.ParseError() != 0)
    {
        return false;
    }

    m_characterIcon = reader.GetInteger("Main", "nCharacterIcon", m_characterIcon);
    m_uiColor = reader.GetBoolean("Main", "bUIColor", m_uiColor);
    m_buttonType = (ButtonType)reader.GetInteger("Main", "bButtonType", m_buttonType);

    // check STH2006 Project button type
    if (Common::IsModEnabled("Gameplay", "nButtonType", "0"))
    {
        m_buttonType = Configuration::ButtonType::BT_Xbox;
    }
    else if (Common::IsModEnabled("Gameplay", "nButtonType", "1"))
    {
        m_buttonType = Configuration::ButtonType::BT_PS3;
    }

    // Overwrite [PS3] section in bb2.ini
    {
        std::stringstream ss;
        std::string s;

        std::ifstream ifstream;
        std::ofstream ofstream;

        ifstream.open(Application::getModDirString() + "core\\bb2.ini");
        if (ifstream.is_open())
        {
            ss << ifstream.rdbuf();
            ifstream.close();

            s = ss.str();
            s = s.substr(0, s.find_last_of(']') + 1) + "\n";
        }

        ofstream.open(Application::getModDirString() + "core\\bb2.ini");
        if (ofstream.is_open())
        {
            ofstream << s;

            if (Configuration::m_buttonType == Configuration::ButtonType::BT_PS3)
            {
                ofstream << "+StageGate.ar.00 = #PlaystationHud.ar.00\n";
                ofstream << "+StageGate.arl = #PlaystationHud.arl\n";
            }

            ofstream.close();
        }
    }


    return true;
}
