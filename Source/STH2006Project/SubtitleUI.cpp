#include "SubtitleUI.h"
#include "Application.h"
#include "UIContext.h"
#include "Configuration.h"
#include "MissionManager.h"

CaptionData SubtitleUI::m_captionData;

void SubtitleUI::applyPatches()
{
    
}

void __cdecl SubtitleUI::addCaption(std::vector<std::string> const& captions, std::string const& speaker, int rejectDialogSize)
{
    m_captionData.clear();
    for (std::string caption : captions)
    {
        caption = std::regex_replace(caption, std::regex(" "), "  ");

        // Split full caption into rows
        Caption newCaption;
        std::string delimiter = "\\n";
        size_t pos = 0;
        std::string token;
        while ((pos = caption.find(delimiter)) != std::string::npos)
        {
            newCaption.m_captions.push_back(caption.substr(0, pos));
            caption.erase(0, pos + delimiter.length());
        }
        newCaption.m_captions.push_back(caption);
        m_captionData.m_captions.push_back(newCaption);
    }

    m_captionData.m_rejectDialogSize = rejectDialogSize;
    m_captionData.m_speaker = speaker;
    m_captionData.m_bypassLoading = Common::IsAtLoadingScreen();
}

bool CaptionData::init()
{
    std::wstring const dir = Application::getModDirWString();
    bool success = true;
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Textbox.dds").c_str(), &m_textbox);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\YesNoBox.dds").c_str(), &m_acceptbox);

    if (!success)
    {
        m_captions.clear();
        MessageBox(nullptr, TEXT("Failed to load assets for custom 06 textbox!"), TEXT("STH2006 Project"), MB_ICONWARNING);
        exit(-1);
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
        clearDraw();
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
        ImGui::Begin("Textbox", &visible, UIContext::m_hudFlags);
        {
            // Fade in and out
            if (m_captionData.m_frame < 5.0f)
            {
                posY += 0.03476f * (5.0f - m_captionData.m_frame);
                alpha = 0.2f * m_captionData.m_frame;
            }

            ImGui::SetWindowFocus();
            ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
            ImGui::Image(m_captionData.m_textbox, ImVec2(sizeX, sizeY), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, alpha * 0.9f));
            ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * posX, *BACKBUFFER_HEIGHT * posY));
        }
        ImGui::End();

        ImGui::Begin("Speaker", &visible, UIContext::m_hudFlags);
        {
            ImGui::SetWindowFocus();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, alpha * 0.9f), m_captionData.m_speaker.c_str());
            ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * 0.179f, *BACKBUFFER_HEIGHT * (posY - 0.0323f)));
        }
        ImGui::End();

        ImGui::Begin("Caption", &visible, UIContext::m_hudFlags);
        {
            ImGui::SetWindowFocus();
            ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
            drawCaptions(caption, alpha);
            ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * 0.2023f, *BACKBUFFER_HEIGHT * (posY + 0.047f)));
        }
        ImGui::End();

        if (m_captionData.m_acceptDialogShown)
        {
            float sizeX2 = *BACKBUFFER_WIDTH * 312.0f / 1920.0f;
            float sizeY2 = *BACKBUFFER_HEIGHT * 215.0f / 1080.0f;
            ImGui::Begin("YesNoBox", &visible, UIContext::m_hudFlags);
            {
                ImGui::SetWindowFocus();
                ImGui::SetWindowSize(ImVec2(sizeX2, sizeY2));
                ImGui::Image(m_captionData.m_acceptbox, ImVec2(sizeX2, sizeY2));
                ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * 0.794f, *BACKBUFFER_HEIGHT * 0.723f));
            }
            ImGui::End();

            ImGui::Begin("MissionYes", &visible, UIContext::m_hudFlags);
            {
                bool isJapanese = *(uint8_t*)Common::GetMultiLevelAddress(0x1E66B34, { 0x8 }) == 1;
                float blueGreenFactor = (m_captionData.m_yesNoColorTime < 0.5f) ? (1.0f - m_captionData.m_yesNoColorTime * 2.0f) : (m_captionData.m_yesNoColorTime - 0.5f) * 2.0f;
                ImVec4 color(1.0f, blueGreenFactor, blueGreenFactor, 1.0f);

                ImGui::SetWindowFocus();
                ImGui::SetWindowSize(ImVec2(sizeX2, sizeY2));
                ImGui::TextColored(MissionManager::m_missionAccept ? color : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), isJapanese ? u8"はい" : "Yes");
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + *BACKBUFFER_HEIGHT * 0.018f);
                ImGui::TextColored(!MissionManager::m_missionAccept ? color : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), isJapanese ? u8"いいえ" : "No");
                ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * 0.852f, *BACKBUFFER_HEIGHT * 0.773f));
            }
            ImGui::End();

            m_captionData.m_yesNoColorTime += Application::getHudDeltaTime();
            if (m_captionData.m_yesNoColorTime > 1.0f)
            {
                m_captionData.m_yesNoColorTime -= 1.0f;
            }
        }

        if (m_captionData.m_isFadeIn && m_captionData.m_frame < 5.0f)
        {
            // Fade in
            m_captionData.m_frame += Application::getHudDeltaTime() * 60.0f;
            Common::ClampFloat(m_captionData.m_frame, 0.0f, 5.0f);
        }
        else if (!m_captionData.m_isFadeIn && m_captionData.m_frame > 0.0f)
        {
            // Fade out
            m_captionData.m_frame -= Application::getHudDeltaTime() * 60.0f;
            Common::ClampFloat(m_captionData.m_frame, 0.0f, 5.0f);
        }

        // Goto next dialog when pressing A
        Sonic::SPadState* padState = Sonic::CInputState::GetPadState();
        if (m_captionData.m_frame >= 5.0f && (padState->IsTapped(Sonic::EKeyState::eKeyState_A) || padState->IsTapped(Sonic::EKeyState::eKeyState_X)))
        {
            int size = m_captionData.m_captions.size();
            if (!m_captionData.m_acceptDialogShown && size == m_captionData.m_rejectDialogSize + 1)
            {
                // Show YES/NO box
                m_captionData.m_acceptDialogShown = true;
            }
            else
            {
                if (m_captionData.m_acceptDialogShown)
                {
                    // Play sfx & close YES/NO box
                    static SharedPtrTypeless soundHandle;
                    Common::PlaySoundStatic(soundHandle, 1000005);
                    m_captionData.m_acceptDialogShown = false;

                    if (MissionManager::m_missionAccept)
                    {
                        // Accept mission
                        m_captionData.clear();
                        return;
                    }
                }

                if (size == 1)
                {
                    // Switch to fade out
                    m_captionData.m_isFadeIn = false;
                }
                else
                {
                    // Next dialog
                    m_captionData.m_captions.pop_front();
                }
            }
        }

        // Yes/No dialog switching
        if (m_captionData.m_acceptDialogShown)
        {
            bool playSound = false;
            if (MissionManager::m_missionAccept && (padState->LeftStickVertical < -0.5f || padState->IsTapped(Sonic::EKeyState::eKeyState_DpadDown)))
            {
                playSound = true;
                MissionManager::m_missionAccept = false;
            }
            else if(!MissionManager::m_missionAccept && (padState->LeftStickVertical > 0.5f || padState->IsTapped(Sonic::EKeyState::eKeyState_DpadUp)))
            {
                playSound = true;
                MissionManager::m_missionAccept = true;
            }

            if (playSound)
            {
                m_captionData.m_yesNoColorTime = 0.0f;
                static SharedPtrTypeless soundHandle;
                Common::PlaySoundStatic(soundHandle, 1000004);
            }
        }

        // Finished
        if (!m_captionData.m_isFadeIn && m_captionData.m_frame == 0.0f)
        {
            m_captionData.clear();
        }
    }
}

void SubtitleUI::clearDraw()
{
    m_captionData.clear();
}

void SubtitleUI::drawCaptions(Caption const& caption, float alpha)
{
    float const offset = 10.0f;
    ImGui::SetCursorPos(ImVec2(offset, offset));

    for (uint32_t i = 0; i < caption.m_captions.size(); i++)
    {
        std::string const& str = caption.m_captions[i];
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, alpha * 0.9f), str.c_str());
        ImGui::SetCursorPosX(offset);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + *BACKBUFFER_HEIGHT * 0.01f);
    }
}
