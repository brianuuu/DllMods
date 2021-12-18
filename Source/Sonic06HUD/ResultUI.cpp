#include "ResultUI.h"
#include "UIContext.h"
#include "Application.h"

bool ResultUI::m_init = false;
PDIRECT3DTEXTURE9 ResultUI::m_resultRankTextures[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
PDIRECT3DTEXTURE9 ResultUI::m_resultNumTextures[10] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
PDIRECT3DTEXTURE9 ResultUI::m_resultTextTextures[RTT_COUNT] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
PDIRECT3DTEXTURE9 ResultUI::m_resultCommaTexture = nullptr;
PDIRECT3DTEXTURE9 ResultUI::m_resultBoxTexture = nullptr;
PDIRECT3DTEXTURE9 ResultUI::m_resultTotalBoxTexture = nullptr;
PDIRECT3DTEXTURE9 ResultUI::m_resultScoreBoxTexture = nullptr;
PDIRECT3DTEXTURE9 ResultUI::m_resultHeaderBoxTexture = nullptr;
PDIRECT3DTEXTURE9 ResultUI::m_resultHeaderShadowTexture = nullptr;
PDIRECT3DTEXTURE9 ResultUI::m_resultHeaderTextTexture = nullptr;
PDIRECT3DTEXTURE9 ResultUI::m_resultRankBoxTexture = nullptr;
PDIRECT3DTEXTURE9 ResultUI::m_resultFadeTexture = nullptr;

ResultUI::CScoreManager* ResultUI::m_pCScoreManager = nullptr;
ResultUI::ScoreSystemType ResultUI::m_scoreSystem = ResultUI::ScoreSystemType::SST_None;

float ResultUI::m_currentTime = 0.0f;
ResultUI::ResultData* ResultUI::m_resultData = nullptr;

HOOK(void, __fastcall, ResultUI_MsgStartGoalResult, 0x10B58A0, uint32_t* This, void* Edx, void* message)
{
	ResultUI::m_resultUIData.start();
	// Don't run the message itself
}

HOOK(bool, __stdcall, ResultUI_CEventScene, 0xB238C0, void* a1)
{
	// Reset when cutscene starts
	ResultUI::m_resultUIData.reset();
	return originalResultUI_CEventScene(a1);
}

HOOK(int, __fastcall, ResultUI_CStateGoalFadeBefore, 0xCFE080, uint32_t* This)
{
	int result = originalResultUI_CStateGoalFadeBefore(This);
	{
		ResultUI::m_currentTime = *(float*)Common::GetMultiLevelAddress(This[2] + 0x60, { 0x8, 0x184 });
		ResultUI::m_resultData = (ResultUI::ResultData*)(This[2] + 0x16C);

		// Fix result camera
		WRITE_MEMORY(0x1A48C7C, float, 0.704f); // OffsetRight
		WRITE_MEMORY(0x1A48C80, float, 0.455f); // OffsetUp
	}
	return result;
}

bool resultFinished = false;
HOOK(void, __fastcall, ResultUI_CHudResultAdvance, 0x10B96D0, uint32_t* This, void* Edx, void* a2)
{
	// Fake pressing A when result has faded out
	if (resultFinished)
	{
		WRITE_JUMP(0x10B96E6, (void*)0x10B974B);
	}

	originalResultUI_CHudResultAdvance(This, Edx, a2);

	// Restore original code
	if (resultFinished)
	{
		WRITE_MEMORY(0x10B96E6, uint8_t, 0xE8, 0x85, 0xD2, 0xFF, 0xFF);
		resultFinished = false;
	}
}

// MsgRestartStage for CScoreManager
HOOK(void, __fastcall, CScoreManager_MsgRestartStage, 0xCF7F10, uint32_t* This, void* Edx, void* message)
{
	originalCScoreManager_MsgRestartStage(This, Edx, message);

	// Grab ptr here
	ResultUI::m_pCScoreManager = (ResultUI::CScoreManager*)This;
}

HOOK(uint32_t*, __fastcall, CScoreManager_Destructor, 0x588AF0, uint32_t* This, void* Edx, bool a2)
{
	ResultUI::m_pCScoreManager = nullptr;
	return originalCScoreManager_Destructor(This, Edx, a2);
}

void ResultUI::applyPatches()
{
	if (!m_init) return;

	if (GetModuleHandle(TEXT("STH2006ProjectExtra.dll")) != nullptr)
	{
		INSTALL_HOOK(CScoreManager_MsgRestartStage);
		INSTALL_HOOK(CScoreManager_Destructor);
		m_scoreSystem = ScoreSystemType::SST_STH2006;
	}
	else if (GetModuleHandle(TEXT("ScoreGenerations.dll")) != nullptr)
	{
		m_scoreSystem = ScoreSystemType::SST_ScoreGens;
	}

	INSTALL_HOOK(ResultUI_MsgStartGoalResult);
	INSTALL_HOOK(ResultUI_CEventScene);
	INSTALL_HOOK(ResultUI_CStateGoalFadeBefore);
	INSTALL_HOOK(ResultUI_CHudResultAdvance);
}

bool ResultUI::initTextures()
{
	std::wstring const dir = Application::getModDirWString();
	m_init = true;

	wchar_t rank[5] = { L'D', L'C', L'B', L'A', L'S' };
	for (int i = 0; i < 5; i++)
	{
		m_init &= UIContext::loadTextureFromFile((dir + L"Assets\\Result\\Result_" + rank[i] + L".dds").c_str(), &m_resultRankTextures[i]);
	}
	
	for (int i = 0; i < 10; i++)
	{
		m_init &= UIContext::loadTextureFromFile((dir + L"Assets\\Result\\Result_" + std::to_wstring(i) + L".dds").c_str(), &m_resultNumTextures[i]);
	}

	for (int i = 0; i < RTT_COUNT; i++)
	{
		std::wstring file;
		switch (i)
		{
		case RTT_Score:		 file = L"Result_TextScore.dds"; break;
		case RTT_Time:		 file = L"Result_TextTime.dds"; break;
		case RTT_Rings:		 file = L"Result_TextRings.dds"; break;
		case RTT_TimeBonus:	 file = L"Result_TextTimeBonus.dds"; break;
		case RTT_RingBonus:	 file = L"Result_TextRingBonus.dds"; break;
		case RTT_TotalScore: file = L"Result_TextTotalScore.dds"; break;
		case RTT_Rank:		 file = L"Result_TextRank.dds"; break;
		default:			 m_init = false; break;
		}
		m_init &= UIContext::loadTextureFromFile((dir + L"Assets\\Result\\" + file).c_str(), &m_resultTextTextures[i]);
	}
	
	m_init &= UIContext::loadTextureFromFile((dir + L"Assets\\Result\\Result_Comma.dds").c_str(), &m_resultCommaTexture);
	m_init &= UIContext::loadTextureFromFile((dir + L"Assets\\Result\\Result_Box.dds").c_str(), &m_resultBoxTexture);
	m_init &= UIContext::loadTextureFromFile((dir + L"Assets\\Result\\Result_Total.dds").c_str(), &m_resultTotalBoxTexture);
	m_init &= UIContext::loadTextureFromFile((dir + L"Assets\\Result\\Result_Score.dds").c_str(), &m_resultScoreBoxTexture);
	m_init &= UIContext::loadTextureFromFile((dir + L"Assets\\Result\\Result_Board.dds").c_str(), &m_resultHeaderBoxTexture);
	m_init &= UIContext::loadTextureFromFile((dir + L"Assets\\Result\\Result_BoardShadow.dds").c_str(), &m_resultHeaderShadowTexture);
	m_init &= UIContext::loadTextureFromFile((dir + L"Assets\\Result\\Result_Header.dds").c_str(), &m_resultHeaderTextTexture);
	m_init &= UIContext::loadTextureFromFile((dir + L"Assets\\Result\\Result_Rank.dds").c_str(), &m_resultRankBoxTexture);
	m_init &= UIContext::loadTextureFromFile((dir + L"Assets\\Result\\Result_Fade.dds").c_str(), &m_resultFadeTexture);

	if (!m_init)
	{
		MessageBox(nullptr, TEXT("Failed to load assets for result, reverting to Generations result screen"), TEXT("STH2006 Project"), MB_ICONWARNING);
	}
	else
	{
		applyPatches();
	}

	return m_init;
}

ResultUI::ResultUIData ResultUI::m_resultUIData;
void ResultUI::draw()
{
	if (!m_init) return;
	m_resultUIData.draw();
}

void ResultUI::clearDraw()
{
	m_resultUIData.reset();
}

ResultUI::ResultUIBox::ResultUIBox(ResultUI::ResultTextType type, ImVec2 pos)
: m_type(type), m_finalPos(pos)
{
	switch (m_type)
	{
	case RTT_TotalScore:
	{
		m_texture = &ResultUI::m_resultTotalBoxTexture;
		m_textureSize = ImVec2(620, 77);
		m_textOffset = ImVec2(10, 13);
		break;
	}
	case RTT_Rank:
	{
		m_texture = &ResultUI::m_resultRankBoxTexture;
		m_textureSize = ImVec2(438, 120);
		m_textOffset = ImVec2(9, 55);
		break;
	}
	default:
	{
		m_texture = &ResultUI::m_resultScoreBoxTexture;
		m_textureSize = ImVec2(472, 70);
		m_textOffset = ImVec2(8, 8);
		break;
	}
	}

	reset();
}

void ResultUI::ResultUIBox::reset()
{
	m_started = false;
	switch (m_type)
	{
	case RTT_TotalScore:
	{
		m_posX = m_finalPos.x - 85.0f * 8.0f;
		break;
	}
	case RTT_Rank:
	{
		m_posX = m_finalPos.x - 64.0f * 6.0f;
		break;
	}
	default:
	{
		m_posX = m_finalPos.x - 96.0f * 6.0f;
		break;
	}
	}
	m_text = "0";
	m_alpha = 0.0f;
}

void ResultUI::ResultUIBox::draw(float scaleX, float scaleY)
{
	if (!m_started)
	{
		return;
	}
	else if (m_posX <= m_finalPos.x)
	{
		float frameAdd = Application::getHudDeltaTime() * 60.0f;
		float posAdd = 0;
		switch (m_type)
		{
		case RTT_TotalScore:
		{
			posAdd = frameAdd * 85.0f;
			m_alpha = 1.0f;
			break;
		}
		case RTT_Rank:
		{
			posAdd = frameAdd * 64.0f;
			m_alpha = min(1.0f, m_alpha + frameAdd / 6.0f);
			break;
		}
		default:
		{
			posAdd = frameAdd * 96.0f;
			m_alpha = 1.0f;
			break;
		}
		}
		m_posX = min(m_finalPos.x, m_posX + posAdd);
	}

	ImGui::SetCursorPos(ImVec2(m_posX * scaleX, m_finalPos.y * scaleY));
	ImGui::Image(*m_texture, ImVec2(m_textureSize.x * scaleX, m_textureSize.y * scaleY), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, m_alpha));
	ImGui::SetCursorPos(ImVec2((m_posX + m_textOffset.x) * scaleX, (m_finalPos.y + m_textOffset.y) * scaleY));
	ImGui::Image(m_resultTextTextures[m_type], ImVec2(280 * scaleX, 32 * scaleY), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, m_alpha));

	if (m_type != ResultTextType::RTT_Rank)
	{
		ImVec2 offset = (m_type == ResultTextType::RTT_TotalScore) ? ImVec2(534, 14) : ImVec2(412, 14);
		ImVec2 numSize = (m_type == ResultTextType::RTT_TotalScore) ? ImVec2(26, 30) : ImVec2(18, 22);
		ImVec2 commaSize = (m_type == ResultTextType::RTT_TotalScore) ? ImVec2(9, 13) : ImVec2(6, 8);

		for (auto iter = m_text.rbegin(); iter != m_text.rend(); iter++)
		{
			int num = *iter - '0';
			if (num >= 0 && num <= 9)
			{
				offset.x -= numSize.x;
				ImGui::SetCursorPos(ImVec2((m_posX + offset.x) * scaleX, (m_finalPos.y + offset.y) * scaleY));
				ImGui::Image(m_resultNumTextures[num], ImVec2(numSize.x * scaleX, numSize.y * scaleY));
			}
			else
			{
				offset.x -= commaSize.x;
				ImGui::SetCursorPos(ImVec2((m_posX + offset.x) * scaleX, (m_finalPos.y + offset.y) * scaleY));
				ImGui::Image(m_resultCommaTexture, ImVec2(commaSize.x * scaleX, commaSize.y * scaleY));
			}
		}
	}
}

