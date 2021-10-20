﻿#pragma once

#include <d3d9.h>

class UIContext
{
    static HWND window;
    static IDirect3DDevice9* device;
    static ImFont* font;

public:
    static bool isInitialized();

    static void initialize(HWND window, IDirect3DDevice9* device);

    static void update();
    static void reset();

    static LRESULT wndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

    static bool loadTextureFromFile(const wchar_t* filename, PDIRECT3DTEXTURE9* out_texture, int* out_width = nullptr, int* out_height = nullptr);
};
