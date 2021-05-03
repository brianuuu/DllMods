#include "Configuration.h"
#include "bbWriter.h"

bool Configuration::load(const std::string& rootPath)
{
    const INIReader reader(rootPath + "mod.ini");
    if (reader.ParseError() != 0)
    {
        return false;
    }

    ModelType model = (ModelType)reader.GetInteger("Main", "nModel", 0);
    LanguageType language = (LanguageType)reader.GetInteger("Main", "nLanguage", 0);
    bool run = reader.GetBoolean("Main", "bRun", false);
    bool physics = reader.GetBoolean("Main", "bPhysics", false);

    string str;

    // Write bb.ini
    {
        bbWriter bb(rootPath + "core/bb.ini");

        // TODO: non-Sonic characters will edit these
        bb.addAR("SonicActionCommon", 1);
        bb.addAR("SonicActionCommonHud", 1);

        // common
        bb.addAR("ssz202", 1);

        // languages
        bb.addName("Languages\\English");
        bb.addName("Languages\\Japanese");
        bb.addReplace("Languages\\French", "Languages\\English");
        bb.addReplace("Languages\\German", "Languages\\English");
        bb.addReplace("Languages\\Italian", "Languages\\English");
        bb.addReplace("Languages\\Spanish", "Languages\\English");

        str = "cmn200";
        bbWriter::applyModel(str, model);
        bbWriter::applyLanguage(str, language);
        bb.addReplace("cmn200.ar.00", str + ".ar.00");
        bb.addName("cmn200.ar.01");
        bb.addReplace("cmn200.arl", str + ".arl");
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
        if (physics)
        {
            bb3.addReplace("#Sonic.ar.00", "#Sonic_06phy.ar.00");
            bb3.addReplace("#Sonic.arl", "#Sonic_06phy.arl");
        }

        // TODO: non-Sonic characters will edit these
        bb3.addAR("SonicActionCommon", 1);
        bb3.addAR("SonicActionCommonHud", 1);

        // stage animations
        str = "SonicBatabata";
        bbWriter::applyModel(str, model);
        bb3.addReplace("SonicBatabata.ar.00", str + ".ar.00");
        bb3.addReplace("SonicBatabata.arl", str + ".arl");
        
        str = "SonicBoard";
        bbWriter::applyModel(str, model);
        bb3.addReplace("SonicBoard.ar.00", str + ".ar.00");
        bb3.addReplace("SonicBoard.arl", str + ".arl");
        bb3.addReplace("SonicHoloska.ar.00", str + ".ar.00");
        bb3.addReplace("SonicHoloska.arl", str + ".arl");

        str = "SonicBoardCsc";
        bbWriter::applyModel(str, model);
        bb3.addReplace("SonicBoardCsc.ar.00", str + ".ar.00");
        bb3.addReplace("SonicBoardCsc.arl", str + ".arl");

        str = "SonicBoardWap";
        bbWriter::applyModel(str, model);
        bb3.addReplace("SonicBoardWap.ar.00", str + ".ar.00");
        bb3.addReplace("SonicBoardWap.arl", str + ".arl");

        str = "SonicBsd";
        bbWriter::applyModel(str, model);
        bb3.addReplace("SonicBsd.ar.00", str + ".ar.00");
        bb3.addReplace("SonicBsd.arl", str + ".arl");

        str = "SonicChina";
        bbWriter::applyModel(str, model);
        bb3.addReplace("SonicChina.ar.00", str + ".ar.00");
        bb3.addReplace("SonicChina.arl", str + ".arl");

        str = "SonicDiving";
        bbWriter::applyModel(str, model);
        bb3.addReplace("SonicDiving.ar.00", str + ".ar.00");
        bb3.addReplace("SonicDiving.arl", str + ".arl");

        str = "SonicMission";
        bbWriter::applyModel(str, model);
        bb3.addReplace("SonicMission.ar.00", str + ".ar.00");
        bb3.addReplace("SonicMission.arl", str + ".arl");

        str = "SonicPam";
        bbWriter::applyModel(str, model);
        bb3.addReplace("SonicPam.ar.00", str + ".ar.00");
        bb3.addReplace("SonicPam.arl", str + ".arl");

        str = "SonicPla";
        bbWriter::applyModel(str, model);
        bb3.addReplace("SonicPla.ar.00", str + ".ar.00");
        bb3.addReplace("SonicPla.arl", str + ".arl");

        str = "SonicSph";
        bbWriter::applyModel(str, model);
        bb3.addReplace("SonicSph.ar.00", str + ".ar.00");
        bb3.addReplace("SonicSph.arl", str + ".arl");

        str = "SonicSsh";
        bbWriter::applyModel(str, model);
        bb3.addReplace("SonicSsh.ar.00", str + ".ar.00");
        bb3.addReplace("SonicSsh.arl", str + ".arl");

        str = "SonicSsz";
        bbWriter::applyModel(str, model);
        bb3.addReplace("SonicSsz.ar.00", str + ".ar.00");
        bb3.addReplace("SonicSsz.arl", str + ".arl");

        str = "SonicTpj";
        bbWriter::applyModel(str, model);
        bb3.addReplace("SonicTpj.ar.00", str + ".ar.00");
        bb3.addReplace("SonicTpj.arl", str + ".arl");

        str = "SonicWater";
        bbWriter::applyModel(str, model);
        bb3.addReplace("SonicWater.ar.00", str + ".ar.00");
        bb3.addReplace("SonicWater.arl", str + ".arl");

        // ranks
        str = "SonicRank";
        bbWriter::applyRun(str, run, model);
        bbWriter::applyModel(str, model);
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
        if (language == LanguageType::English)
        {
            str = "voices\\English";
            bbWriter::applyModel(str, model);
            bb3.addReplace("voices\\English", str);
        }
        else
        {
            str = "voices\\Japanese";
            bbWriter::applyModel(str, model);
            bb3.addReplace("voices\\Japanese", str);
        }

        // Sonic.ar
        str = "Sonic";
        bbWriter::applyPhysics(str, physics, model);
        bbWriter::applyRun(str, run, model);
        bbWriter::applyModel(str, model);
        bbWriter::applyLanguage(str, language);
        bb3.addReplace("Sonic.arl", str + ".arl");
        bb3.addReplace("Sonic.ar.00", str + ".ar.00");

        str = "Sonic";
        bbWriter::applyModel(str, model);
        bb3.addReplace("Sonic.ar.01", str + ".ar.01");
    }

    // Code
    //Patch "Disable Light Dash Hints" by "Hyper"
    WRITE_MEMORY(0x528A08, uint8_t, 0xE9, 0xC8, 0x00, 0x00, 0x00);

    //Patch "Disable Boost Button Prompt" by "Hyper"
    WRITE_MEMORY(0x109BC7C, uint8_t, 0xE9, 0x71, 0x01, 0x00, 0x00);

    //Patch "Disable Spin Dash on Dash Panels" by "Hyper"
    bool noSpin = reader.GetBoolean("Main", "bNoSpin", true);
    if (noSpin)
    {
        WRITE_MEMORY(0xE0AC1C, uint8_t, 0xE9, 0x27, 0x01, 0x00, 0x00);
        WRITE_MEMORY(0xE0C734, uint8_t, 0xE9, 0x27, 0x01, 0x00, 0x00);
    }

    //Patch "Disable Homing Reticle" by "Hyper"
    bool noCursor = reader.GetBoolean("Main", "bNoCursor", false);
    if (noCursor)
    {
        WRITE_MEMORY(0xDEBC36, uint8_t, 0x00);
    }

    return true;
}