ResultUI::ResultUIData::ResultUIData()
{
	for (int i = 0; i < ResultTextType::RTT_COUNT; i++)
	{
		ImVec2 pos(0, 0);
		switch (i)
		{
		case RTT_Score:		 pos = ImVec2(57,165); break;
		case RTT_Time:		 pos = ImVec2(57,225); break;
		case RTT_Rings:		 pos = ImVec2(57,285); break;
		case RTT_TimeBonus:	 pos = ImVec2(57,385); break;
		case RTT_RingBonus:	 pos = ImVec2(57,445); break;
		case RTT_TotalScore: pos = ImVec2(57,566); break;
		case RTT_Rank:		 pos = ImVec2(803,521); break;
		default: break;
		}
		m_boxes[i] = ResultUIBox((ResultTextType)i, pos);
	}

	reset();
}

static float topBorderPosY = 0;
static float bottomBorderPosY = 0;
static float rightBoxPosX = 0;
static PDIRECT3DTEXTURE9* rankTexture = nullptr;
static float rankScale = 2.5f;
void ResultUI::ResultUIData::reset()
{
	topBorderPosY = -38 - 9.0f * 12.0f;
	bottomBorderPosY = 600 + 9.0f * 12.0f;
	rightBoxPosX = 799 + 40.0f * 20.0f;
	rankTexture = nullptr;
	rankScale = 2.5f;

	for (int i = 0; i < ResultTextType::RTT_COUNT; i++)
	{
		m_boxes[i].reset();
	}

	// Get current score system
	if (m_scoreSystem == ScoreSystemType::SST_STH2006 && m_pCScoreManager)
	{
		m_currentScoreSystem = ScoreSystemType::SST_STH2006;
	}
	else if (m_scoreSystem == ScoreSystemType::SST_ScoreGens && !ScoreGenerationsAPI::IsStageForbidden())
	{
		m_currentScoreSystem = ScoreSystemType::SST_ScoreGens;
	}
	else
	{
		m_currentScoreSystem = ScoreSystemType::SST_None;
	}

	m_state = ResultState::RS_Idle;
	m_frame = 0.0f;
	m_scoreDestination = 0;
	m_scoreCount = 0;
}

