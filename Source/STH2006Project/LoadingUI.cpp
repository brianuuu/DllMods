#include "LoadingUI.h"
#include "UIContext.h"
#include "Application.h"

bool LoadingUI::m_init = false;
bool LoadingUI::m_drawEnabled = false;
float LoadingUI::m_fadeInTime = 0.0f;
int LoadingUI::m_frame = 0;
bool LoadingUI::m_showNowLoading = false;
PDIRECT3DTEXTURE9 LoadingUI::m_backgroundTexture = nullptr;
PDIRECT3DTEXTURE9 LoadingUI::m_stageTexture = nullptr;
PDIRECT3DTEXTURE9 LoadingUI::m_nowLoadingTexture = nullptr;
PDIRECT3DTEXTURE9 LoadingUI::m_arrowTexture = nullptr;
uint32_t LoadingUI::m_stagePrevious;
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
	uint32_t currentStage = Common::GetCurrentStageID();
	switch (currentStage)
	{
	case SMT_ghz200:
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\wvo.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"エッグキャリアを追跡せよ！" : "Pursue  the  Egg  Carrier!";
		break;
	}
	case SMT_cpz200:
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\dtd.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"Ｄｒ．エッグマンの追っ手から逃げ切れ！" : "Escape  Dr.  Eggman's  Minions!";
		break;
	}
	case SMT_ssz200:
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\wap.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"Ｄｒ．エッグマンの基地に侵入せよ！" : "Infiltrate  Dr.  Eggman's  Base!";
		break;
	}
	case SMT_sph200:
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\csc.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"データベースを探し出せ！" : "Find  the  Database!";
		break;
	}
	case SMT_cte200:
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\flc.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"カオスエメラルドを探し出せ！" : "Find  the  Chaos  Emerald!";
		break;
	}
	case SMT_ssh200:
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\rct.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"Ｄｒ．エッグマンの列車を追え！！" : "Follow  Dr.  Eggman's  Train!";
		break;
	}
	case SMT_csc200:
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\tpj.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"Ｄｒ．エッグマンの追跡をかわせ！" : "Avoid  Dr.  Eggman's  Pursuit!";
		break;
	}
	case SMT_euc200:
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\kdv.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"エッグキャリアを追え！" : "Follow  the  Egg  Carrier!";
		break;
	}
	case SMT_pla200:
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\aqa.dds").c_str(), &LoadingUI::m_stageTexture);
		LoadingUI::m_bottomText = isJapanese ? u8"エッグキャリアの発進を阻止せよ！" : "Stop  the  Egg  Carrier's  Launch!";
		break;
	}
	case SMT_pam000:
	{
		if (LoadingUI::m_stagePrevious == SMT_ghz200 ||
			LoadingUI::m_stagePrevious == SMT_cpz200 ||
			LoadingUI::m_stagePrevious == SMT_pla200 ||
			LoadingUI::m_stagePrevious == SMT_bsl || 
			LoadingUI::m_stagePrevious == (SMT_bsl | SMT_BossHard))
		{
			UIContext::loadTextureFromFile((dir + L"Assets\\Title\\twn_a.dds").c_str(), &LoadingUI::m_stageTexture);
			// TODO: text
		}
		else if (LoadingUI::m_stagePrevious == SMT_ssz200 ||
			LoadingUI::m_stagePrevious == SMT_sph200 ||
			LoadingUI::m_stagePrevious == SMT_ssh200 ||
			LoadingUI::m_stagePrevious == SMT_bpc ||
			LoadingUI::m_stagePrevious == (SMT_bpc | SMT_BossHard))
		{
			UIContext::loadTextureFromFile((dir + L"Assets\\Title\\twn_b.dds").c_str(), &LoadingUI::m_stageTexture);
			// TODO: text
		}
		else
		{
			UIContext::loadTextureFromFile((dir + L"Assets\\Title\\twn_c.dds").c_str(), &LoadingUI::m_stageTexture);
			// TODO: text
		}
		break;
	}
	default:
	{
		UIContext::loadTextureFromFile((dir + L"Assets\\Title\\cmn.dds").c_str(), &LoadingUI::m_stageTexture);
		if (currentStage == SMT_ghz100)
		{
			LoadingUI::m_bottomText = isJapanese ? u8"ソレアナへ向かえ！" : "Get  to  Soleanna!";
		}
		else if (currentStage == SMT_bsl || currentStage == (SMT_bsl | SMT_BossHard))
		{
			LoadingUI::m_bottomText = isJapanese ? u8"シルバーを倒せ！" : "Defeat  Silver!";
		}
		else if (currentStage == SMT_bpc || currentStage == (SMT_bpc | SMT_BossHard))
		{
			LoadingUI::m_bottomText = isJapanese ? u8"イブリースを倒せ！" : "Defeat  Iblis!";
		}
		// TODO: text
		break;
	}
	}

	if (LoadingUI::m_stageTexture)
	{
		LoadingUI::startNowLoading();
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

	LoadingUI::stopNowLoading();
}

