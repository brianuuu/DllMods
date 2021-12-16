#include "SubtitleUI.h"
#include "Application.h"
#include "UIContext.h"
#include "Configuration.h"

CaptionData SubtitleUI::m_captionData;
void __declspec(naked) addCaption_COmochaoFollow()
{
    static uint32_t sub_6B02B0 = 0x6B02B0;
    static uint32_t returnAddress = 0x461407;
    __asm
    {
        call    [sub_6B02B0]

        push    eax
        push    edx

        // not cutscene
        push    0

        // duration
        mov     ecx, [esp + 0x1C + 0xC]
        push    ecx

        // caption
        mov     ecx, [eax]
        push    ecx

        // this (COmochao)
        mov     ecx, [esi + 8]
        push    ecx

        call    SubtitleUI::addCaptionImpl
        add     esp, 0x10

        pop     edx
        pop     eax

        jmp     [returnAddress]
    }
}

void __declspec(naked) addCaption_MsgOmochao_PlayVoice()
{
    static uint32_t sub_6B02B0 = 0x6B02B0;
    static uint32_t returnAddress = 0x1155E86;
    __asm
    {
        call    [sub_6B02B0]

        push    eax
        push    edx

        // not cutscene
        push    0

        // duration
        mov     ecx, [esp + 0x1C + 0xC]
        push    ecx

        // caption
        mov     ecx, [eax]
        push    ecx

        // this (COmochao)
        push    ebx

        call    SubtitleUI::addCaptionImpl
        add     esp, 0x10

        pop     edx
        pop     eax

        jmp     [returnAddress]
    }
}

float cutsceneCaptionDuration = 0.0f;
void __declspec(naked) addCaption_GetCutsceneDuration()
{
    static uint32_t sub_6AE910 = 0x6AE910;
    static uint32_t returnAddress = 0xB16E71;
    __asm
    {
        lea     eax, [cutsceneCaptionDuration]
        mov     edx, [esp + 20h]
        movss   xmm0, dword ptr [edx + 4h]
        movss   [eax], xmm0

        mov     byte ptr[ecx + 44h], 0 // force display immediately
        call    [sub_6AE910]
        mov     cutsceneCaptionDuration, 0
        jmp     [returnAddress]
    }
}

void __declspec(naked) addCaption_Cutscene()
{
    static uint32_t returnAddress = 0x6AE0A3;
    __asm
    {
        cmp     cutsceneCaptionDuration, 0
        jz      jump

        push    eax
        push    ebp

        push    1
        push    cutsceneCaptionDuration
        push    eax
        push    0
        call    SubtitleUI::addCaptionImpl
        add     esp, 0x10

        pop     ebp
        pop     eax

        // original function
        jump:
        mov     edx, [eax + 8]
        sub     edx, [eax + 4]
        jmp     [returnAddress]
    }
}

HOOK(bool, __stdcall, SubtitleUI_CEventSceneStart, 0xB238C0, void* a1)
{
    // Reset when cutscene starts
    SubtitleUI::m_captionData.clear();
    return originalSubtitleUI_CEventSceneStart(a1);
}

HOOK(bool, __fastcall, SubtitleUI_CEventSceneAdvance, 0xB24A40, uint32_t* This, void* Edx, int a2)
{
    // Reset when cutscene ends
    if (This[73] == 3)
    {
        SubtitleUI::m_captionData.clear();
    }
    return originalSubtitleUI_CEventSceneAdvance(This, Edx, a2);
}

void SubtitleUI::applyPatches()
{
    if (initFontDatabase())
    {
        // Omochao subtitle
        WRITE_JUMP(0x461402, addCaption_COmochaoFollow);
        WRITE_JUMP(0x1155E81, addCaption_MsgOmochao_PlayVoice);
        WRITE_JUMP(0x11F8813, (void*)0x11F8979);

        // Cutscene subtitle
        WRITE_MEMORY(0xB16D7A, uint8_t, 0); // disable original textbox
        WRITE_JUMP(0xB16E6C, addCaption_GetCutsceneDuration);
        WRITE_JUMP(0x6AE09D, addCaption_Cutscene);
        INSTALL_HOOK(SubtitleUI_CEventSceneStart);
        INSTALL_HOOK(SubtitleUI_CEventSceneAdvance);
    }
}

std::map<uint32_t, wchar_t> SubtitleUI::m_fontDatabase;
bool SubtitleUI::initFontDatabase()
{
    std::ifstream database(Application::getModDirWString() + L"Fonts\\FontDatabase.txt");
    if (database.is_open())
    {
        std::stringstream ss;
        ss << database.rdbuf();
        database.close();

        uint32_t key = 0x82;
        std::wstring str = Common::multiByteToWideChar(ss.str().c_str());
        for (wchar_t const& c : str)
        {
            m_fontDatabase[key] = c;
            key++;
        }

        return true;
    }

    MessageBox(nullptr, TEXT("Failed to load font database, reverting to in-game textbox."), TEXT("Sonic 06 HUD"), MB_ICONWARNING);
    return false;
}

