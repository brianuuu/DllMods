#include <mmsystem.h>

#include "Configuration.h"

#include "Application.h"
#include "AnimationSetPatcher.h"
#include "VoiceOver.h"
#include "RankQuote.h"
#include "RankRunAnimation.h"
#include "SuperSonic.h"
#include "Navigation.h"
#include "RailPhysics.h"
#include "ArchiveTreePatcher.h"
#include "Itembox.h"
#include "NextGenPhysics.h"
#include "NextGenObjects.h"
#include "ChaosEnergy.h"
#include "CustomCamera.h"

extern "C" void __declspec(dllexport) OnFrame()
{
    //printf("OnFrame\n");

    // Detect whether custom camera was used last frame
    CustomCamera::advance();
}

extern "C" __declspec(dllexport) void Init(ModInfo_t * modInfo)
{
    std::string dir = modInfo->CurrentMod->Path;

    size_t pos = dir.find_last_of("\\/");
    if (pos != std::string::npos)
    {
        dir.erase(pos + 1);
    }
    
    if (!Configuration::load(dir))
    {
        MessageBox(NULL, L"Failed to parse Sonic06DefinitiveExperience.ini", NULL, MB_ICONERROR);
    }

    if (Common::DoesArchiveExist("Sonic.ar.00", { modInfo->CurrentMod->Name }))
    {
        MessageBox(nullptr, TEXT("You are NOT allowed to use other character mods with this mod, please disable them."), TEXT("Sonic 06 Definitive Experience"), MB_ICONERROR);
        exit(-1);
    }

    if (Common::IsModEnabled("Main", "DLLFile", "STH2006Project.dll"))
    {
        printf("[S06DE] STH2006Project.dll detected\n");
        Configuration::m_usingSTH2006Project = true;
    }

    // -------------Patches--------------
    // General application patches
    Application::applyPatches();
    
    // Inject new animations
    AnimationSetPatcher::applyPatches();

    // Handle voice modifications
    VoiceOver::applyPatches();

    // Allow rank comments
    RankQuote::applyPatches();

    // Allow running goal animation for some stages
    RankRunAnimation::applyPatches();

    // Changer Super Sonic's properties
    SuperSonic::applyPatches();

    // Disable lightdash hints
    Navigation::applyPatches();

    // Allow 1up and 10ring to be locked-on
    ArchiveTreePatcher::applyPatches();
    Itembox::applyPatches();

    // Apply various 06 rail physics
    RailPhysics::applyPatches();

    // Replicate 06 physics
    NextGenPhysics::applyPatches();

    // Fix up 06 objects
    NextGenObjects::applyPatches();

    // Changes how Chaos Enemgy awards boost
    ChaosEnergy::applyPatches();

    // Implement custom camera
    CustomCamera::applyPatches();

    // For checking current state
    WRITE_JUMP(0xE4FF30, StateManager::ChangeStateHOOK);

    // -------------Mandatory codes--------------
    // Patch "Disable Boost Button Prompt" by "Hyper"
    WRITE_MEMORY(0x109BC7C, uint8_t, 0xE9, 0x71, 0x01, 0x00, 0x00);

    // Patch "No Boosting Animation When Grinding" by "Skyth"
    WRITE_MEMORY(0xDF2380, uint16_t, 0xA4E9);
    WRITE_MEMORY(0xDF2382, uint8_t, 0x0);
    WRITE_NOP(0xDF2385, 1);

    // Patch "Longer Blue Trail" by "N69 & Nekit"
    WRITE_NOP(0xE5FB17, 6);
    WRITE_MEMORY(0xE5FE10, uint8_t, 0x48);
    WRITE_MEMORY(0xE5FE70, uint8_t, 0x48);

    // Patch "Fix Homing Trail Position" by "Ahremic"
    WRITE_MEMORY(0xE4F442, uint8_t, 0xF0);

    // Patch "Disable Spin Dash on Dash Panels" by "Hyper"
    WRITE_MEMORY(0xE0AC1C, uint8_t, 0xE9, 0x27, 0x01, 0x00, 0x00);
    WRITE_MEMORY(0xE0C734, uint8_t, 0xE9, 0x27, 0x01, 0x00, 0x00);

    // Patch "Disable Ring Cap" by "Dario"
    WRITE_MEMORY(0xE68522, uint16_t, 0xDEB);
    WRITE_MEMORY(0x1098E5F, uint8_t, 0xEB);
}

