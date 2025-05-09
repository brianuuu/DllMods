#include "SubtitleUI.h"
#include "Application.h"
#include "UIContext.h"
#include "Configuration.h"
#include "MissionManager.h"

SharedPtrTypeless SubtitleUI::m_subtitleSfx;
CaptionData SubtitleUI::m_captionData;
std::set<SubtitleUI::DialogCallbackFunc> SubtitleUI::m_acceptCallbacks;
std::set<SubtitleUI::DialogCallbackFunc> SubtitleUI::m_finishCallbacks;

void SubtitleUI::applyPatches()
{
    
}

float SubtitleUI::addSubtitle(mst::TextEntry const& entry, std::vector<float> const& durationOverrides)
{
    m_captionData.clear();
    int tagIndex = 0;
    float totalDuration = 0.0f;
    for (int i = 0; i < entry.m_subtitles.size(); i++)
    {
        std::wstring const& wsubtitle = entry.m_subtitles.at(i);
        std::string subtitle = Common::wideCharToMultiByte(wsubtitle.c_str());
        subtitle = std::regex_replace(subtitle, std::regex(" "), "  ");

        Subtitle newSubtitle;

        // Get duration from sound cue or override
        if (i < durationOverrides.size() && durationOverrides[i] > 0.0f)
        {
            newSubtitle.m_duration = durationOverrides[i];
        }
        
        // Front tag can be voice
        if (tagIndex < entry.m_tags.size() && subtitle.front() == '$')
        {
            // is this a sound or a picture?
            std::string synthName = entry.m_tags.at(tagIndex);
            if (synthName.find("sound") != std::string::npos)
            {
                // get cueID from synth
                synthName = synthName.substr(synthName.find('(') + 1);
                synthName.pop_back();
                newSubtitle.m_cueID = Common::GetSoundCueFromSynth(synthName.c_str());

                // Get duration from sound cue if not overritten
                if (newSubtitle.m_duration == 0.0f && newSubtitle.m_cueID)
                {
                    float const duration = Common::GetSoundCueDuration(newSubtitle.m_cueID);
                    if (duration > 0.0f)
                    {
                        newSubtitle.m_duration = duration;
                    }

                    if (i == 0)
                    {
                        m_subtitleSfx.reset();
                        Common::PlaySoundStatic(m_subtitleSfx, newSubtitle.m_cueID);
                    }
                }

                tagIndex++;
                subtitle.erase(0, 1);
            }
        }

        // Last subtitle last a little longer
        if (i == entry.m_subtitles.size() - 1)
        {
            newSubtitle.m_duration += 1.0f;
        }

        totalDuration += newSubtitle.m_duration;

        // helper function to find button tags of a string
        auto fnSplitButtonTags = [&newSubtitle, &tagIndex, &entry](std::string& subtitle)
        {
            size_t pos = 0;
            while ((pos = subtitle.find('$')) != std::string::npos)
            {
                if (tagIndex < entry.m_tags.size())
                {
                    std::string buttonName = entry.m_tags.at(tagIndex++);
                    buttonName = buttonName.substr(buttonName.find('(') + 1);
                    buttonName.pop_back();
                    newSubtitle.m_buttons[newSubtitle.m_subtitles.size()] = getButtonTypeFromTag(buttonName);
                }
                else
                {
                    return;
                }

                std::string const buttonSplit = subtitle.substr(0, pos); // NOT include $
                subtitle.erase(0, pos + 1);
                newSubtitle.m_subtitles.push_back(buttonSplit);
            }
        };

        // Split full caption into rows
        size_t pos = 0;
        while ((pos = subtitle.find('\n')) != std::string::npos)
        {
            std::string split = subtitle.substr(0, pos + 1); // include \n
            fnSplitButtonTags(split);
            newSubtitle.m_subtitles.push_back(split);
            subtitle.erase(0, pos + 1);
        }

        // Last string
        fnSplitButtonTags(subtitle);
        newSubtitle.m_subtitles.push_back(subtitle);
        m_captionData.m_subtitles.push_back(newSubtitle);
    }

    m_captionData.m_bypassLoading = Common::IsAtLoadingScreen();

    // Prevent dialog overlapping
    S06HUD_API::CloseCaptionWindow();

    return totalDuration;
}

SubtitleButtonType SubtitleUI::getButtonTypeFromTag(std::string const& tag)
{
    if (tag == "button_a") return SBT_A;
    if (tag == "button_b") return SBT_B;
    if (tag == "button_x") return SBT_X;
    if (tag == "button_y") return SBT_Y;
    if (tag == "button_lb") return SBT_LB;
    if (tag == "button_rb") return SBT_RB;
    if (tag == "button_lt") return SBT_LT;
    if (tag == "button_rt") return SBT_RT;
    if (tag == "button_start") return SBT_Start;
    if (tag == "button_back") return SBT_Back;

    return SBT_A;
}

