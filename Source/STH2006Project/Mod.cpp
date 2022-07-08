#include "Configuration.h"
#include "Application.h"
#include "EnemyTrigger.h"
#include "Navigation.h"
#include "Omochao.h"
#include "ChaosEnergy.h"
#include "ArchiveTreePatcher.h"
#include "Itembox.h"
#include "Stage.h"
#include "SoleannaNPC.h"
#include "ScoreManager.h"
#include "UIContext.h"
#include "SynchronizedObject.h"
#include "ParamManager.h"
#include "MissionManager.h"
#include "TitleUI.h"
#include "Window.h"

extern "C" __declspec(dllexport) void Init(ModInfo * modInfo)
{
    std::string modDir = modInfo->CurrentMod->Path;
    size_t pos = modDir.find_last_of("\\/");
    if (pos != std::string::npos)
    {
        modDir.erase(pos + 1);
    }
    Application::setModDir(modDir);
    
    if (!Configuration::load(modDir))
    {
        MessageBox(NULL, L"Failed to parse mod.ini", NULL, MB_ICONERROR);
    }

    if (GetModuleHandle(TEXT("Sonic06HUD.dll")))
    {
        MessageBox(nullptr, TEXT("'Sonic 06 HUD' mod detected, please put it higher priority than 'STH2006 Project'!"), TEXT("STH2006 Project"), MB_ICONERROR);
        exit(-1);
    }

    if (GetModuleHandle(TEXT("Sonic06DefinitiveExperience.dll")))
    {
        MessageBox(nullptr, TEXT("'Sonic 06 Definitive Experience' mod detected, please put it higher priority than 'STH2006 Project'!"), TEXT("STH2006 Project"), MB_ICONERROR);
        exit(-1);
    }

    // -------------Patches--------------
    // General application patches
    Application::applyPatches();

    // Enable enemy event and triggers
    EnemyTrigger::applyPatches();

    // Disable lightdash hints
    Navigation::applyPatches();

    // Character specific dialogs
    Omochao::applyPatches();

    // Changes how Chaos Enemgy awards boost
    ChaosEnergy::applyPatches();

    // Internal score system (must install before ArchiveTreePatcher)
    ScoreManager::applyPatches();

    // Allow 1up and 10ring to be locked-on
    ArchiveTreePatcher::applyPatches();
    Itembox::applyPatches();

    // Stage specific patches
    Stage::applyPatches();

    // Animate Soleanna NPCs
    SoleannaNPC::applyPatches();

    // Get Param Data
    ParamManager::applyPatches();

    // Mission related
    MissionManager::applyPatches();

    // Demo menu
    //DemoUI::applyPatches();

    // Title/Main Menu
    TitleUI::applyPatches();

    // Custom window title
    Window::applyPatches();

    // -------------Mandatory codes--------------
    // Patch "Red Rings Appear On New Game" by "brianuuu"
    WRITE_NOP(0x11A9ECB, 2);

    // Move EnemyEggRobo (Lancer/Stinger) lock-on
    static float const c_eggRoboLockScale = 0.3f;
    WRITE_MEMORY(0xBAAFBD, uint8_t, 0xD);
    WRITE_MEMORY(0xBAAFBE, float*, &c_eggRoboLockScale);
    WRITE_NOP(0xBAAFC2, 0x1);

    // Always use sega_logo_us.sfd
    WRITE_NOP(0x122C674, 2);

    // Change slide and jumpball to hit enemy as if you're boosting
    // This will make targeted enemies don't trip or explode early and get launched to targets always
    WRITE_MEMORY(0x11D72F3, uint32_t, 0x1E61B90); // slide start
    WRITE_MEMORY(0x11D7090, uint32_t, 0x1E61B90); // slide end
    WRITE_MEMORY(0x11BCC43, uint32_t, 0x1E61B90); // jumpball start
    WRITE_MEMORY(0x11BCBB2, uint32_t, 0x1E61B90); // jumpball end

    // Patch "All Levels Display Names Even If Not Unlocked" by "Muzzarino & brianuuu" (Demo 5 only)
    WRITE_MEMORY(0xD58A93, uint8_t, 0xEB, 0x41, 0xEB, 0x3F)

    // Disable character switch and camera zoom in HUB World (TODO: remove UI)
    WRITE_JUMP(0xD31697, (void*)0xD316BE);
    WRITE_MEMORY(0xD317A5, uint8_t, 0xEB);

    // Disable using RB/LB to teleport to stages (TODO: remove UI)
    WRITE_MEMORY(0x1081068, uint8_t, 0xEB);
    WRITE_MEMORY(0x1081090, uint8_t, 0xE9, 0xB7, 0x00, 0x00, 0x00, 0x90);

    // Default enable subtitle in game settings
    WRITE_MEMORY(0x551F21, uint8_t, 0x37);
    WRITE_MEMORY(0xD20009, uint8_t, 0x37);

    // Patch "1280x720 Media Player" by "N69"
    WRITE_MEMORY(0xB210A1, uint16_t, 1280);
    WRITE_MEMORY(0xB210B1, uint16_t, 720);

    // Patch "Disable Title Outro" by "Skyth"
    WRITE_MEMORY(0x57346F, uint32_t, 0x16A36CC);
}

