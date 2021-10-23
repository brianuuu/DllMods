#include "Omochao.h"
#include "Application.h"
#include "UIContext.h"

CaptionData Omochao::m_captionData;

HOOK(int, __fastcall, Omochao_MsgNotifyObjectEvent, 0x114FB60, void* This, void* Edx, uint32_t a2)
{
	uint32_t* pEvent = (uint32_t*)(a2 + 16);
	switch (*pEvent)
	{
	case 51:
	{
		// Elise specific dialogs
		if (!Common::IsPlayerSuper() && Common::CheckPlayerNodeExist("ch_princess01_elise"))
		{
			*pEvent = 6;
		}
		break;
	}
	case 52:
	{
		// Non-Elise specific dialogs
		if (Common::IsPlayerSuper() || !Common::CheckPlayerNodeExist("ch_princess01_elise"))
		{
			*pEvent = 6;
		}
		break;
	}
	default: break;
	}

	return originalOmochao_MsgNotifyObjectEvent(This, Edx, a2);
}

uint32_t sub_6B02B0 = 0x6B02B0;
uint32_t addCaptionReturnAddress = 0x461407;
void __declspec(naked) addCaption()
{
    __asm
    {
        call    [sub_6B02B0]

        push    eax
        push    edx

        // duration
        mov     ecx, [esp + 0x1C + 0x8]
        push    ecx

        // caption
        mov     ecx, [eax]
        push    ecx

        // this (COmochao)
        mov     ecx, [esi + 8]
        push    ecx

        call    Omochao::addCaptionImpl
        add     esp, 0xC

        pop     edx
        pop     eax

        jmp     [addCaptionReturnAddress]
    }
}

void Omochao::applyPatches()
{
	INSTALL_HOOK(Omochao_MsgNotifyObjectEvent);

    // Caption subtitle
    if (initFontDatabase())
    {
        WRITE_JUMP(0x461402, addCaption);
        WRITE_JUMP(0x11F8813, (void*)0x11F8979);
    }
}

std::map<uint32_t, wchar_t> Omochao::m_fontDatabase;
bool Omochao::initFontDatabase()
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

#if _DEBUG
        std::wstring missingStr = L"…俺窟秤";
        std::sort(missingStr.begin(), missingStr.end());
        int i = 0;
        for (wchar_t const& c : missingStr)
        {
            printf("0x%04X, ", c);
            i++;
            if (i % 10 == 0)
            {
                printf("\n");
            }
        }
#endif

        return true;
    }
    
    MessageBox(nullptr, TEXT("Failed to load font database, reverting to in-game textbox."), TEXT("STH2006 Project"), MB_ICONWARNING);
    return false;
}

void __cdecl Omochao::addCaptionImpl(uint32_t* owner, uint32_t* caption, float duration)
{
	// Caption disabled
	if ((*(uint8_t*)Common::GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0x18 }) & 0x10) == 0)
	{
		return;
	}

	if (m_captionData.m_owner != owner)
	{
		m_captionData.clear();
	}
	m_captionData.m_owner = owner;

    Caption newCaption;
    newCaption.m_duration = duration;

    // Read caption and convert to string
    uint32_t const length = (caption[2] - caption[1]) / 4;
    uint32_t* captionList = (uint32_t*)caption[1];
    std::wstring str;
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
        }
        else if (m_fontDatabase.count(key))
        {
            if (key == 0x82)
            {
                str += L"  ";
            }
            else
            {
                str += m_fontDatabase[key];
            }
        }
        else if (key == 0)
        {
            str += L'\n';
            newCaption.m_captions.push_back(Common::wideCharToMultiByte(str.c_str()));
            str.clear();
        }
        else
        {
            str += L'?';
        }
    }

    // Push remaining text
    if (!str.empty())
    {
        newCaption.m_captions.push_back(Common::wideCharToMultiByte(str.c_str()));
    }

	m_captionData.m_captions.push_back(newCaption);
    m_captionData.m_bypassLoading = (*(uint32_t**)0x1E66B40)[2] > 0;
}

