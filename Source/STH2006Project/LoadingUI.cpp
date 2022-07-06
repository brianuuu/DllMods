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
PDIRECT3DTEXTURE9 LoadingUI::m_tipsTexture = nullptr;
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

	if (LoadingUI::m_tipsTexture)
	{
		LoadingUI::m_tipsTexture->Release();
		LoadingUI::m_tipsTexture = nullptr;
	}

	bool isJapanese = Common::GetUILanguageType() == LT_Japanese;
	LoadingUI::m_bottomText.clear();

	std::wstring const dir = Application::getModDirWString();
	uint32_t currentStage = Common::GetCurrentStageID();

	// Deal with special case first
	switch (currentStage)
	{
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
	}

	if (LoadingUI::m_stageTexture == nullptr)
	{
		// Get loading UI data from ini
		const INIReader reader(Application::getModDirString() + "Assets\\Title\\titleData.ini");
		if (reader.ParseError() != 0)
		{
			UIContext::loadTextureFromFile((dir + L"Assets\\Title\\cmn.dds").c_str(), &LoadingUI::m_stageTexture);
			LoadingUI::m_bottomText = "MISSING TEXT";
		}
		else
		{
			std::string currentStageStr = std::to_string(currentStage);

			// Title card
			std::string title = reader.Get(currentStageStr, "Title", "cmn");
			std::wstring titleWide = dir + L"Assets\\Title\\" + Common::multiByteToWideChar(title.c_str()) + L".dds";
			UIContext::loadTextureFromFile(titleWide.c_str(), &LoadingUI::m_stageTexture);

			// Bottom text (add extra space)
			LoadingUI::m_bottomText = reader.Get(currentStageStr, isJapanese ? "TextJP" : "Text", "MISSING TEXT");
			LoadingUI::m_bottomText = std::regex_replace(LoadingUI::m_bottomText, std::regex(" "), "  ");
			LoadingUI::m_bottomText = std::regex_replace(LoadingUI::m_bottomText, std::regex("\\\\n"), "\n");

			// Character tips (For S06DE character movesets only)
			if (currentStage >= SMT_ghz200 && currentStage <= SMT_pla200)
			{
				if (S06DE_API::IsUsingCharacterMoveset())
				{
					std::srand(static_cast<unsigned int>(std::time(nullptr)));
					switch (S06DE_API::GetModelType())
					{
						case S06DE_API::ModelType::Sonic:
						{
							UIContext::loadTextureFromFile((dir + L"Assets\\Title\\tips_sonic" + std::to_wstring(std::rand() % 3) + L".dds").c_str(), &LoadingUI::m_tipsTexture);
							break;
						}
						case S06DE_API::ModelType::SonicElise:
						{
							UIContext::loadTextureFromFile((dir + L"Assets\\Title\\tips_princess.dds").c_str(), &LoadingUI::m_tipsTexture);
							break;
						}
						case S06DE_API::ModelType::Blaze:
						{
							UIContext::loadTextureFromFile((dir + L"Assets\\Title\\tips_blaze.dds").c_str(), &LoadingUI::m_tipsTexture);
							break;
						}
					}
				}
			}
		}
	}

	LoadingUI::startNowLoading();
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

HOOK(int, __fastcall, LoadingUI_CLoadingSimple, 0x44A480, uint32_t* This, void* Edx, float* a2)
{
	int result = originalLoadingUI_CLoadingSimple(This, Edx, a2);
	if (!Common::IsAtLoadingScreen() || result == 2)
	{
		LoadingUI::stopNowLoading();
	}
	else if (result == 0)
	{
		LoadingUI::startNowLoading();
	}
	return result;
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

	// Trigger 06 loading when restarting stage/retry mission
	INSTALL_HOOK(LoadingUI_CLoadingSimple);
}

bool m_ignoreStopNowLoadingRequest = false;
float LoadingUI::m_startCountdown = -1.0f;
void LoadingUI::startNowLoading(float countdown, bool ignoreOthers)
{
	if (!m_drawEnabled)
	{
		m_ignoreStopNowLoadingRequest = ignoreOthers;
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

void LoadingUI::stopNowLoading(float fadeInTime, bool forceStop)
{
	if (!m_ignoreStopNowLoadingRequest || (m_ignoreStopNowLoadingRequest && forceStop))
	{
		if (m_drawEnabled && LoadingUI::m_fadeInTime == 0.0f)
		{
			LoadingUI::m_fadeInTime = (fadeInTime <= 0.0f) ? 0.001f : fadeInTime;
		}
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

				if (m_stageTexture)
				{
					m_stageTexture->Release();
					m_stageTexture = nullptr;
				}

				if (m_tipsTexture)
				{
					m_tipsTexture->Release();
					m_tipsTexture = nullptr;
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

				// Character movesets
				if (m_tipsTexture)
				{
					ImGui::SetCursorPos(ImVec2(128 * scaleX, 109 * scaleY));
					ImGui::Image(m_tipsTexture, ImVec2(1024 * scaleX, 512 * scaleY));
				}

				// Bottom text
				if (!m_bottomText.empty())
				{
					ImVec2 size = ImGui::CalcTextSize(m_bottomText.c_str());
					ImGui::SetCursorPos(ImVec2(640 * scaleX - size.x * 0.5f, 566 * scaleY));
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
				ImGui::SetCursorPos(ImVec2(914 * scaleX, 617 * scaleY));
				ImGui::Image(m_nowLoadingTexture, ImVec2(256 * scaleX, 40 * scaleY));

				// Pop out
				if (m_frame >= 24 && m_frame <= 76)
				{
					float nowLoadingAlpha = (float)(76 - m_frame) / 52.0f;
					float nowLoadingscale = 1.0f + (1.0f - nowLoadingAlpha) * 0.3f;
					ImGui::SetCursorPos(ImVec2((1042 - 128 * nowLoadingscale) * scaleX, (637 - 20 * nowLoadingscale) * scaleY));
					ImGui::Image(m_nowLoadingTexture, ImVec2(256 * nowLoadingscale * scaleX, 40 * nowLoadingscale * scaleY), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, nowLoadingAlpha));
				}
			}

			// Now Loading Arrpws
			ImGui::SetCursorPos(ImVec2((-113 + 39 * m_frame)* scaleX, 617 * scaleY));
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