static SharedPtrTypeless soundHandle_boxIn;
static SharedPtrTypeless soundHandle_countUp;
void ResultUI::ResultUIData::draw()
{
	if (m_state == ResultState::RS_Idle) return;

	float scaleX = *BACKBUFFER_WIDTH / 1280.0f;
	float scaleY = *BACKBUFFER_HEIGHT / 720.0f;

	switch (m_state)
	{
	case RS_Idle: break;
	case RS_Border:
	case RS_RightBox:
	{
		// frame: 29
		if (m_state == ResultState::RS_Border && m_frame <= 20.0f)
		{
			m_state = ResultState::RS_RightBox;
			Common::PlaySoundStatic(soundHandle_boxIn, 1010004);
		}

		topBorderPosY = -38 - 9.0f * max(0.0f, m_frame - 17.0f);
		bottomBorderPosY = 600 + 9.0f * max(0.0f, m_frame - 17.0f);
		rightBoxPosX = 799 + 40.0f * max(0.0f, m_frame - 9.0f);
		break;
	}
	case RS_TimeBounsCount:
	{
		// frame: 14
		if (m_scoreCount != m_scoreDestination)
		{
			m_frame = 14.0f;
			countScore(ResultTextType::RTT_TimeBonus);
		}
		break;
	}
	case RS_RingBonusCount:
	{
		// frame: 20
		if (m_scoreCount != m_scoreDestination)
		{
			m_frame = 20.0f;
			countScore(ResultTextType::RTT_RingBonus);
		}
		break;
	}
	case RS_TotalScoreCount:
	{
		// frame: 14
		if (m_scoreCount != m_scoreDestination)
		{
			m_frame = 14.0f;
			countScore(ResultTextType::RTT_TotalScore);
		}
		break;
	}
	case RS_RankShow:
	{
		// frame: 110
		if (m_frame >= 105.0f)
		{
			// 2.5 -> 0.9
			rankScale = 2.5f - 0.32f * (110.f - m_frame);
		}
		else if (m_frame >= 100.0f)
		{
			// 0.9 -> 1.0
			rankScale = 0.9f + 0.02f * (105.f - m_frame);
		}
		else
		{
			rankScale = 1.0f;
		}
		break;
	}
	case RS_Finish:
	{
		if (Common::IsAtLoadingScreen())
		{
			reset();
			return;
		}
		break;
	}
	default: break;
	}

	static bool visible = true;
	ImGui::Begin((std::string("ResultBorder")).c_str(), &visible, UIContext::m_hudFlags);
	{
		ImGui::SetWindowFocus();
		ImGui::SetWindowPos(ImVec2(-5, -5));

		// Top border + text
		ImGui::SetCursorPos(ImVec2(-104 * scaleX, topBorderPosY * scaleY));
		ImGui::Image(m_resultHeaderShadowTexture, ImVec2(1490 * scaleX, 160 * scaleY), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 0.5f));
		ImGui::SetCursorPos(ImVec2(-104 * scaleX, topBorderPosY * scaleY));
		ImGui::Image(m_resultHeaderBoxTexture, ImVec2(1490 * scaleX, 160 * scaleY), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 0.5f));
		ImGui::SetCursorPos(ImVec2(495 * scaleX, (topBorderPosY + 70) * scaleY));
		ImGui::Image(m_resultHeaderTextTexture, ImVec2(300 * scaleX, 100 * scaleY));
		
		// Bottom border
		ImGui::SetCursorPos(ImVec2(-100 * scaleX, bottomBorderPosY * scaleY));
		ImGui::Image(m_resultHeaderShadowTexture, ImVec2(1490 * scaleX, 160 * scaleY), ImVec2(1, 1), ImVec2(0, 0), ImVec4(1, 1, 1, 0.5f));
		ImGui::SetCursorPos(ImVec2(-100 * scaleX, bottomBorderPosY * scaleY));
		ImGui::Image(m_resultHeaderBoxTexture, ImVec2(1490 * scaleX, 160 * scaleY), ImVec2(1, 1), ImVec2(0, 0), ImVec4(1, 1, 1, 0.5f));

		// Box on the right
		ImGui::SetCursorPos(ImVec2(rightBoxPosX * scaleX, 198 * scaleY));
		ImGui::Image(m_resultBoxTexture, ImVec2(800 * scaleX, 170 * scaleY), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 0.7f));
	}
	ImGui::End();

	ImGui::Begin((std::string("ScoreBoxes")).c_str(), &visible, UIContext::m_hudFlags);
	{
		ImGui::SetWindowFocus();
		ImGui::SetWindowPos(ImVec2(-5, -5));
		for (int i = 0; i < ResultTextType::RTT_COUNT; i++)
		{
			m_boxes[i].draw(scaleX, scaleY);
		}

		// Draw rank
		if (rankTexture)
		{
			float size = 180 * rankScale;
			ImGui::SetCursorPos(ImVec2((1113 - size * 0.5f) * scaleX, (538 - size * 0.5f) * scaleY));
			ImGui::Image(*rankTexture, ImVec2(size * scaleX, size * scaleY));
		}

		// Fade out
		if (m_state >= ResultState::RS_FadeOut)
		{
			ImGui::SetCursorPos(ImVec2(-5, -5));
			ImGui::Image(m_resultFadeTexture, ImVec2((float)*BACKBUFFER_WIDTH * 1.1f, (float)*BACKBUFFER_HEIGHT * 1.1f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, min(1.0f, 1.0f - (m_frame - 20.0f) / 120.0f)));
		}

		// Dummy image at the bottom right
		ImGui::SetCursorPos(ImVec2((*BACKBUFFER_WIDTH + 5.0f) * scaleX, (*BACKBUFFER_WIDTH + 5.0f) * scaleY));
		ImGui::Image(m_resultCommaTexture, ImVec2(1, 1));
	}
	ImGui::End();

	if (m_state != ResultState::RS_Finish)
	{
		m_frame -= Application::getHudDeltaTime() * 60.0f;
		if (m_frame <= 0.0f)
		{
			nextState();
		}
	}
}