void CaptionData::init()
{
    std::wstring const dir = Application::getModDirWString();
    bool success = true;
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Textbox.dds").c_str(), &m_textbox);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Button_A.dds").c_str(), &m_buttonA);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Button_B.dds").c_str(), &m_buttonB);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Button_X.dds").c_str(), &m_buttonX);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Button_Y.dds").c_str(), &m_buttonY);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Button_LB.dds").c_str(), &m_buttonLB);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Button_LT.dds").c_str(), &m_buttonLT);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Button_RB.dds").c_str(), &m_buttonRB);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Button_RT.dds").c_str(), &m_buttonRT);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Button_Start.dds").c_str(), &m_buttonStart);
    success &= UIContext::loadTextureFromFile((dir + L"Assets\\Button_Back.dds").c_str(), &m_buttonBack);

    if (!success)
    {
        m_captions.clear();
        WRITE_MEMORY(0x461402, uint8_t, 0xE8, 0xA9, 0xEE, 0x24, 0x00);
        WRITE_MEMORY(0x11F8813, uint8_t, 0x0F, 0x84, 0x60, 0x01, 0x00, 0x00);
        MessageBox(nullptr, TEXT("Failed to load assets for custom 06 textbox, reverting to in-game textbox."), TEXT("STH2006 Project"), MB_ICONWARNING);
    }
}

void Omochao::draw()
{
    if (m_captionData.m_bypassLoading)
    {
        // No longer at loading screen
        if ((*(uint32_t**)0x1E66B40)[2] == 0)
        {
            m_captionData.m_bypassLoading = false;
        }
    }
    else if((*(uint32_t**)0x1E66B40)[2] > 0)
    {
        // At loading screen, clear all
        m_captionData.m_captions.clear();
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

            alpha *= 0.9f;
            ImGui::SetWindowFocus();
            ImGui::SetWindowSize(ImVec2(sizeX, sizeY)); 
            ImGui::Image(m_captionData.m_textbox, ImVec2(sizeX, sizeY), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, alpha));
            ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * posX, *BACKBUFFER_HEIGHT * posY));
        }
        ImGui::End();

        ImGui::Begin("Caption", &visible, UIContext::m_hudFlags);
        {
            ImGui::SetWindowFocus();
            ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
            for (uint32_t i = 0; i < caption.m_captions.size(); i++)
            {
                std::string const& str = caption.m_captions[i];
                ImGui::TextColored(ImVec4(1, 1, 1, alpha), str.c_str());
                if (caption.m_buttons.count(i))
                {
                    float buttonSizeX = 28.0f;
                    switch (caption.m_buttons[i])
                    {
                    case CBT_LB:
                    case CBT_RB:
                    case CBT_LT:
                    case CBT_RT:
                        buttonSizeX = 56.0f;
                        break;
                    }

                    PDIRECT3DTEXTURE9 texture = nullptr;
                    switch (caption.m_buttons[i])
                    {
                    case CBT_A:     texture = m_captionData.m_buttonA;      break;
                    case CBT_B:     texture = m_captionData.m_buttonB;      break;
                    case CBT_Y:     texture = m_captionData.m_buttonY;      break;
                    case CBT_X:     texture = m_captionData.m_buttonX;      break;
                    case CBT_LB:    texture = m_captionData.m_buttonLB;     break;
                    case CBT_RB:    texture = m_captionData.m_buttonRB;     break;
                    case CBT_LT:    texture = m_captionData.m_buttonLT;     break;
                    case CBT_RT:    texture = m_captionData.m_buttonRT;     break;
                    case CBT_Start: texture = m_captionData.m_buttonStart;  break;
                    case CBT_Back:  texture = m_captionData.m_buttonBack;   break;
                    }

                    ImGui::SameLine();
                    if (texture)
                    {
                        ImGui::Image(texture, ImVec2(*BACKBUFFER_WIDTH * buttonSizeX / 1280.0f, *BACKBUFFER_HEIGHT * 28.0f / 720.0f));
                    }
                }

                if (str.back() == '\n')
                {
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + *BACKBUFFER_HEIGHT * 0.01f);
                }
                else
                {
                    ImGui::SameLine();
                }
            }
            ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * 0.2023f, *BACKBUFFER_HEIGHT* (posY + 0.047f)));
        }
        ImGui::End();

        m_captionData.m_timer += Application::getHudDeltaTime();
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
