#include "SynchronizedObject.h"
#include "UIContext.h"

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

extern "C" __declspec(dllexport) void Init(ModInfo_t * modInfo)
{
    auto getDirectoryPath = [](const std::string & path) -> std::string
    {
        const size_t pos = path.find_last_of("\\/");
        return path.substr(0, pos != std::string::npos ? pos : 0);
    };

    UIContext::setModDirectoryPath(getDirectoryPath(modInfo->CurrentMod->Path));

    INSTALL_HOOK(WndProc);
}

extern "C" __declspec(dllexport) void PostInit(ModInfo_t * modInfo)
{
	
}

SynchronizedObject** const APPLICATION_DOCUMENT = (SynchronizedObject**)0x1E66B34;
extern "C" void __declspec(dllexport) OnFrame()
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
}