void ResultUI::ResultUIData::nextState()
{
	// Finished the black screen delay, exit
	if (m_state == ResultState::RS_Finish)
	{
		reset();
		return;
	}

	m_state = (ResultState)(m_state + 1);

	// Skip score state if not enabled
	if (m_state == ResultState::RS_Score && m_currentScoreSystem == ScoreSystemType::SST_None)
	{
		m_state = (ResultState)(m_state + 1);
	}

	switch (m_state)
	{
	case RS_Idle: break;
	case RS_Border: 
	{
		printf("[ResultUI] State: Border\n");
		m_frame = 29.0f; 
		break;
	}
	case RS_Score: 
	{			 
		printf("[ResultUI] State: Score\n");
		ResultUIBox& box = m_boxes[ResultTextType::RTT_Score];
		box.m_started = true;

		if (m_currentScoreSystem == ScoreSystemType::SST_STH2006 && m_pCScoreManager)
		{
			box.m_text = std::to_string(m_pCScoreManager->m_score);
		}
		else
		{
			box.m_text = std::to_string(ScoreGenerationsAPI::GetScore());
		}

		m_frame = 10.0f; 
		break; 
	}
	case RS_Time: 
	{			
		printf("[ResultUI] State: Time\n");
		ResultUIBox& box = m_boxes[ResultTextType::RTT_Time];
		box.m_started = true;
		box.m_finalPos = ImVec2(57, m_currentScoreSystem != ScoreSystemType::SST_None ? 225.f : 195.f);

		int millisecond = (int)(m_currentTime * 1000.0f) % 1000;
		int second = (int)m_currentTime % 60;
		int minute = (int)m_currentTime / 60;
		static char buffer[16];
		sprintf(buffer, "%01d'%02d''%03d", minute, second, millisecond);
		box.m_text = std::string(buffer);

		m_frame = 10.0f; 
		break; 
	}
	case RS_Rings: 
	{		 
		printf("[ResultUI] State: Rings\n");
		ResultUIBox& box = m_boxes[ResultTextType::RTT_Rings];
		box.m_started = true;
		box.m_text = std::to_string(*Common::GetPlayerRingCount());
		box.m_finalPos = ImVec2(57, m_currentScoreSystem != ScoreSystemType::SST_None ? 285.f : 255.f);

		m_frame = 28.0f; 
		break; 
	}
	case RS_TimeBonus: 
	{	 
		printf("[ResultUI] State: Time Bonus\n");
		ResultUIBox& box = m_boxes[ResultTextType::RTT_TimeBonus];
		box.m_started = true;
		box.m_finalPos = ImVec2(57, m_currentScoreSystem != ScoreSystemType::SST_None ? 385.f : 355.f);

		Common::PlaySoundStatic(soundHandle_boxIn, 1010004);
		m_frame = 18.0f; 
		break; 
	}
	case RS_RingBonus: 
	{	 
		printf("[ResultUI] State: Ring Bonus\n");
		ResultUIBox& box = m_boxes[ResultTextType::RTT_RingBonus];
		box.m_started = true;
		box.m_finalPos = ImVec2(57, m_currentScoreSystem != ScoreSystemType::SST_None ? 445.f : 415.f);

		m_frame = 20.0f; 
		break; 
	}
	case RS_TimeBounsCount: 
	{ 
		printf("[ResultUI] State: Time Bonus Count\n");
		if (m_currentScoreSystem == ScoreSystemType::SST_STH2006 && m_pCScoreManager)
		{
			m_scoreDestination = m_resultData->m_score - m_pCScoreManager->m_score - *Common::GetPlayerRingCount() * 100;
		}
		else if (m_currentScoreSystem == ScoreSystemType::SST_ScoreGens)
		{
			m_scoreDestination = ScoreGenerationsAPI::ComputeTimeBonus();
		}
		else
		{
			m_scoreDestination = m_resultData->m_score - min(5000, *Common::GetPlayerRingCount() * 50);
		}
		m_scoreCount = 0;

		if (m_scoreDestination != 0)
		{
			Common::PlaySoundStatic(soundHandle_countUp, 1010003);
		}
		m_frame = 14.0f; 
		break; 
	}
	case RS_RingBonusCount: 
	{ 
		printf("[ResultUI] State: Ring Bonus Count\n");
		if (m_currentScoreSystem != ScoreSystemType::SST_None)
		{
			m_scoreDestination = *Common::GetPlayerRingCount() * 100;
		}
		else
		{
			m_scoreDestination = min(5000, *Common::GetPlayerRingCount() * 50);
		}
		m_scoreCount = 0;

		if (m_scoreDestination != 0)
		{
			Common::PlaySoundStatic(soundHandle_countUp, 1010003);
		}
		m_frame = 20.0f; 
		break; 
	}
	case RS_TotalScore: 
	{ 
		printf("[ResultUI] State: Total Score\n");
		m_boxes[ResultTextType::RTT_TotalScore].m_started = true;
		m_scoreDestination = m_resultData->m_score;
		m_scoreCount = 0;

		Common::PlaySoundStatic(soundHandle_boxIn, 1010004);
		m_frame = 110.0f; 
		break; 
	}
	case RS_TotalScoreCount: 
	{ 
		printf("[ResultUI] State: Total Score Count\n");
		m_scoreCount = 0;

		if (m_scoreDestination != 0)
		{
			Common::PlaySoundStatic(soundHandle_countUp, 1010003);
		}
		m_frame = 14.0f; 
		break; 
	}
	case RS_Rank: 
	{	 
		printf("[ResultUI] State: Rank\n");
		m_boxes[ResultTextType::RTT_Rank].m_started = true;

		Common::PlaySoundStatic(soundHandle_boxIn, 1010004);
		m_frame = 115.0f; 
		break; 
	}
	case RS_RankShow: 
	{	 
		printf("[ResultUI] State: Rank Show\n");

		// Enable rank quote
		FUNCTION_PTR(void, __cdecl, EnableRankQuote, 0x10B77A8);
		WRITE_MEMORY(0x10B77AD, uint8_t, 0x58, 0xC3, 0x90, 0x90, 0x90);
		EnableRankQuote();

		// Use this to play rank quote
		FUNCTION_PTR(void*, __thiscall, CHudResult_CStateChangeRank, 0x10B76D0, void* This);
		WRITE_JUMP(0x10B76D0, (void*)0x11D2350);
		CHudResult_CStateChangeRank(nullptr);

		rankTexture = &m_resultRankTextures[m_resultData->m_perfectRank];

		m_frame = 110.0f;
		break; 
	}
	case RS_FadeOut:
	{	 
		printf("[ResultUI] State: Fade Out\n");
		m_frame = 140.0f; 
		break; 
	}
	case RS_Finish: 
	{
		printf("[ResultUI] State: Finish\n");
		m_resultData = nullptr;
		resultFinished = true;
		m_frame = 0.0f;
		break;
	}
	}
}

void ResultUI::ResultUIData::countScore(ResultTextType type)
{
	int digit = (m_scoreCount == 0) ? 1 : 0;
	int n = m_scoreCount;
	while (n)
	{
		n /= 10;
		digit++;
	}
	digit = max(1, digit - 1);

	int countUp = 1;
	for (int i = 0; i < digit; i++)
	{
		m_scoreCount += countUp;
		countUp *= 10;
	}

	// Skip counting up if player pressed A
	Sonic::SPadState* padState = Sonic::CInputState::GetPadState();
	if (padState->IsTapped(Sonic::EKeyState::eKeyState_A))
	{
		m_scoreCount = m_scoreDestination;
	}

	if (m_scoreCount >= m_scoreDestination)
	{
		soundHandle_countUp.reset();
		m_scoreCount = m_scoreDestination;
	}
	m_boxes[type].m_text = std::to_string(m_scoreCount);
}
