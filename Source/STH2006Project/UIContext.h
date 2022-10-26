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
    static HWND window;
    static IUnknown* device;
    static Backend backend;
    static ImFont* font;
    static ImFont* fontSubtitle;

public:
    static bool isInitialized();
    static Backend getBackend();

    static void initialize(HWND window, IUnknown* device);
    static bool initFontDatabase(std::wstring const& file, ImFontGlyphRangesBuilder& builder);

    static void update();
    static void clearDraw();
    static void reset();

    static LRESULT wndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

    static bool loadTextureFromFile(const wchar_t* filename, PDIRECT3DTEXTURE9* out_texture, int* out_width = nullptr, int* out_height = nullptr);

    static constexpr ImGuiWindowFlags m_hudFlags
        = ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoBackground;
};
