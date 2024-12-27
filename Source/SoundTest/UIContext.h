#pragma once

#include <d3d9.h>
#include <d3d11.h>

enum class Backend
{
    Unknown,
    DX9,
    DX11
};

class UIContext
{
    static std::string modDirectoryPath;
    static std::string imGuiIniPath;

    static HWND window;
    static IUnknown* device;
    static Backend backend;
    static ImFont* font;

public:
    static std::string getModDirectoryPath();
    static void setModDirectoryPath(const std::string& value);

    static bool isInitialized();
    static Backend getBackend();

    static void initialize(HWND window, IUnknown* device);

    static void update();
    static void reset();

    static void DrawSoundTest();

    static LRESULT wndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
};
