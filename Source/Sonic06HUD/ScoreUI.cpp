#include "ScoreUI.h"
#include "Configuration.h"
#include "Application.h"
#include "UIContext.h"

int ScoreUI::m_rainbowRingScore = 0;
int ScoreUI::m_physicsScore = 0;
float ScoreUI::m_bonusDrawTimer = 0.0f;
PDIRECT3DTEXTURE9 ScoreUI::m_bonus_Great = nullptr;

void ScoreUI::applyPatches()
{
	// Manual adjust scores
	Tables::ScoreTable scoreTable = ScoreGenerationsAPI::GetScoreTable();
	m_physicsScore = scoreTable.Physics;
	if (m_physicsScore > 0)
	{

	}
}

void __declspec(naked) ScoreUI_GetRainbow()
{
	static uint32_t returnAddress = 0x115A8FC;
	__asm
	{
		// set timer to 4.0f
		mov		ScoreUI::m_bonusDrawTimer, 40800000h

		// original code
		mov     eax, [esi + 0BCh]
		jmp		[returnAddress]
	}
}

void ScoreUI::applyUIPatches()
{
	// Apply hooks for displaying rainbow ring score
	Tables::ScoreTable scoreTable = ScoreGenerationsAPI::GetScoreTable();
	m_rainbowRingScore = scoreTable.RainbowRing;
	if (m_rainbowRingScore > 0)
	{
		WRITE_JUMP(0x115A8F6, ScoreUI_GetRainbow);
	}
}

bool ScoreUI::initTextures()
{
	// Only run this if player have Score Generations and NOT playing STH2006 Project
	if (GetModuleHandle(TEXT("ScoreGenerations.dll")) == nullptr || GetModuleHandle(TEXT("STH2006Project.dll")) != nullptr)
	{
		return false;
	}

	std::wstring const dir = Application::getModDirWString();
	bool success = true;
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Bonus\\Bonus_Great.dds").c_str(), &m_bonus_Great);
	
	if (!success)
	{
		MessageBox(nullptr, TEXT("Failed to load assets for score bonus texts, they will not be displayed."), TEXT("STH2006 Project"), MB_ICONWARNING);
	}
	else
	{
		applyUIPatches();
	}

	applyPatches();
	return success;
}

void ScoreUI::draw()
{
	if (Common::IsAtLoadingScreen())
	{
		m_bonusDrawTimer = 0.0f;
		return;
	}

	if (m_bonusDrawTimer > 0)
	{
		static bool visible = true;
		ImGui::Begin("Bonus", &visible, UIContext::m_hudFlags);
		{
			std::string const bonusStr = std::to_string(m_rainbowRingScore);
			ImVec2 size = ImGui::CalcTextSize(bonusStr.c_str());
			ImGui::SetWindowPos(ImVec2((float)*BACKBUFFER_WIDTH * 0.8f - size.x / 2, (float)*BACKBUFFER_HEIGHT * 0.295f - size.y / 2));
			ImGui::Text(bonusStr.c_str());
		}
		ImGui::End();

		if (m_bonus_Great)
		{
			ImGui::Begin("BonusComment", &visible, UIContext::m_hudFlags);
			{
				float sizeX = *BACKBUFFER_WIDTH * 300.0f / 1280.0f;
				float sizeY = *BACKBUFFER_HEIGHT * 50.0f / 720.0f;
				float posX = 0.645f;
				float posY = 0.347f;
				if (m_bonusDrawTimer > 3.9f)
				{
					posX = 1.0f - (1.0f - posX) * ((4.0f - m_bonusDrawTimer) / 0.1f);
				}

				ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * posX, *BACKBUFFER_HEIGHT * posY));
				ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
				ImGui::Image(m_bonus_Great, ImVec2(sizeX, sizeY));
			}
			ImGui::End();
		}

		m_bonusDrawTimer -= Application::getHudDeltaTime();
	}
}
