#include "UIContext.h"

#include "Managers/MstManager.h"
#include "Managers/ScoreManager.h"
#include "Objects/cmn/Itembox.h"
#include "System/Application.h"
#include "UI/LoadingUI.h"
#include "UI/SubtitleUI.h"
#include "UI/TitleUI.h"

HWND UIContext::window;
IUnknown* UIContext::device;
Backend UIContext::backend;
ImFont* UIContext::font;
ImFont* UIContext::fontSubtitle;

bool UIContext::isInitialized()
{
    return window && device;
}

HOOK(float*, __fastcall, MsgFadeOutFxp, 0x10CEDB0, void* This, void* Edx, float* a2)
{
    UIContext::clearDraw();
    return originalMsgFadeOutFxp(This, Edx, a2);
}

HOOK(float*, __fastcall, MsgFadeOutMtfx, 0x57B270, void* This, void* Edx, float* a2)
{
    UIContext::clearDraw();
    return originalMsgFadeOutMtfx(This, Edx, a2);
}

Backend UIContext::getBackend()
{
    return backend;
}

void UIContext::initialize(HWND window, IUnknown* device)
{
    cacheSubtitleCharacters();

    INSTALL_HOOK(MsgFadeOutFxp);
    INSTALL_HOOK(MsgFadeOutMtfx);

    UIContext::window = window;
    UIContext::device = device;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    backend = Backend::Unknown;
    {
        IDirect3DDevice9* d3d9Device = nullptr;

        if (SUCCEEDED(device->QueryInterface(&d3d9Device)))
        {
            backend = Backend::DX9;

            ImGui_ImplDX9_Init(d3d9Device);

            d3d9Device->Release();
        }
    }

    if (backend == Backend::Unknown)
    {
        ID3D11Device* d3d11Device = nullptr;

        if (SUCCEEDED(device->QueryInterface(&d3d11Device)))
        {
            backend = Backend::DX11;

            ID3D11DeviceContext* d3d11Context = nullptr;
            d3d11Device->GetImmediateContext(&d3d11Context);

            ImGui_ImplDX11_Init(d3d11Device, d3d11Context);

            d3d11Device->Release();
            d3d11Context->Release();
        }
    }

    if (backend == Backend::Unknown)
    {
        MessageBox(nullptr, TEXT("[UIContext] Failed to initialize"), TEXT("STH2006 Project"), MB_ICONERROR);
        exit(-1);
    }

    ImGui_ImplWin32_Init(window);

    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = false;
    io.IniFilename = (Application::getModDirString() + "ImGui.ini").c_str();

    RECT rect;
    GetClientRect(window, &rect);

    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddText("0123456789'\"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ: ?");
    builder.BuildRanges(&ranges);

    const float fontSize = 43.0f;
    if ((font = io.Fonts->AddFontFromFileTTF((Application::getModDirString() + "Fonts\\FOT-NewRodin-Pro-EB.otf").c_str(), fontSize, nullptr, ranges.Data)) == nullptr)
    {
        MessageBox(nullptr, TEXT("[UIContext] Failed to load FOT-NewRodin-Pro-EB.otf\n"), TEXT("STH2006 Project"), MB_ICONWARNING);
        font = io.Fonts->AddFontDefault();
    }

    ImVector<ImWchar> rangesTextbox;
    ImFontGlyphRangesBuilder builderTextbox;
    builderTextbox.AddText("0123456789'\"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ: ?");
    initFontDatabase(L"Assets\\Database\\npcData.ini", builderTextbox);
    initFontDatabase(L"Assets\\Database\\titleData.ini", builderTextbox);
    for (auto const& iter : TitleUI::m_yesNoText)
    {
        builderTextbox.AddText(iter.second.c_str());
    }
    for (auto const& iter : TitleUI::m_menuText)
    {
        builderTextbox.AddText(iter.second.c_str());
    }
    builderTextbox.BuildRanges(&rangesTextbox);

    const float fontSubtitleSize = 39.0f;
    if ((fontSubtitle = io.Fonts->AddFontFromFileTTF((Application::getModDirString() + "Fonts\\FOT-RodinCattleyaPro-DB.otf").c_str(), fontSubtitleSize, nullptr, rangesTextbox.Data)) == nullptr)
    {
        MessageBox(nullptr, TEXT("[UIContext] Failed to load FOT-RodinCattleyaPro-DB.otf\n"), TEXT("STH2006 Project"), MB_ICONWARNING);
        fontSubtitle = io.Fonts->AddFontDefault();
    }
    io.Fonts->Build();

    // Initial textures
    Itembox::initTextures();
    ScoreManager::initTextures();
    SubtitleUI::m_captionData.init();
    LoadingUI::initTextures();
}

