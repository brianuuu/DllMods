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

    Common::TestModPriority(modInfo->CurrentMod->Name, "Sonic 06 Definitive Experience", true);

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

    // -------------Mandatory codes--------------
    // Patch "Red Rings Appear On New Game" by "brianuuu"
    WRITE_NOP(0x11A9ECB, 2);

    // Move EnemyEggRobo (Lancer/Stinger) lock-on
    static float const c_eggRoboLockScale = 0.3f;
    WRITE_MEMORY(0xBAAFBD, uint8_t, 0xD);
    WRITE_MEMORY(0xBAAFBE, float*, &c_eggRoboLockScale);
    WRITE_NOP(0xBAAFC2, 0x1);

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

SynchronizedObject** const APPLICATION_DOCUMENT = (SynchronizedObject**)0x1E66B34;
extern "C" __declspec(dllexport) void OnFrame()
{
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