void __cdecl SubtitleUI::addCaptionImpl(uint32_t* owner, uint32_t* caption, float duration, bool isCutscene)
{
    // Caption disabled
    if ((*(uint8_t*)Common::GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0x18 }) & 0x10) == 0)
    {
        return;
    }

    if (m_captionData.m_owner != owner || isCutscene)
    {
        m_captionData.clear();
    }
    m_captionData.m_owner = owner;
    m_captionData.m_isCutscene = isCutscene && !Common::CheckCurrentStage("blb");

    Caption newCaption;
    newCaption.m_duration = duration;

    // Read caption and convert to string
    uint32_t const length = (caption[2] - caption[1]) / 4;
    uint32_t* captionList = (uint32_t*)caption[1];

    bool isJapanese = *(uint8_t*)Common::GetMultiLevelAddress(0x1E66B34, { 0x8 }) == 1;
    bool adjustLineBreak = !Configuration::m_usingSTH2006Project && !isJapanese && !m_captionData.m_isCutscene;

    std::wstring str;
    int rowLength = 0;
    int linebreakCount = 0;
    for (uint32_t i = 0; i < length; i++)
    {
        uint32_t const key = captionList[i];
        if (key >= 0x64 && key <= 0x6D)
        {
            // Button at the beginning of a line, add dummy text
            if (str.empty())
            {
                newCaption.m_captions.push_back("");
            }
            else
            {
                newCaption.m_captions.push_back(Common::wideCharToMultiByte(str.c_str()));
                str.clear();
            }
            newCaption.m_buttons[newCaption.m_captions.size() - 1] = (CaptionButtonType)(key - 0x64);
            rowLength += 2;
        }
        else if (m_fontDatabase.count(key))
        {
            if (key == 0x82)
            {
                if (adjustLineBreak && linebreakCount < 2 && rowLength > (m_captionData.m_isCutscene ? 72 : 50))
                {
                    // Do line break manually
                    str += L'\n';
                    rowLength = 0;
                    linebreakCount++;

                    newCaption.m_captions.push_back(Common::wideCharToMultiByte(str.c_str()));
                    str.clear();
                }
                else
                {
                    str += L"  ";
                    rowLength += 2;
                }
            }
            else
            {
                str += m_fontDatabase[key];
                rowLength++;
            }
        }
        else if (key == 0)
        {
            if (adjustLineBreak)
            {
                // Don't do line break here
                if (!str.empty() && str.back() != L' ')
                {
                    str += L"  ";
                    rowLength += 2;
                }
            }
            else
            {
                str += L'\n';
                rowLength = 0;

                newCaption.m_captions.push_back(Common::wideCharToMultiByte(str.c_str()));
                str.clear();
            }
        }
        else if (key != 0x4)
        {
            str += L'?';
            rowLength++;
        }
    }

    // Push remaining text
    if (!str.empty())
    {
        newCaption.m_captions.push_back(Common::wideCharToMultiByte(str.c_str()));
    }

    m_captionData.m_captions.push_back(newCaption);
    m_captionData.m_bypassLoading = Common::IsAtLoadingScreen();
}

bool CaptionData::init()
{
    std::wstring const dir = Application::getModDirWString();
    bool success = true;
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Textbox.dds").c_str(), &m_textbox);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Button_A.dds").c_str(), &m_buttonA);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Button_B.dds").c_str(), &m_buttonB);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Button_X.dds").c_str(), &m_buttonX);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Button_Y.dds").c_str(), &m_buttonY);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Button_LB.dds").c_str(), &m_buttonLB);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Button_LT.dds").c_str(), &m_buttonLT);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Button_RB.dds").c_str(), &m_buttonRB);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Button_RT.dds").c_str(), &m_buttonRT);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Button_Start.dds").c_str(), &m_buttonStart);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Button_Back.dds").c_str(), &m_buttonBack);

    if (!success)
    {
        m_captions.clear();
        MessageBox(nullptr, TEXT("Failed to load assets for custom 06 textbox, reverting to in-game textbox."), TEXT("Sonic 06 HUD"), MB_ICONWARNING);
    }

    return success;
}