bool UIContext::initFontDatabase(std::wstring const& file, ImFontGlyphRangesBuilder& builder)
{
    std::ifstream fstream(Application::getModDirWString() + file);
    if (fstream.is_open())
    {
        std::stringstream ss;
        ss << fstream.rdbuf();
        fstream.close();
        builder.AddText(ss.str().c_str());
        return true;
    }

    MessageBox(nullptr, TEXT("Failed to load font database, text may not display correctly."), TEXT("STH2006 Project"), MB_ICONERROR);
    return false;
}

void UIContext::update()
{
    switch (backend)
    {
    case Backend::DX9:
        ImGui_ImplDX9_NewFrame();
        break;

    case Backend::DX11:
        ImGui_ImplDX11_NewFrame();
        break;
    }

    ImGui_ImplWin32_NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = (float)(*BACKBUFFER_WIDTH) * 1.1f;
    io.DisplaySize.y = (float)(*BACKBUFFER_HEIGHT)*2 * 1.1f;

    float fontScale = ((float)*BACKBUFFER_WIDTH / (float)*BACKBUFFER_HEIGHT) >= (16.0f / 9.0f)
                    ? (float)*BACKBUFFER_HEIGHT / 1080.0f
                    : (float)*BACKBUFFER_WIDTH / 1920.0f;

    ImGui::NewFrame();
    
    // Check if HUD is enabled
    if (*(bool*)0x1A430D7)
    {
        // Draw imgui here
        if (!S06HUD_API::IsYesNoWindowDrawing())
        {
            Itembox::draw();

            font->Scale = fontScale;
            ImGui::PushFont(font);
            TitleUI::drawMenu();
            ScoreManager::draw();
            ImGui::PopFont();

            fontSubtitle->Scale = fontScale;
            ImGui::PushFont(fontSubtitle);
            TitleUI::drawStageData();
            TitleUI::drawYesNoWindow();
            SubtitleUI::draw();
            LoadingUI::draw();
            ImGui::PopFont();
        }
    }

    ImGui::EndFrame();
    ImGui::Render();

    switch (backend)
    {
    case Backend::DX9:
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        break;

    case Backend::DX11:
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        break;
    }
}

void UIContext::clearDraw()
{
    Itembox::clearDraw();
    ScoreManager::clearDraw();
    SubtitleUI::clearDraw();
}

void UIContext::reset()
{
    switch (backend)
    {
    case Backend::DX9:
        ImGui_ImplDX9_InvalidateDeviceObjects();
        break;

    case Backend::DX11:
        ImGui_ImplDX11_InvalidateDeviceObjects();
        break;
    }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT UIContext::wndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
}

bool UIContext::loadTextureFromFile(const wchar_t* filename, IUnknown** out_texture, int* out_width, int* out_height)
{
    HRESULT hr = S_FALSE;

    // Load texture from disk.
    switch (backend)
    {
    case Backend::DX9:
    {
        IDirect3DDevice9* d3d9Device = nullptr;
        if (SUCCEEDED(device->QueryInterface(&d3d9Device)))
        {
            IDirect3DTexture9* texture = nullptr;
            hr = DirectX::CreateDDSTextureFromFile(d3d9Device, filename, &texture);
            d3d9Device->Release();

            if (hr == S_OK)
            {
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
        }
        break;
    }
    case Backend::DX11:
    {
        ID3D11Device* d3d11Device = nullptr;
        if (SUCCEEDED(device->QueryInterface(&d3d11Device)))
        {
            ID3D11Resource* texture = nullptr;
            ID3D11ShaderResourceView* textureView = nullptr;

            hr = DirectX::CreateDDSTextureFromFile(d3d11Device, filename, &texture, &textureView);
            d3d11Device->Release();

            if (hr == S_OK)
            {
                ID3D11Texture2D* texture2D = (ID3D11Texture2D*)texture;
                D3D11_TEXTURE2D_DESC my_image_desc;
                texture2D->GetDesc(&my_image_desc);
                *out_texture = textureView;

                if (out_width)
                {
                    *out_width = (int)my_image_desc.Width;
                }

                if (out_height)
                {
                    *out_height = (int)my_image_desc.Height;
                }

                texture2D->Release();
                return true;
            }
        }
        break;
    }
    }

    printf("[UIContext] Error reading texture! (0x%08x)\n", hr);
    return false;
}