extern "C" __declspec(dllexport) void PostInit()
{
    if (GetModuleHandle(TEXT("GenerationsD3D9Ex.dll")) == nullptr)
    {
        MessageBox(nullptr, TEXT("This mod requires the latest version of 'Direct3D 9 Ex' enabled."), TEXT("STH2006 Project"), MB_ICONERROR);
        exit(-1);
    }

    if (GetModuleHandle(TEXT("Sonic06HUD.dll")) != nullptr)
    {
        Configuration::m_using06HUD = true;

        // Use the alt version of GameHint so it can do 3 lines
        WRITE_STRING(0x1667AFC, "GameHint_C06");
        WRITE_STRING(0x1667FB8, "GameHint_C06");
        WRITE_STRING(0x16683FC, "GameHint_C06");
        WRITE_STRING(0x1668AD0, "GameHint_C06");
        WRITE_STRING(0x1668B60, "GameHint_C06");
        WRITE_STRING(0x16AF744, "GameHint_C06");
    }

    if (GetModuleHandle(TEXT("CustomizableResultsMusic.dll")))
    {
        MessageBox(nullptr, TEXT("'Customizable Results Music' mod is not compatible with this mod, please disable it."), TEXT("STH2006 Project"), MB_ICONERROR);
        exit(-1);
    }

    // Override score to all 0s and implement them ourselves
    ScoreManager::applyPostInit();
}

HOOK(LRESULT, __stdcall, WndProc, 0xE7B6C0, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (UIContext::wndProc(hWnd, Msg, wParam, lParam))
        return true;

    return originalWndProc(hWnd, Msg, wParam, lParam);
}

VTABLE_HOOK(HRESULT, WINAPI, IDirect3DDevice9, Reset, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    UIContext::reset();

    return originalIDirect3DDevice9Reset(This, pPresentationParameters);
}

bool fixedLanguage = false;
SynchronizedObject** const APPLICATION_DOCUMENT = (SynchronizedObject**)0x1E66B34;
extern "C" __declspec(dllexport) void OnFrame()
{
    if (!fixedLanguage)
    {
        // Force game language to be Japanese or English
        fixedLanguage = true;

        uint32_t voiceOverAddress = Common::GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0x10 });
        if (*(LanguageType*)voiceOverAddress != LT_English && *(LanguageType*)voiceOverAddress != LT_Japanese)
        {
            *(uint8_t*)voiceOverAddress = LT_English;
        }

        uint32_t uiAddress = Common::GetMultiLevelAddress(0x1E66B34, { 0x8 });
        if (*(LanguageType*)uiAddress != LT_English && *(LanguageType*)uiAddress != LT_Japanese)
        {
            *(uint8_t*)uiAddress = LT_English;
        }
    }

    const SynchronizedObject::Lock lock(*APPLICATION_DOCUMENT);

    if (!lock.getSuccess())
        return;

    const uint32_t applicationDocument = (uint32_t)lock.getObject();
    if (!applicationDocument)
        return;

    if (!UIContext::isInitialized())
    {
        const uint32_t application = *(uint32_t*)(applicationDocument + 20);
        if (!application)
            return;

        IDirect3DDevice9* device = *(IDirect3DDevice9**)(*(uint32_t*)(application + 80) + 8);
        if (!device)
            return;

        HWND window = *(HWND*)(application + 72);
        if (!window)
            return;

        UIContext::initialize(window, device);

        INSTALL_VTABLE_HOOK(IDirect3DDevice9, device, Reset, 16);
    }

    UIContext::update();

    // Reset HUD dt
    Application::setHudDeltaTime(0);
}
