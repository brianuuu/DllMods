#include "UIContext.h"

HWND UIContext::window;
IUnknown* UIContext::device;
Backend UIContext::backend;
ImFont* UIContext::font;

std::string UIContext::modDirectoryPath;
std::string UIContext::imGuiIniPath = "ImGui.ini";

std::string UIContext::getModDirectoryPath()
{
    return modDirectoryPath;
}

void UIContext::setModDirectoryPath(const std::string& value)
{
    modDirectoryPath = value;
    imGuiIniPath = modDirectoryPath + "/ImGui.ini";
}

bool UIContext::isInitialized()
{
    return window && device;
}

Backend UIContext::getBackend()
{
    return backend;
}

void UIContext::initialize(HWND window, IUnknown* device)
{
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
    io.MouseDrawCursor = true;
    io.IniFilename = imGuiIniPath.c_str();
    io.DisplaySize = { (float)*BACKBUFFER_WIDTH, (float)*BACKBUFFER_HEIGHT };

    static float c_fontSize = 24.0f;
    static float c_labelDisplayTime = 5.0f;

    const float fontSize = max(c_fontSize, *BACKBUFFER_HEIGHT / 1080.0f * c_fontSize);

    if ((font = io.Fonts->AddFontFromFileTTF((modDirectoryPath + "/Fonts/DroidSans.ttf").c_str(), fontSize, nullptr, io.Fonts->GetGlyphRangesDefault())) == nullptr)
    {
        DebugDrawText::log("Failed to load DroidSans.ttf", c_labelDisplayTime);
        font = io.Fonts->AddFontDefault();
    }

    ImFontConfig fontConfig;
    fontConfig.MergeMode = true;

    if (!io.Fonts->AddFontFromFileTTF((modDirectoryPath + "/Fonts/DroidSansJapanese.ttf").c_str(), fontSize * 1.25f, &fontConfig, io.Fonts->GetGlyphRangesJapanese()))
        DebugDrawText::log("Failed to load DroidSansJapanese.ttf", c_labelDisplayTime);

    io.Fonts->Build();
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
    io.DisplaySize = { (float)*BACKBUFFER_WIDTH, (float)*BACKBUFFER_HEIGHT };

    ImGui::NewFrame();
    DrawSoundTest();
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

boost::shared_ptr<Sonic::CObjectBase> m_spSingleton;
class CObjSoundTest : public Sonic::CObjectBase
{
private:
    boost::shared_ptr<Hedgehog::Mirage::CSingleElement> m_spModel;
    SElementInfo m_elementInfo;
    Hedgehog::Math::CVector m_Position;

public:
    CObjSoundTest
    (
        Hedgehog::Math::CVector const& _Position
    )
        : m_elementInfo(m_spModel)
        , m_Position(_Position)
    {
    }

    ~CObjSoundTest()
    {
    }

    void AddCallback
    (
        const Hedgehog::Base::THolder<Sonic::CWorld>& worldHolder,
        Sonic::CGameDocument* pGameDocument,
        const boost::shared_ptr<Hedgehog::Database::CDatabase>& spDatabase
    ) override
    {
        Sonic::CObjectBase::AddCallback(worldHolder, pGameDocument, spDatabase);

        Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
        pGameDocument->AddUpdateUnit("0", this);

        m_spMatrixNodeTransform->m_Transform.SetPosition(m_Position);
        m_spMatrixNodeTransform->NotifyChanged();

        AddRenderable("cmn_obj_balloonB_HD", m_elementInfo, m_spMatrixNodeTransform, spDatabase);
    }

    bool ProcessMessage
    (
        Hedgehog::Universe::Message& message,
        bool flag
    ) override
    {
        if (flag)
        {
            if (std::strstr(message.GetType(), "MsgRestartStage") != nullptr
             || std::strstr(message.GetType(), "MsgStageClear") != nullptr)
            {
                Kill();
                return true;
            }
        }

        return Sonic::CObjectBase::ProcessMessage(message, flag);
    }

    void UpdateParallel
    (
        const Hedgehog::Universe::SUpdateInfo& updateInfo
    ) override
    {
        
    }

    void Kill()
    {
        SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
    }
};

LanguageType* GetVoiceLanguageType()
{
    uint32_t voiceOverAddress = Common::GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0x10 });
    return (LanguageType*)voiceOverAddress;
}

void SetMusicVolume(float volume)
{
    uint32_t* pCSoundModuleManager = *(uint32_t**)0x1E77290;
    FUNCTION_PTR(void, __thiscall, SetMusicVolume, 0x75EEF0, void* This, int a2, float volume);

    SetMusicVolume(pCSoundModuleManager, 0, volume * 0.01f * 0.63f);
    *(uint8_t*)((uint32_t)GetVoiceLanguageType() + 1) = (uint8_t)volume;
}

