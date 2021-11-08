#include "LoadingUI.h"
#include "UIContext.h"
#include "Application.h"

bool LoadingUI::m_init = false;
bool LoadingUI::m_drawEnabled = false;
float LoadingUI::m_fadeInTime = 0.0f;
PDIRECT3DTEXTURE9 LoadingUI::m_backgroundTexture = nullptr;
PDIRECT3DTEXTURE9 LoadingUI::m_stageTexture = nullptr;
std::string LoadingUI::m_stagePrevious;
std::string LoadingUI::m_bottomText;

HOOK(void, __fastcall, LoadingUI_MsgRequestStartLoading, 0x1092D80, uint32_t* This, void* Edx, void* message)
{
	if (LoadingUI::m_drawEnabled) return;

	if (LoadingUI::m_stageTexture)
	{
		LoadingUI::m_stageTexture->Release();
		LoadingUI::m_stageTexture = nullptr;
	}

	bool isJapanese = *(uint8_t*)Common::GetMultiLevelAddress(0x1E66B34, { 0x8 }) == 1;
	LoadingUI::m_bottomText.clear();

	std::wstring const dir = Application::getModDirWString();
	std::string currentStage = std::string((char*)0x01E774D4);
	if (currentStage == "ghz200")
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\wvo.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"エッグキャリアを追跡せよ！" : "Pursue  the  Egg  Carrier!";
	}
	else if (currentStage == "cpz200")
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\dtd.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"Ｄｒ．エッグマンの追っ手から逃げ切れ！" : "Escape  Dr.  Eggman's  Minions!";
	}
	else if (currentStage == "ssz200")
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\wap.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"Ｄｒ．エッグマンの基地に侵入せよ！" : "Infiltrate  Dr.  Eggman's  Base!";
	}
	else if (currentStage == "sph200")
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\csc.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"データベースを探し出せ！" : "Find  the  Database!";
	}
	else if (currentStage == "cte200")
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\flc.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"カオスエメラルドを探し出せ！" : "Find  the  Chaos  Emerald!";
	}
	else if (currentStage == "ssh200")
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\rct.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"Ｄｒ．エッグマンの列車を追え！！" : "Follow  Dr.  Eggman's  Train!";
	}
	else if (currentStage == "csc200")
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\tpj.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"Ｄｒ．エッグマンの追跡をかわせ！" : "Avoid  Dr.  Eggman's  Pursuit!";
	}
	else if (currentStage == "euc200")
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\kdv.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"エッグキャリアを追え！" : "Follow  the  Egg  Carrier!";
	}
	else if (currentStage == "pla200")
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\aqa.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"エッグキャリアの発進を阻止せよ！" : "Stop  the  Egg  Carrier's  Launch!";
	}
	else if (currentStage == "pam000")
	{
		if (LoadingUI::m_stagePrevious == "ghz200" ||
			LoadingUI::m_stagePrevious == "cpz200" ||
			LoadingUI::m_stagePrevious == "pla200" ||
			LoadingUI::m_stagePrevious == "bsl" ||
			LoadingUI::m_stagePrevious == "bsl001")
		{
			UIContext::loadTextureFromFile((dir + L"Assets\\Title\\twn_a.dds").c_str(), &LoadingUI::m_stageTexture);
			// TODO: text
		}
		else if (LoadingUI::m_stagePrevious == "ssz200" ||
				 LoadingUI::m_stagePrevious == "sph200" ||
				 LoadingUI::m_stagePrevious == "ssh200" ||
				 LoadingUI::m_stagePrevious == "bpc" ||
				 LoadingUI::m_stagePrevious == "bpc001")
		{
			UIContext::loadTextureFromFile((dir + L"Assets\\Title\\twn_b.dds").c_str(), &LoadingUI::m_stageTexture);
			// TODO: text
		}
		else
		{
			UIContext::loadTextureFromFile((dir + L"Assets\\Title\\twn_c.dds").c_str(), &LoadingUI::m_stageTexture);
			// TODO: text
		}
	}
	else
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\cmn.dds").c_str(), &LoadingUI::m_stageTexture);
		if (currentStage == "ghz100")
		{
			LoadingUI::m_bottomText = isJapanese ? u8"ソレアナに向かえ！" : "Get  to  Soleanna!";
		}
		else if (currentStage == "bsl" || currentStage == "bsl001")
		{
			LoadingUI::m_bottomText = isJapanese ? u8"シルバーを倒せ！" : "Defeat  Silver!";
		}
		else if (currentStage == "bpc" || currentStage == "bpc001")
		{
			LoadingUI::m_bottomText = isJapanese ? u8"イブリースを倒せ！" : "Defeat  Iblis!";
		}
		// TODO: text
	}

	if (LoadingUI::m_stageTexture)
	{
		LoadingUI::m_drawEnabled = true;
		LoadingUI::m_fadeInTime = 0.0f;
	}
	LoadingUI::m_stagePrevious = currentStage;
}