extern "C" __declspec(dllexport) void PostInit(ModInfo_t * modInfo)
{
    auto CheckPriority = [&modInfo](std::string const& dllName, bool isHighPriority = true) -> bool
    {
        auto* api = modInfo->API;
        auto* thisMod = modInfo->CurrentMod;

        std::string modID;
        if (Common::GetModIDFromDLL(dllName, modID))
        {
            auto* otherMod = api->FindMod(modID.c_str());
            if (otherMod)
            {
                // 0 = highest priority
                if (isHighPriority != otherMod->Priority < thisMod->Priority)
                {
                    return false;
                }
            }
        }

        return true;
    };

    if (*(uint16_t*)0x115A6AF == 0x9090)
    {
        // Revert "No Trick Rainbow Rings" code
        WRITE_MEMORY(0x115A6AF, uint8_t, 0xF7, 0xD9);

        // Apply no trick hooks if not already
        if (!Configuration::m_noTrick)
        {
            Configuration::m_noTrick = true;
            NextGenPhysics::applyNoTrickPatches();
        }
    }

    if (Configuration::m_physics && *(uint32_t*)0x121EDA4 == 0xADE9)
    {
        // Revert "Disable Grind Rail Lock-on" code
        WRITE_MEMORY(0x121EDA4, uint8_t, 0x0F, 0x84, 0xAC, 0x00, 0x00);
    }

    if (!CheckPriority("QTERestoration.dll", false))
    {
        MessageBox(nullptr, TEXT("'QTE Restoration' mod must be lower priority than 'Sonic 06 Definitive Experience'!"), TEXT("Sonic 06 Definitive Experience"), MB_ICONERROR);
        exit(-1);
    }

    if (!CheckPriority("Sonic06HUD.dll"))
    {
        MessageBox(nullptr, TEXT("'Sonic 06 HUD' mod must be higher priority than 'Sonic 06 Definitive Experience'!"), TEXT("Sonic 06 Definitive Experience"), MB_ICONERROR);
        exit(-1);
    }

    if (!Common::IsModEnabled("Main", "DLLFile", "Sonic06HUD.dll"))
    {
        MessageBox(nullptr, TEXT("This mod requires the latest version of 'Sonic 06 HUD' enabled."), TEXT("Sonic 06 Definitive Experience"), MB_ICONERROR);
        exit(-1);
    }

    if (!Configuration::loadPostInit())
    {
        MessageBox(NULL, L"Failed to parse Sonic06DefinitiveExperience.ini", NULL, MB_ICONERROR);
    }

    if (Common::IsModEnabled("Main", "DLLFile", "CustomizableResultsMusic.dll"))
    {
        MessageBox(nullptr, TEXT("'Customizable Results Music' mod is not compatible with this mod, please disable it."), TEXT("Sonic 06 Definitive Experience"), MB_ICONERROR);
        exit(-1);
    }

    if (!Common::IsModEnabled("Main", "Depends1", "brianuuu.s06hud.shc2025|Sonic 06 HUD (SHC2025)||")
    || !Common::IsModEnabled("Main", "Depends2", "brianuuu.s06de.shc2025|Sonic 06 Definitive Experience (SHC2025)||")
    || !Common::IsModEnabled("Main", "Depends1", "brianuuu.sth2006.shadow.shc2025|STH2006 Project Shadow Demo (SHC2025)||")
    || !Common::IsModEnabled("Main", "DLLFile", "Sonic06HUD.dll")
    || !Common::IsModEnabled("Main", "DLLFile", "Sonic06DefinitiveExperience.dll")
    || !Common::IsModEnabled("Main", "DLLFile", "STH2006Project.dll"))
    {
        MessageBox(nullptr, TEXT("The following mods MUST be enabled together:\n-Sonic 06 HUD (SHC2025)\n-Sonic 06 Definitive Experience (SHC2025)\n-STH2006 Project Shadow Demo (SHC2025)\n\nUsage outside of SHC2025 or modifications are not allowed."), TEXT("WARNING"), MB_ICONERROR);
        exit(-1);
    }

    bool const isSonicCheck = Configuration::m_model == Configuration::ModelType::Sonic && Configuration::Sonic::m_gemsEnabled;
    bool const isShadowCheck = Configuration::m_model == Configuration::ModelType::Shadow && Configuration::m_characterMoveset;
    if (isSonicCheck || isShadowCheck)
    {
        bool noGamepad = true;
        for (UINT i = 0; i <= 15; i++)
        {
            JOYINFO info;
            if (joyGetPos(i, &info) == JOYERR_NOERROR)
            {
                printf("[S06DE] Joystick Detected!\n");
                noGamepad = false;
                break;
            }
        }

        if (noGamepad)
        {
            if (isSonicCheck)
            {
                MessageBox(nullptr, TEXT("No controllers detected, keyboard is NOT supported if Gems are enabled for 06 Sonic."), TEXT("Sonic 06 Definitive Experience"), MB_ICONWARNING);
            }
            else if (isShadowCheck)
            {
                MessageBox(nullptr, TEXT("No controllers detected, it is required to make full use of 06 Shadow movesets."), TEXT("Sonic 06 Definitive Experience"), MB_ICONWARNING);
            }
        }
    }

    // Post init apply patches
    NextGenPhysics::applyPatchesPostInit();
}
