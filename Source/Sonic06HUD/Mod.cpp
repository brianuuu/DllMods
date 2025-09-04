#include "Configuration.h"
#include "Application.h"
#include "Stage.h"
#include "SubtitleUI.h"
#include "UIContext.h"
#include "CustomHUD.h"
#include "SynchronizedObject.h"

extern "C" __declspec(dllexport) void Init(ModInfo_t * modInfo)
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
        MessageBox(NULL, L"Failed to parse Config.ini", NULL, MB_ICONERROR);
    }

    if (Common::IsModEnabled("Main", "DLLFile", "Sonic06DefinitiveExperience.dll"))
    {
        printf("[06HUD] Sonic06DefinitiveExperience.dll detected\n");
        Configuration::m_usingS06DE = true;
    }

    if (Common::IsModEnabled("Main", "DLLFile", "STH2006Project.dll"))
    {
        printf("[06HUD] STH2006Project.dll detected\n");
        Configuration::m_usingSTH2006Project = true;
    }

    // -------------Patches--------------
    // General application patches
    Application::applyPatches();

    // Stage specific patches
    Stage::applyPatches();

    // 06 dialog box
    SubtitleUI::applyPatches();

    // Original 06 HUD
    CustomHUD::applyPatches();
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

    if (!CheckPriority("ScoreGenerations.dll", false))
    {
        MessageBox(nullptr, TEXT("'Score Generations' mod must be lower priority than 'Sonic 06 HUD'!"), TEXT("Sonic 06 HUD"), MB_ICONERROR);
        exit(-1);
    }

    if (!CheckPriority("ChipReturns.dll"))
    {
        MessageBox(nullptr, TEXT("'Chip Returns' mod must be higher priority than 'Sonic 06 HUD'!"), TEXT("Sonic 06 HUD"), MB_ICONERROR);
        exit(-1);
    }

    if (!Common::IsModEnabled("Main", "DLLFile", "GenerationsD3D9Ex.dll") && !Common::IsModEnabled("Main", "DLLFile", "GenerationsD3D11.dll"))
    {
        MessageBox(nullptr, TEXT("This mod requires the latest version of 'Direct3D 9 Ex' OR 'Direct3D 11' enabled."), TEXT("Sonic 06 HUD"), MB_ICONERROR);
        exit(-1);
    }

    if (S06DE_API::GetVersion() != "SHC2025" || STH2006_API::GetVersion() != "SHC2025")
    {
        MessageBox(nullptr, TEXT("The following mods MUST be enabled together:\n-Sonic 06 HUD (SHC2025)\n-Sonic 06 Definitive Experience (SHC2025)\n-STH2006 Project Shadow Demo (SHC2025)\n\nUsage outside of SHC2025 or modifications are not allowed."), TEXT("WARNING"), MB_ICONERROR);
        exit(-1);
    }
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

        IUnknown* device = *(IUnknown**)(*(uint32_t*)(application + 80) + 8);
        if (!device)
            return;

        HWND window = *(HWND*)(application + 72);
        if (!window)
            return;

        UIContext::initialize(window, device);

        if (UIContext::getBackend() == Backend::DX9)
            INSTALL_VTABLE_HOOK(IDirect3DDevice9, device, Reset, 16);
    }

    UIContext::update();

    // Reset HUD dt
    Application::setHudDeltaTime(0);
}