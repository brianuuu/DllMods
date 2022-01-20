#include "SubtitleUI.h"
#include "Application.h"
#include "UIContext.h"
#include "Configuration.h"

CaptionData SubtitleUI::m_captionData;

void SubtitleUI::applyPatches()
{
    
}

void __cdecl SubtitleUI::addCaption(std::vector<std::string> const& captions, std::string const& speaker)
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

    m_captionData.m_speaker = speaker;
    m_captionData.m_bypassLoading = Common::IsAtLoadingScreen();
}

bool CaptionData::init()
{
    std::wstring const dir = Application::getModDirWString();
    bool success = UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Textbox.dds").c_str(), &m_textbox);

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
        if (m_captionData.m_frame >= 5.0f && padState->IsTapped(Sonic::EKeyState::eKeyState_A))
        {
            if (m_captionData.m_captions.size() == 1)
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