HOOK(void, __fastcall, LoadingUI_MsgRequestCloseLoading, 0x1092BF0, uint32_t* This, void* Edx, void* message)
{
	// Release control immediately
	uint32_t** hudCount = (uint32_t**)0x1E66B40;
	if (*hudCount)
	{
		(*hudCount)[2] = 0;
	}

	LoadingUI::m_fadeInTime = 0.3f;
}

void LoadingUI::applyPatches()
{
	if (!m_init) return;

	// Don't use original stage loading
	WRITE_MEMORY(0x1092DFE, uint8_t, 0xEB);

	INSTALL_HOOK(LoadingUI_MsgRequestStartLoading);
	INSTALL_HOOK(LoadingUI_MsgRequestCloseLoading);

	// Disable loading sfx
	WRITE_MEMORY(0x44A2E8, int, -1);
	WRITE_MEMORY(0x44A4F5, int, -1);
}

void LoadingUI::draw()
{
	if (!m_drawEnabled || !m_stageTexture) return;

	float scaleX = *BACKBUFFER_WIDTH / 1280.0f;
	float scaleY = *BACKBUFFER_HEIGHT / 720.0f;

	static bool visible = true;
	ImGui::Begin((std::string("StageTitle")).c_str(), &visible, UIContext::m_hudFlags);
	{
		ImGui::SetWindowFocus();
		ImGui::SetWindowPos(ImVec2(-5, -5));

		float alpha = 1.0f;
		if (m_fadeInTime > 0.0f)
		{
			m_fadeInTime = max(0.0f, m_fadeInTime - Application::getHudDeltaTime());
			if (m_fadeInTime <= 0.2f)
			{
				alpha = m_fadeInTime * 2.5f;
			}

			if (m_fadeInTime == 0.0f)
			{
				m_drawEnabled = false;
			}
		}

		// Draw black background
		ImGui::SetCursorPos(ImVec2(5, 5));
		ImGui::Image(m_backgroundTexture, ImVec2((float)*BACKBUFFER_WIDTH, (float)*BACKBUFFER_HEIGHT), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, alpha));

		if (m_drawEnabled && (m_fadeInTime == 0.0f || m_fadeInTime >= 0.2f))
		{
			// Stage title
			ImGui::SetCursorPos(ImVec2(5, 109 * scaleY));
			ImGui::Image(m_stageTexture, ImVec2(1280 * scaleX, 512 * scaleY));

			// Bottom text
			if (!m_bottomText.empty())
			{
				ImVec2 size = ImGui::CalcTextSize(m_bottomText.c_str());
				ImGui::SetCursorPos(ImVec2(640 * scaleX - size.x * 0.5f, 570 * scaleY));
				ImGui::Text(m_bottomText.c_str());
			}
		}
	}
	ImGui::End();
}

bool LoadingUI::initTextures()
{
	std::wstring const dir = Application::getModDirWString();
	m_init = UIContext::loadTextureFromFile((dir + L"Assets\\Title\\rect.dds").c_str(), &m_backgroundTexture);

	if (!m_init)
	{
		MessageBox(nullptr, TEXT("Failed to load assets stage titles, reverting to Generations"), TEXT("STH2006 Project"), MB_ICONWARNING);
	}
	else
	{
		applyPatches();
	}

	return m_init;
}
