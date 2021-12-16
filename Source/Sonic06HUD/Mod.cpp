#include "Configuration.h"
#include "Application.h"
#include "Stage.h"
#include "SubtitleUI.h"
#include "UIContext.h"
#include "SynchronizedObject.h"

bool isScoreGenLowerPriority = false;
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
        MessageBox(NULL, L"Failed to parse Config.ini", NULL, MB_ICONERROR);
    }

    isScoreGenLowerPriority = (GetModuleHandle(TEXT("ScoreGenerations.dll")) != nullptr || GetModuleHandle(TEXT("STH2006ProjectExtra.dll")) != nullptr);

    // -------------Patches--------------
    // General application patches
    Application::applyPatches();

    // Stage specific patches
    Stage::applyPatches();

    // 06 dialog box
    SubtitleUI::applyPatches();
}

extern "C" __declspec(dllexport) void PostInit()
{
    if (!isScoreGenLowerPriority && (GetModuleHandle(TEXT("ScoreGenerations.dll")) != nullptr || GetModuleHandle(TEXT("STH2006ProjectExtra.dll")) != nullptr))
    {
        MessageBox(nullptr, TEXT("'Score Generations' mod must be lower priority than 'Sonic 06 HUD'!"), TEXT("Sonic 06 HUD"), MB_ICONERROR);
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