void SetEffectVolume(float volume)
{
    uint32_t* pCSoundModuleManager = *(uint32_t**)0x1E77290;
    FUNCTION_PTR(void, __thiscall, SetMusicVolume, 0x75EEF0, void* This, int a2, float volume);

    SetMusicVolume(pCSoundModuleManager, 1, volume * 0.01f * 0.63f);
    *(uint8_t*)((uint32_t)GetVoiceLanguageType() + 2) = (uint8_t)volume;
}

static SharedPtrTypeless soundHandleSfx;
static SharedPtrTypeless soundHandleObj;
void UIContext::DrawSoundTest()
{
    bool const playerExist = *PLAYER_CONTEXT;
    ImGui::Begin("Sound Test:");
    {
        ImGui::Text("Volume");

        static float bgm = 100;
        static float sfx = 100;
        if (ImGui::DragFloat("BGM", &bgm, 0.1f, 0.0f, 100.0f))
        {
            SetMusicVolume(bgm);
        }
        if (ImGui::DragFloat("SFX", &sfx, 0.1f, 0.0f, 100.0f))
        {
            SetEffectVolume(sfx);
        }

        ImGui::NewLine();
        ImGui::Separator();
        //------------------------------------------------------------------
        ImGui::Text("Sound Effect");

        static int cueIDsfx = 4002004; // checkpoint
        static int flag = 1;
        ImGui::InputInt("Cue ID##sfx", &cueIDsfx);
        ImGui::InputInt("Flag", &flag);

        static char buffer[128];
        ImGui::InputText("Aisac Name##sfx", buffer, 128);
        static float slider = 1.0f;
        if (ImGui::SliderFloat("Value##sliderSfx", &slider, 0.0f, 1.0f) && soundHandleSfx && !std::string(buffer).empty())
        {
            FUNCTION_PTR(void*, __thiscall, SetAisac, 0x763D50, void* This, hh::base::CSharedString const& name, float value);
            SetAisac(soundHandleSfx.get(), buffer, slider);
        }

        if (ImGui::Button("Play System Sound"))
        {
            Common::PlaySoundStatic(soundHandleSfx, cueIDsfx);
        }

        if (playerExist)
        {
            ImGui::SameLine();
            if (ImGui::Button("Play Sonic Context"))
            {
                Common::SonicContextPlaySound(soundHandleSfx, cueIDsfx, flag);
            }

            ImGui::SameLine();
            if (ImGui::Button("Play Sonic Voice"))
            {
                Common::SonicContextPlayVoice(soundHandleSfx, cueIDsfx, 0);
            }
        }

        if (ImGui::Button("Stop Sound##sfx"))
        {
            soundHandleSfx.reset();
        }

        ImGui::NewLine();
        ImGui::Separator();
        //------------------------------------------------------------------
        ImGui::Text("Stage Music:");

        static constexpr size_t textSize = 50;
        static char synthName[textSize] = "Speed_Highway_Generic1";
        static float fadeTime = 1.0f;
        ImGui::InputText("Synth Name", synthName, textSize);
        ImGui::InputFloat("Fade Time", &fadeTime);

        if (playerExist)
        {
            if (ImGui::Button("Play Music"))
            {
                Common::PlayStageMusic(synthName, fadeTime);
            }
        }

        ImGui::NewLine();
        ImGui::Separator();
        //------------------------------------------------------------------
        ImGui::Text("Object Test:");
        if (playerExist)
        {
            if (ImGui::Button("Spawn Object"))
            {
                if (m_spSingleton)
                {
                    ((CObjSoundTest*)m_spSingleton.get())->Kill();
                }

                auto* context = Sonic::Player::CPlayerSpeedContext::GetInstance();
                hh::math::CVector pos = context->m_spMatrixNode->m_Transform.m_Position;
                pos.y() += 0.5f;
                m_spSingleton = boost::make_shared<CObjSoundTest>(pos);
                Sonic::CGameDocument::GetInstance()->AddGameObject(m_spSingleton);
            }

            static int cueIDobj = 4002006; // goalring_lp
            ImGui::InputInt("Cue ID##obj", &cueIDobj);
            if (m_spSingleton)
            {
                if (ImGui::Button("Play Sound"))
                {
                    Common::ObjectPlaySound(m_spSingleton.get(), cueIDobj, soundHandleObj);
                }

                ImGui::SameLine();
                if (ImGui::Button("Stop Sound##obj"))
                {
                    soundHandleObj.reset();
                }

                static char bufferObj[128];
                ImGui::InputText("Aisac Name##obj", bufferObj, 128);
                static float sliderObj = 1.0f;
                if (ImGui::SliderFloat("Value##sliderObj", &sliderObj, 0.0f, 1.0f) && soundHandleObj && !std::string(bufferObj).empty())
                {
                    FUNCTION_PTR(void*, __thiscall, SetAisac, 0x763D50, void* This, hh::base::CSharedString const& name, float value);
                    SetAisac(soundHandleObj.get(), bufferObj, sliderObj);
                }
            }
        }
    }
    ImGui::End();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT UIContext::wndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
}
