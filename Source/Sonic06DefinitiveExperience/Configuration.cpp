#include "Configuration.h"
#include "bbWriter.h"

Configuration::ModelType Configuration::m_model = Configuration::ModelType::Sonic;
Configuration::LanguageType Configuration::m_language = Configuration::LanguageType::English;

bool Configuration::m_physics = false;
bool Configuration::m_characterMoveset = false;
bool Configuration::m_xButtonAction = false;
bool Configuration::m_noTrick = false;

bool Configuration::m_camera = false;
bool Configuration::m_cameraInvertX = false;
bool Configuration::m_cameraInvertY = false;

bool Configuration::m_rapidSpindash = false;
Configuration::RunResultType Configuration::m_run = Configuration::RunResultType::Disable;
vector<string> Configuration::m_runStages = {};

bool Configuration::load(const std::string& rootPath)
{
    const INIReader reader(rootPath + "Sonic06DefinitiveExperience.ini");
    if (reader.ParseError() != 0)
    {
        return false;
    }

    // --------------General--------------
    m_model = (ModelType)reader.GetInteger("Main", "nModel", 0);
    m_language = (LanguageType)reader.GetInteger("Main", "nLanguage", 0);

    static double roundClearLength = 7.831;
    WRITE_MEMORY(0xCFD562, double*, &roundClearLength);
    if (reader.GetInteger("Main", "nStageClear", 0))
    {
        static const char* Result_Town = "Result_Town";
        WRITE_MEMORY(0xCFD3C9, char*, Result_Town);
    }
    if (reader.GetInteger("Main", "nStageLoop", 0))
    {
        // E3 loop
        WRITE_STRING(0x15B38F8, "Result2");
        WRITE_STRING(0x15B3900, "Result2");
    }
    else 
    {
        // retail loop
        WRITE_STRING(0x15B38F8, "Result1");
        WRITE_STRING(0x15B3900, "Result1");
    }

    // --------------Camera--------------
    m_camera = reader.GetBoolean("Main", "bCamera", false);
    m_cameraInvertX = reader.GetBoolean("Main", "bCameraInvertX", false);
    m_cameraInvertY = reader.GetBoolean("Main", "bCameraInvertY", false);

    // --------------Physics--------------
    m_physics = reader.GetBoolean("Main", "bPhysics", false);
    m_characterMoveset = reader.GetBoolean("Main", "bCharacterMoveset", false);
    m_xButtonAction = reader.GetBoolean("Main", "bXAction", false);
    m_noTrick = reader.GetBoolean("Main", "bNoTrick", false);

    // --------------Sonic--------------
    m_rapidSpindash = reader.GetBoolean("Main", "bRapidSpindash", false);

    // Get running goal custom stage list
    string runStages = reader.Get("Main", "sRunStages", "");
    string delimiter = ",";
    size_t pos = 0;
    string token;
    while ((pos = runStages.find(delimiter)) != string::npos)
    {
        token = runStages.substr(0, pos);
        m_runStages.push_back(token);
        runStages.erase(0, pos + delimiter.length());
    }
    m_runStages.push_back(runStages);
    m_run = (RunResultType)reader.GetInteger("Main", "nRun", 0);

    string str;

    // Write bb.ini
    {
        bbWriter bb(rootPath + "core/bb.ini");

        // TODO: non-Sonic characters will edit these
        bb.addAR("SonicActionCommon", 1);
        bb.addAR("SonicActionCommonHud", 1);

        // common
        bb.addAR("cmn200", 1);
        bb.addAR("ssz202", 1);
        bb.addAR("#SystemCommonItemboxLock", 1);
        bb.addAR("ItemboxLock", 1);

        // languages
        bb.addName("Languages\\English");
        bb.addName("Languages\\Japanese");
        bb.addReplace("Languages\\French", "Languages\\English");
        bb.addReplace("Languages\\German", "Languages\\English");
        bb.addReplace("Languages\\Italian", "Languages\\English");
        bb.addReplace("Languages\\Spanish", "Languages\\English");
    }

    // Write bb3.ini
    {
        bbWriter bb3(rootPath + "core/bb3.ini");

        // language
        bb3.addName("Languages\\English");
        bb3.addName("Languages\\Japanese");
        bb3.addReplace("Languages\\French", "Languages\\English");
        bb3.addReplace("Languages\\German", "Languages\\English");
        bb3.addReplace("Languages\\Italian", "Languages\\English");
        bb3.addReplace("Languages\\Spanish", "Languages\\English");

        // Physics
        if (m_physics)
        {
            bb3.addReplace("#Sonic.ar.00", "#Sonic_06phy.ar.00");
            bb3.addReplace("#Sonic.arl", "#Sonic_06phy.arl");
        }
        else
        {
            bb3.addAR("#Sonic", 1);
        }

        // TODO: non-Sonic characters will edit these
        bb3.addAR("SonicActionCommon", 1);
        bb3.addAR("SonicActionCommonHud", 1);

        // stage animations
        str = "SonicBatabata";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicBatabata.ar.00", str + ".ar.00");
        bb3.addReplace("SonicBatabata.arl", str + ".arl");
        
        str = "SonicBoard";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicBoard.ar.00", str + ".ar.00");
        bb3.addReplace("SonicBoard.arl", str + ".arl");
        bb3.addReplace("SonicHoloska.ar.00", str + ".ar.00");
        bb3.addReplace("SonicHoloska.arl", str + ".arl");

        str = "SonicBoardCsc";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicBoardCsc.ar.00", str + ".ar.00");
        bb3.addReplace("SonicBoardCsc.arl", str + ".arl");

        str = "SonicBoardWap";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicBoardWap.ar.00", str + ".ar.00");
        bb3.addReplace("SonicBoardWap.arl", str + ".arl");

        str = "SonicBsd";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicBsd.ar.00", str + ".ar.00");
        bb3.addReplace("SonicBsd.arl", str + ".arl");

        str = "SonicChina";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicChina.ar.00", str + ".ar.00");
        bb3.addReplace("SonicChina.arl", str + ".arl");

        str = "SonicDiving";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicDiving.ar.00", str + ".ar.00");
        bb3.addReplace("SonicDiving.arl", str + ".arl");

        str = "SonicMission";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicMission.ar.00", str + ".ar.00");
        bb3.addReplace("SonicMission.arl", str + ".arl");

        str = "SonicPam";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicPam.ar.00", str + ".ar.00");
        bb3.addReplace("SonicPam.arl", str + ".arl");

        str = "SonicPla";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicPla.ar.00", str + ".ar.00");
        bb3.addReplace("SonicPla.arl", str + ".arl");

        str = "SonicSph";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicSph.ar.00", str + ".ar.00");
        bb3.addReplace("SonicSph.arl", str + ".arl");

        str = "SonicSsh";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicSsh.ar.00", str + ".ar.00");
        bb3.addReplace("SonicSsh.arl", str + ".arl");

        str = "SonicSsz";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicSsz.ar.00", str + ".ar.00");
        bb3.addReplace("SonicSsz.arl", str + ".arl");

        str = "SonicTpj";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicTpj.ar.00", str + ".ar.00");
        bb3.addReplace("SonicTpj.arl", str + ".arl");

        str = "SonicWater";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicWater.ar.00", str + ".ar.00");
        bb3.addReplace("SonicWater.arl", str + ".arl");

        // ranks
        str = "SonicRank";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("SonicRankS.ar.00", str + ".ar.00");
        bb3.addReplace("SonicRankS.arl", str + ".arl");
        bb3.addReplace("SonicRankA.ar.00", str + ".ar.00");
        bb3.addReplace("SonicRankA.arl", str + ".arl");
        bb3.addReplace("SonicRankB.ar.00", str + ".ar.00");
        bb3.addReplace("SonicRankB.arl", str + ".arl");
        bb3.addReplace("SonicRankC.ar.00", str + ".ar.00");
        bb3.addReplace("SonicRankC.arl", str + ".arl");
        bb3.addReplace("SonicRankD.ar.00", str + ".ar.00");
        bb3.addReplace("SonicRankD.arl", str + ".arl");

        // voice
        if (m_language == LanguageType::English)
        {
            str = "voices\\English";
            bbWriter::applyModel(str, m_model);
            bb3.addReplace("voices\\English", str);
        }
        else
        {
            str = "voices\\Japanese";
            bbWriter::applyModel(str, m_model);
            bb3.addReplace("voices\\Japanese", str);
        }

        // Sonic.ar
        str = "Sonic";
        bbWriter::applyModel(str, m_model);
        bb3.addReplace("Sonic.arl", str + ".arl");
        bb3.addReplace("Sonic.ar.00", str + ".ar.00");
    }

    // Optional codes
    // Patch "Disable Homing Reticle" by "Hyper"
    if (reader.GetBoolean("Main", "bNoCursor", false))
    {
        WRITE_MEMORY(0xB6AB8C, uint8_t, 0xE9, 0xF7, 0x01, 0x00, 0x00);
        WRITE_MEMORY(0xDEBC36, uint8_t, 0x00);
    }

    // Patch "Boost Gauge Starts Empty" by "PTKickass"
    if (reader.GetBoolean("Main", "bNoBoost", false))
    {
        WRITE_MEMORY(0xE64F2F, uint32_t, 0x90C9570F);
        WRITE_NOP(0xE64F33, 4);
    }

    // Patch "All Rings Can Be Light Dashed" by "Skyth"
    // Patch "Disable Light Dash Particles" by "Hyper"
    if (reader.GetBoolean("Main", "bLightdashAll", false))
    {
        WRITE_MEMORY(0x105334D, uint32_t, 0x10C47C6);
        WRITE_NOP(0x1053351, 16);
        WRITE_MEMORY(0x10538EB, uint8_t, 0xE9, 0x8F, 0x00, 0x00, 0x00, 0x90);
    }

    return true;
}
