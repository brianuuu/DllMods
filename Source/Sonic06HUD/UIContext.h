#pragma once

#include <d3d9.h>

class UIContext
{
    static HWND window;
    static IDirect3DDevice9* device;
    static ImFont* font;
    static ImFont* fontSubtitle;
    static ImFont* fontSubtitleBig;

public:
    static bool isInitialized();

    static void initialize(HWND window, IDirect3DDevice9* device);

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