HOOK(int, __fastcall, LoadingUI_CStateGoalFadeIn, 0xCFD2D0, uint32_t* This)
{
	// Stop loading in case it is activated before result screen
	LoadingUI::stopNowLoading();
	return originalLoadingUI_CStateGoalFadeIn(This);
}

void LoadingUI::applyPatches()
{
	if (!m_init) return;

	// Don't use original stage loading
	WRITE_MEMORY(0x1092DFE, uint8_t, 0xEB);

	INSTALL_HOOK(LoadingUI_MsgRequestStartLoading);
	INSTALL_HOOK(LoadingUI_MsgRequestCloseLoading);
	INSTALL_HOOK(LoadingUI_CStateGoalFadeIn);

	// Disable loading sfx
	WRITE_MEMORY(0x44A2E8, int, -1);
	WRITE_MEMORY(0x44A4F5, int, -1);
}

float LoadingUI::m_startCountdown = -1.0f;
void LoadingUI::startNowLoading(float countdown)
{
	if (!m_drawEnabled)
	{
		if (countdown > 0.0f)
		{
			m_startCountdown = countdown;
		}
		else
		{
			m_startCountdown = -1.0f;
			m_drawEnabled = true;
			m_fadeInTime = 0.0f;
			m_frame = 0;
			m_showNowLoading = false;
		}
	}
}

void LoadingUI::stopNowLoading()
{
	if (m_drawEnabled)
	{
		LoadingUI::m_fadeInTime = 0.4f;
	}
}

void LoadingUI::draw()
{
	// Loading has been requested with countdown
	if (m_startCountdown > 0.0f)
	{
		m_startCountdown -= Application::getDeltaTime();
		if (m_startCountdown <= 0.0f)
		{
			startNowLoading();
		}
	}

	if (!m_drawEnabled) return;

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
			m_fadeInTime = max(0.0f, m_fadeInTime - Application::getDeltaTime());
			if (m_fadeInTime <= 0.2f)
			{
				alpha = m_fadeInTime * 5.0f;
			}

			if (m_fadeInTime == 0.0f)
			{
				m_startCountdown = -1.0f;
				m_drawEnabled = false;
				if (LoadingUI::m_stageTexture)
				{
					LoadingUI::m_stageTexture->Release();
					LoadingUI::m_stageTexture = nullptr;
				}
			}
		}

		// Draw black background
		ImGui::SetCursorPos(ImVec2(5, 5));
		ImGui::Image(m_backgroundTexture, ImVec2((float)*BACKBUFFER_WIDTH, (float)*BACKBUFFER_HEIGHT), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, alpha));

		if (m_drawEnabled && (m_fadeInTime == 0.0f || m_fadeInTime >= 0.2f))
		{
			if (m_stageTexture)
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

			// Show text after 24 frames
			if (!m_showNowLoading && m_frame >= 24)
			{
				m_showNowLoading = true;
			}

			// Now Loading Text
			if (m_showNowLoading)
			{
				ImGui::SetCursorPos(ImVec2(914 * scaleX, 612 * scaleY));
				ImGui::Image(m_nowLoadingTexture, ImVec2(256 * scaleX, 40 * scaleY));

				// Pop out
				if (m_frame >= 24 && m_frame <= 76)
				{
					float nowLoadingAlpha = (float)(76 - m_frame) / 52.0f;
					float nowLoadingscale = 1.0f + (1.0f - nowLoadingAlpha) * 0.3f;
					ImGui::SetCursorPos(ImVec2((1042 - 128 * nowLoadingscale) * scaleX, (632 - 20 * nowLoadingscale) * scaleY));
					ImGui::Image(m_nowLoadingTexture, ImVec2(256 * nowLoadingscale * scaleX, 40 * nowLoadingscale * scaleY), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, nowLoadingAlpha));
				}
			}

			// Now Loading Arrpws
			ImGui::SetCursorPos(ImVec2((-113 + 39 * m_frame)* scaleX, 612 * scaleY));
			ImGui::Image(m_arrowTexture, ImVec2(110 * scaleX, 40 * scaleY));
		}
	}
	ImGui::End();

	m_frame++;
	if (m_frame >= 100)
	{
		m_frame = 0;
	}
}

bool LoadingUI::initTextures()
{
	std::wstring const dir = Application::getModDirWString();
	m_init = true;
	
	m_init &= UIContext::loadTextureFromFile((dir + L"Assets\\Title\\rect.dds").c_str(), &m_backgroundTexture);
	m_init &= UIContext::loadTextureFromFile((dir + L"Assets\\Title\\NowLoading.dds").c_str(), &m_nowLoadingTexture);
	m_init &= UIContext::loadTextureFromFile((dir + L"Assets\\Title\\NowLoadingArrow.dds").c_str(), &m_arrowTexture);

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
