#include "UIContext.h"
#include "Application.h"
#include "Itembox.h"
#include "ScoreManager.h"
#include "Stage.h"
#include "Omochao.h"
#include "ResultUI.h"
#include "LoadingUI.h"

HWND UIContext::window;
IDirect3DDevice9* UIContext::device;
ImFont* UIContext::font;
ImFont* UIContext::fontSubtitle;

bool UIContext::isInitialized()
{
    return window && device;
}

void UIContext::initialize(HWND window, IDirect3DDevice9* device)
{
    UIContext::window = window;
    UIContext::device = device;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplDX9_Init(device);
    ImGui_ImplWin32_Init(window);

    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = false;
    io.IniFilename = (Application::getModDirString() + "ImGui.ini").c_str();

    RECT rect;
    GetClientRect(window, &rect);

    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddText("0123456789'\"");
    builder.BuildRanges(&ranges);

    const float fontSize = 43.0f * (float)*BACKBUFFER_WIDTH / 1920.0f;
    if ((font = io.Fonts->AddFontFromFileTTF((Application::getModDirString() + "Fonts\\FOT-NewRodin Pro EB.otf").c_str(), fontSize, nullptr, ranges.Data)) == nullptr)
    {
        MessageBox(nullptr, TEXT("[UIContext] Failed to load FOT-NewRodin Pro EB.otf\n"), TEXT("STH2006 Project"), MB_ICONWARNING);
        font = io.Fonts->AddFontDefault();
    }

    ImVector<ImWchar> rangesTextbox;
    ImFontGlyphRangesBuilder builderTextbox;
    for (auto const& iter : Omochao::m_fontDatabase)
    {
        builderTextbox.AddChar(iter.second);
    }
    builderTextbox.BuildRanges(&rangesTextbox);

    const float fontSubtitleSize = 40.0f * (float)*BACKBUFFER_WIDTH / 1920.0f;
    if ((fontSubtitle = io.Fonts->AddFontFromFileTTF((Application::getModDirString() + "Fonts\\FOT-RodinCattleyaPro-DB.otf").c_str(), fontSubtitleSize, nullptr, rangesTextbox.Data)) == nullptr)
    {
        MessageBox(nullptr, TEXT("[UIContext] Failed to load FOT-RodinCattleyaPro-DB.otf\n"), TEXT("STH2006 Project"), MB_ICONWARNING);
        fontSubtitle = io.Fonts->AddFontDefault();
    }
    io.Fonts->Build();

    // Initial textures
    Itembox::initTextures();
    ScoreManager::initTextures();
    Omochao::m_captionData.init();
    ResultUI::initTextures();
    LoadingUI::initTextures();
}

void UIContext::update()
{
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = (float)(*BACKBUFFER_WIDTH) * 1.1f;
    io.DisplaySize.y = (float)(*BACKBUFFER_HEIGHT)*2 * 1.1f;

    ImGui::NewFrame();
    ImGui::PushFont(font);
    
    // Check if HUD is enabled
    if (*(bool*)0x1A430D7)
    {
        // Draw imgui here
        Itembox::draw();
        ScoreManager::draw();
        Stage::draw();

        ImGui::PushFont(fontSubtitle);
        Omochao::draw();
        ResultUI::draw();
        LoadingUI::draw();
        ImGui::PopFont();
    }

    ImGui::PopFont();
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void UIContext::reset()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

LRESULT UIContext::wndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
}

bool UIContext::loadTextureFromFile(const wchar_t* filename, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height)
{
    IDirect3DTexture9* texture;

    // Load texture from disk.
    HRESULT hr = DirectX::CreateDDSTextureFromFile(device, filename, &texture);
    if (hr != S_OK)
    {
        printf("[UIContext] Error reading texture! (0x%08x)\n", hr);
        return false;
    }

    // Retrieve description of the texture surface so we can access its size.
    D3DSURFACE_DESC my_image_desc;
    texture->GetLevelDesc(0, &my_image_desc);
    *out_texture = texture;

    if (out_width)
    {
        *out_width = (int)my_image_desc.Width;
    }

    if (out_height)
    {
        *out_height = (int)my_image_desc.Height;
    }

    return true;
}
