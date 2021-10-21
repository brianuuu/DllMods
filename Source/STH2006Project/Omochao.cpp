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

    // Read caption and convert to string
    uint32_t const length = (caption[2] - caption[1]) / 4;
    uint32_t* captionList = (uint32_t*)caption[1];
    wchar_t str[1024];
    for (uint32_t i = 0; i < length; i++)
    {
        uint32_t const key = captionList[i];
        if (m_fontDatabase.count(key))
        {
            str[i] = m_fontDatabase[key];
        }
        else if (key == 0)
        {
            str[i] = L'\n';
        }
        else
        {
            str[i] = L'?';
        }
    }
    str[length] = L'\0';

	m_captionData.m_captions.push_back(Caption({ Common::wideCharToMultiByte(str), duration }));
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
            if (frame1 < 5)
            {
                posY += 0.03476f * (5 - frame1);
                alpha = 0.2f * (5 - frame1);
            }
            else if (frame2 < 5)
            {
                posY += 0.03476f * (5 - frame2);
                alpha = 0.2f * (5 - frame2);
            }

            ImGui::SetWindowFocus();
            ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * posX, *BACKBUFFER_HEIGHT * posY));
            ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
            ImGui::Image(m_captionData.m_textbox, ImVec2(sizeX, sizeY));
        }
        ImGui::End();

        ImGui::Begin("Caption", &visible, UIContext::m_hudFlags);
        {
            ImGui::SetWindowFocus();
            ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * 0.2023f, *BACKBUFFER_HEIGHT * (posY + 0.0438f)));
            ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
            ImGui::Text(caption.m_caption.c_str());
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