void SubtitleUI::draw()
{
    if (m_captionData.m_bypassLoading)
    {
        // No longer at loading screen
        if (!Common::IsAtLoadingScreen())
        {
            m_captionData.m_bypassLoading = false;
        }
    }
    else if (Common::IsAtLoadingScreen())
    {
        // At loading screen, clear all
        m_captionData.clear();
        cutsceneCaptionDuration = 0.0f;
        return;
    }

    if (!m_captionData.m_captions.empty())
    {
        Caption& caption = m_captionData.m_captions.front();
        float sizeX = *BACKBUFFER_WIDTH * 890.0f / 1280.0f;
        float sizeY = *BACKBUFFER_HEIGHT * 170.0f / 720.0f;
        float posX = 0.143f;
        float posY = 0.6958f;
        float alpha = 1.0f;

        static bool visible = true;
        if (m_captionData.m_isCutscene)
        {
            // Don't draw first frame since position is stale
            alpha = (m_captionData.m_timer == 0.0f) ? 0.0f : 1.0f;
        }
        else
        {
            ImGui::Begin("Textbox", &visible, UIContext::m_hudFlags);
            {
                // Fade in and out
                float frame1 = m_captionData.m_timer * 60.0f;
                float frame2 = (caption.m_duration - m_captionData.m_timer) * 60.0f;
                if (frame1 < 5.0f)
                {
                    posY += 0.03476f * (5.0f - frame1);
                    alpha = 0.2f * frame1;
                }
                else if (frame2 < 5.0f)
                {
                    posY += 0.03476f * (5.0f - frame2);
                    alpha = 0.2f * frame2;
                }

                ImGui::SetWindowFocus();
                ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
                ImGui::Image(m_captionData.m_textbox, ImVec2(sizeX, sizeY), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, alpha * 0.9f));
                ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * posX, *BACKBUFFER_HEIGHT * posY));
            }
            ImGui::End();
        }

        ImGui::Begin("Caption", &visible, UIContext::m_hudFlags);
        {
            ImGui::SetWindowFocus();
            ImGui::SetWindowSize(ImVec2(sizeX, sizeY));

            if (m_captionData.m_isCutscene)
            {
                drawCaptions(caption, alpha, true, true);
                float textWidth = drawCaptions(caption, alpha, false, true);
                ImGui::SetWindowPos(ImVec2((*BACKBUFFER_WIDTH - textWidth) * 0.5f - 10.0f, *BACKBUFFER_HEIGHT * 0.8502f - 10.0f));
            }
            else
            {
                drawCaptions(caption, alpha);
                ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * 0.2023f, *BACKBUFFER_HEIGHT * (posY + 0.047f)));
            }
        }
        ImGui::End();

        m_captionData.m_timer += m_captionData.m_isCutscene ? Application::getDeltaTime() : Application::getHudDeltaTime();
        if (m_captionData.m_timer > caption.m_duration)
        {
            m_captionData.m_captions.pop_front();
            m_captionData.m_timer = 0.0f;

            // Finished
            if (m_captionData.m_captions.empty())
            {
                m_captionData.clear();
            }
        }
    }
}

float SubtitleUI::drawCaptions(Caption const& caption, float alpha, bool isShadow, bool isCutscene)
{
    float maxWidth = 0.0f;
    float currentWidth = 0.0f;

    float const offset = 10.0f;
    float const shadowPosX = *BACKBUFFER_WIDTH * 0.002f;
    if (isShadow)
    {
        ImGui::SetCursorPos(ImVec2(shadowPosX + offset, *BACKBUFFER_HEIGHT * 0.0036f + offset));
    }
    else
    {
        ImGui::SetCursorPos(ImVec2(offset, offset));
    }

    for (uint32_t i = 0; i < caption.m_captions.size(); i++)
    {
        std::string const& str = caption.m_captions[i];
        float color = isShadow ? 0.0f : 1.0f;

        ImGui::TextColored(ImVec4(color, color, color, alpha * 0.9f), str.c_str());
        currentWidth += ImGui::CalcTextSize(str.c_str()).x;

        if (caption.m_buttons.count(i))
        {
            float buttonSizeX = 28.0f;
            switch (caption.m_buttons.at(i))
            {
            case CBT_LB:
            case CBT_RB:
            case CBT_LT:
            case CBT_RT:
                buttonSizeX = 56.0f;
                break;
            }

            PDIRECT3DTEXTURE9* texture = nullptr;
            switch (caption.m_buttons.at(i))
            {
            case CBT_A:     texture = &m_captionData.m_buttonA;      break;
            case CBT_B:     texture = &m_captionData.m_buttonB;      break;
            case CBT_X:     texture = &m_captionData.m_buttonX;      break;
            case CBT_Y:     texture = &m_captionData.m_buttonY;      break;
            case CBT_LB:    texture = &m_captionData.m_buttonLB;     break;
            case CBT_RB:    texture = &m_captionData.m_buttonRB;     break;
            case CBT_LT:    texture = &m_captionData.m_buttonLT;     break;
            case CBT_RT:    texture = &m_captionData.m_buttonRT;     break;
            case CBT_Start: texture = &m_captionData.m_buttonStart;  break;
            case CBT_Back:  texture = &m_captionData.m_buttonBack;   break;
            }

            ImGui::SameLine();
            if (texture)
            {
                ImGui::Image(*texture, ImVec2(*BACKBUFFER_WIDTH * buttonSizeX / 1280.0f, *BACKBUFFER_HEIGHT * 28.0f / 720.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, isShadow ? 0.0f : alpha));
                currentWidth += buttonSizeX;
            }
        }

        if (!str.empty() && str.back() == '\n')
        {
            ImGui::SetCursorPosX(isShadow ? shadowPosX + offset : offset);
            if (!isCutscene)
            {
                // y-spacing is slightly larger for gameplay dialog
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + *BACKBUFFER_HEIGHT * 0.01f);
            }

            maxWidth = max(maxWidth, currentWidth);
            currentWidth = 0.0f;
        }
        else
        {
            ImGui::SameLine();
        }
    }

    maxWidth = max(maxWidth, currentWidth);
    return maxWidth;
}