void __cdecl SubtitleUI::addCaption(std::vector<std::string> const& captions, std::string const& speaker, int acceptDialogSize, int rejectDialogSize)
{
    m_captionData.clear();
    for (std::string caption : captions)
    {
        caption = std::regex_replace(caption, std::regex(" "), "  ");

        // Split full caption into rows
        Caption newCaption;
        std::string delimiter = "\\n";
        size_t pos = 0;
        while ((pos = caption.find(delimiter)) != std::string::npos)
        {
            newCaption.m_captions.push_back(caption.substr(0, pos));
            caption.erase(0, pos + delimiter.length());
        }
        newCaption.m_captions.push_back(caption);
        m_captionData.m_captions.push_back(newCaption);
    }

    m_captionData.m_acceptDialogSize = acceptDialogSize;
    m_captionData.m_rejectDialogSize = rejectDialogSize;
    m_captionData.m_speaker = speaker;
    m_captionData.m_bypassLoading = Common::IsAtLoadingScreen();

    // Prevent dialog overlapping
    S06HUD_API::CloseCaptionWindow();
}

bool CaptionData::init()
{
    std::wstring const dir = Application::getModDirWString();
    bool success = true;
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Textbox.dds").c_str(), &m_textbox);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\YesNoBox.dds").c_str(), &m_acceptbox);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Arrow.dds").c_str(), &m_arrow);

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
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Button_LStick.dds").c_str(), &m_buttonLStick);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Button_RStick.dds").c_str(), &m_buttonRStick);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox\\Button_DPad.dds").c_str(), &m_buttonDPad);

    if (!success)
    {
        m_captions.clear();
        MessageBox(nullptr, TEXT("Failed to load assets for custom 06 textbox!"), TEXT("STH2006 Project"), MB_ICONWARNING);
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

    if (!m_captionData.m_subtitles.empty())
    {
        Subtitle& subtitle = m_captionData.m_subtitles.front();
        float sizeX = *BACKBUFFER_WIDTH * 890.0f / 1280.0f;
        float sizeY = *BACKBUFFER_HEIGHT * 170.0f / 720.0f;
        float posX = 0.143f;
        float posY = 0.6958f;
        float alpha = 1.0f;

        static bool visible = true;
        ImGui::Begin("Textbox", &visible, UIContext::m_hudFlags);
        {
            // Fade in and out
            float frame1 = m_captionData.m_timer * 60.0f;
            float frame2 = (subtitle.m_duration - m_captionData.m_timer) * 60.0f;
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

        ImGui::Begin("Caption", &visible, UIContext::m_hudFlags);
        {
            ImGui::SetWindowFocus();
            ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
            drawSubtitle(subtitle, alpha);
            ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * 0.2023f, *BACKBUFFER_HEIGHT * (posY + 0.047f)));
        }
        ImGui::End();

        m_captionData.m_timer += Application::getHudDeltaTime();
        if (m_captionData.m_timer > subtitle.m_duration)
        {
            m_captionData.m_subtitles.pop_front();
            m_captionData.m_timer = 0.0f;

            // Finished
            if (m_captionData.m_subtitles.empty())
            {
                m_captionData.clear();
            }
            else if (m_captionData.m_subtitles.front().m_cueID)
            {
                // play next sound
                m_subtitleSfx.reset();
                Common::PlaySoundStatic(m_subtitleSfx, m_captionData.m_subtitles.front().m_cueID);
            }
        }
    }
    else if (!m_captionData.m_captions.empty())
    {
        // caption system for HUB world
        Caption& caption = m_captionData.m_captions.front();
        float sizeX = *BACKBUFFER_WIDTH * 890.0f / 1280.0f;
        float sizeY = *BACKBUFFER_HEIGHT * 170.0f / 720.0f;
        float posX = 0.143f;
        float posY = 0.6958f;
        float alpha = 1.0f;

        float const dt = Application::getHudDeltaTime();
        float const df = dt * 60.0f;

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
            ImGui::SetCursorPos(ImVec2(sizeX, sizeY));
            ImGui::Text(" "); // dummy text to expand window size
            ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * 0.2023f, *BACKBUFFER_HEIGHT * (posY + 0.047f)));
        }
        ImGui::End();

        if (m_captionData.m_acceptDialogShown)
        {
            ImGui::Begin("YesNoBox", &visible, UIContext::m_hudFlags);
            {
                float sizeX2 = *BACKBUFFER_WIDTH * 312.0f / 1920.0f;
                float sizeY2 = *BACKBUFFER_HEIGHT * 215.0f / 1080.0f;
                ImGui::SetWindowFocus();
                ImGui::SetWindowSize(ImVec2(sizeX2, sizeY2));
                ImGui::Image(m_captionData.m_acceptbox, ImVec2(sizeX2, sizeY2));
                ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * 0.794f, *BACKBUFFER_HEIGHT * 0.723f));
            }
            ImGui::End();

            ImGui::Begin("MissionYes", &visible, UIContext::m_hudFlags);
            {
                bool isJapanese = Common::GetUILanguageType() == LT_Japanese;
                float blueGreenFactor = (m_captionData.m_yesNoColorTime < 0.5f) ? (1.0f - m_captionData.m_yesNoColorTime * 2.0f) : (m_captionData.m_yesNoColorTime - 0.5f) * 2.0f;
                ImVec4 color(1.0f, blueGreenFactor, blueGreenFactor, 1.0f);

                ImGui::SetWindowFocus();
                ImGui::TextColored(MissionManager::m_missionAccept ? color : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), isJapanese ? u8"はい" : "Yes");
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + *BACKBUFFER_HEIGHT * 0.018f);
                ImGui::TextColored(!MissionManager::m_missionAccept ? color : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), isJapanese ? u8"いいえ" : "No");
                ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * 0.852f, *BACKBUFFER_HEIGHT * 0.773f));
            }
            ImGui::End();

            ImGui::Begin("YesNoArrow", &visible, UIContext::m_hudFlags);
            {
                float sizeX3 = *BACKBUFFER_WIDTH * 27.0f / 2560.0f;
                float sizeY3 = *BACKBUFFER_HEIGHT * 50.0f / 1440.0f;
                float posX = 0.8183f;
                float posY = MissionManager::m_missionAccept ? 0.7711f : 0.8280f;

                float alpha1 = 0.6f;
                float alpha2 = 0.4f;
                float alpha3 = 0.2f;

                if (m_captionData.m_yesNoArrowFrame >= 6.0f && m_captionData.m_yesNoArrowFrame <= 12.0f)
                {
                    alpha1 = (m_captionData.m_yesNoArrowFrame <= 9.0f)
                        ? 0.6f + ((m_captionData.m_yesNoArrowFrame - 6.0f) * 0.4f / 3.0f)
                        : 1.0f - (m_captionData.m_yesNoArrowFrame - 9.0f) * 0.4f;
                }

                if (m_captionData.m_yesNoArrowFrame >= 3.0f && m_captionData.m_yesNoArrowFrame <= 9.0f)
                {
                    alpha2 = (m_captionData.m_yesNoArrowFrame <= 6.0f)
                        ? 0.4f + ((m_captionData.m_yesNoArrowFrame - 3.0f) * 0.6f / 3.0f)
                        : 1.0f - (m_captionData.m_yesNoArrowFrame - 6.0f) * 0.6f;
                }

                if (m_captionData.m_yesNoArrowFrame >= 0.0f && m_captionData.m_yesNoArrowFrame <= 6.0f)
                {
                    alpha3 = (m_captionData.m_yesNoArrowFrame <= 3.0f) 
                        ? 0.2f + (m_captionData.m_yesNoArrowFrame * 0.8f / 3.0f) 
                        : 1.0f - (m_captionData.m_yesNoArrowFrame - 3.0f) * 0.8f;
                }

                ImGui::SetWindowFocus();
                ImGui::SetWindowSize(ImVec2(sizeX3, sizeY3));
                ImGui::Image(m_captionData.m_arrow, ImVec2(sizeX3, sizeY3), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, alpha1));
                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() - *BACKBUFFER_HEIGHT * 0.015f);
                ImGui::Image(m_captionData.m_arrow, ImVec2(sizeX3, sizeY3), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, alpha2));
                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() - *BACKBUFFER_HEIGHT * 0.015f);
                ImGui::Image(m_captionData.m_arrow, ImVec2(sizeX3, sizeY3), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, alpha3));
                ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * posX, *BACKBUFFER_HEIGHT * posY));
            }
            ImGui::End();

            m_captionData.m_yesNoColorTime += dt;
            if (m_captionData.m_yesNoColorTime >= 1.0f)
            {
                m_captionData.m_yesNoColorTime -= 1.0f;
            }

            m_captionData.m_yesNoArrowFrame += df;
            if (m_captionData.m_yesNoArrowFrame >= 13.0f)
            {
                m_captionData.m_yesNoArrowFrame -= 13.0f;
            }
        }

        if (m_captionData.m_isFadeIn && m_captionData.m_frame < 5.0f)
        {
            // Fade in
            m_captionData.m_frame += df;
            Common::ClampFloat(m_captionData.m_frame, 0.0f, 5.0f);
        }
        else if (!m_captionData.m_isFadeIn && m_captionData.m_frame > 0.0f)
        {
            // Fade out
            m_captionData.m_frame -= df;
            Common::ClampFloat(m_captionData.m_frame, 0.0f, 5.0f);
        }

        // Goto next dialog when pressing A
        Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
        if (m_captionData.m_frame >= 5.0f && dt > 0.0f && 
            (
                padState->IsTapped(Sonic::EKeyState::eKeyState_A) ||
                padState->IsTapped(Sonic::EKeyState::eKeyState_Y) ||
                padState->IsTapped(Sonic::EKeyState::eKeyState_X) ||
                (!m_captionData.m_acceptDialogShown && padState->IsTapped(Sonic::EKeyState::eKeyState_B))
            ))
        {
            int size = m_captionData.m_captions.size();
            if (!m_captionData.m_acceptDialogShown && size == m_captionData.m_acceptDialogSize + m_captionData.m_rejectDialogSize + 1)
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
                        // Accept mission, remove reject dialogs
                        for (int i = 0; i < m_captionData.m_rejectDialogSize; i++)
                        {
                            m_captionData.m_captions.pop_back();
                        }

                        // Accept callback
                        for (DialogCallbackFunc pFn : m_acceptCallbacks)
                        {
                            pFn(MissionManager::m_genericNPCObject, MissionManager::m_genericNPCDialog);
                        }
                    }
                    else
                    {
                        // Reject mission, remove accept dialogs
                        for (int i = 0; i < m_captionData.m_acceptDialogSize; i++)
                        {
                            m_captionData.m_captions.pop_front();
                        }
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
        if (m_captionData.m_acceptDialogShown && dt > 0.0f)
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

            // Finish callback
            for (DialogCallbackFunc pFn : m_finishCallbacks)
            {
                pFn(MissionManager::m_genericNPCObject, MissionManager::m_genericNPCDialog);
            }
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

float SubtitleUI::drawSubtitle(Subtitle const& subtitle, float alpha)
{
    float maxWidth = 0.0f;
    float currentWidth = 0.0f;

    float const offset = 10.0f;
    ImGui::SetCursorPos(ImVec2(offset, offset));

    for (uint32_t i = 0; i < subtitle.m_subtitles.size(); i++)
    {
        std::string const& str = subtitle.m_subtitles[i];
        float const color = 1.0f;

        ImGui::TextColored(ImVec4(color, color, color, alpha * 0.9f), str.c_str());
        currentWidth += ImGui::CalcTextSize(str.c_str()).x;

        if (subtitle.m_buttons.count(i))
        {
            float buttonSizeX = 28.0f;
            switch (subtitle.m_buttons.at(i))
            {
            case SBT_LB:
            case SBT_RB:
            case SBT_LT:
            case SBT_RT:
                buttonSizeX = 56.0f;
                break;
            }

            IUnknown** texture = nullptr;
            switch (subtitle.m_buttons.at(i))
            {
            case SBT_A:     texture = &m_captionData.m_buttonA;      break;
            case SBT_B:     texture = &m_captionData.m_buttonB;      break;
            case SBT_X:     texture = &m_captionData.m_buttonX;      break;
            case SBT_Y:     texture = &m_captionData.m_buttonY;      break;
            case SBT_LB:    texture = &m_captionData.m_buttonLB;     break;
            case SBT_RB:    texture = &m_captionData.m_buttonRB;     break;
            case SBT_LT:    texture = &m_captionData.m_buttonLT;     break;
            case SBT_RT:    texture = &m_captionData.m_buttonRT;     break;
            case SBT_Start: texture = &m_captionData.m_buttonStart;  break;
            case SBT_Back:  texture = &m_captionData.m_buttonBack;   break;
            case SBT_LStick:  texture = &m_captionData.m_buttonLStick;   break;
            case SBT_RStick:  texture = &m_captionData.m_buttonRStick;   break;
            case SBT_DPad:  texture = &m_captionData.m_buttonDPad;   break;
            }

            ImGui::SameLine();
            if (texture)
            {
                ImGui::Image(*texture, ImVec2(*BACKBUFFER_WIDTH * buttonSizeX / 1280.0f, *BACKBUFFER_HEIGHT * 28.0f / 720.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, alpha));
                currentWidth += buttonSizeX;
            }
        }

        if (!str.empty() && str.back() == '\n')
        {
            // y-spacing is slightly larger for gameplay dialog
            ImGui::SetCursorPosX(offset);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + *BACKBUFFER_HEIGHT * 0.01f);

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

void SubtitleUI::addDialogAcceptCallback(DialogCallbackFunc pFn)
{
    m_acceptCallbacks.insert(pFn);
}

void SubtitleUI::addDialogFinishCallback(DialogCallbackFunc pFn)
{
    m_finishCallbacks.insert(pFn);
}
