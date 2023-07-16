#include "ScoreUI.h"
#include "Configuration.h"
#include "Application.h"
#include "UIContext.h"

uint32_t ScoreUI::m_rainbowRingChain = 0;
int ScoreUI::m_bonus = 0;
float ScoreUI::m_bonusTimer = 0.0f;
float ScoreUI::m_bonusDrawTimer = 0.0f;
IUnknown** ScoreUI::m_bonusTexture = nullptr;
IUnknown* ScoreUI::m_bonus_Great = nullptr;

void __declspec(naked) ScoreUI_GetRainbow()
{
	static uint32_t returnAddress = 0x115A8FC;
	__asm
	{
		push	esi
		add		ScoreUI::m_rainbowRingChain, 1
		call	ScoreUI::addRainbowScore
		pop		esi

		// original code
		mov     eax, [esi + 0BCh]
		jmp		[returnAddress]
	}
}

HOOK(void, __fastcall, ScoreUI_CHudSonicStageUpdate, 0x1098A50, void* This, void* Edx, float* dt)
{
	// Bonus timer
	ScoreUI::m_bonusTimer = max(0.0f, ScoreUI::m_bonusTimer - *dt);
	if (ScoreUI::m_bonusTimer == 0.0f)
	{
		ScoreUI::m_bonus = 0;
		ScoreUI::m_rainbowRingChain = 0;
	}

	ScoreUI::m_bonusDrawTimer = max(0.0f, ScoreUI::m_bonusDrawTimer - *dt);
	if (ScoreUI::m_bonusDrawTimer == 0.0f)
	{
		ScoreUI::m_bonusTexture = nullptr;
	}

	originalScoreUI_CHudSonicStageUpdate(This, Edx, dt);
}

HOOK(void, __stdcall, ScoreUI_AddPhysicsScore, 0xEA49B0, uint32_t This, int a2, void* a3, bool a4)
{
	// Check if it's a breakable object
	if (!*(bool*)(This + 0x120))
	{
		if (!a4 || *(uint32_t*)(This + 0x108) == 1)
		{
			// Manually do Object Physics score except for cmn_itembox_lock
			std::string name(*(char**)(This + 0x130));
			if (name.find("cmn_itembox_lock") == std::string::npos)
			{
				ScoreGenerationsAPI::AddScore(20);
			}
		}
	}

	originalScoreUI_AddPhysicsScore(This, a2, a3, a4);
}

void ScoreUI::applyPatches()
{
	// Apply hooks for displaying rainbow ring score
	WRITE_JUMP(0x115A8F6, ScoreUI_GetRainbow);
	INSTALL_HOOK(ScoreUI_CHudSonicStageUpdate);

	// Maunally add score for object physics (cuz Score Gens is bugged)
	INSTALL_HOOK(ScoreUI_AddPhysicsScore);
}

int ScoreUI::calculateRainbowRingChainBonus()
{
	switch (m_rainbowRingChain)
	{
	case 1: return 1000;
	case 2: return 1600;
	case 3: return 2000;
	case 4: return 3400;
	default: return 4000 + (m_rainbowRingChain - 5) * 600;
	}
}

void __fastcall ScoreUI::addRainbowScore()
{
	// Add to rainbow ring stack bonus and notify draw GUI
	int score = calculateRainbowRingChainBonus();
	ScoreGenerationsAPI::AddScore(score);
	m_bonus += score;

	// Always clear texture first
	if (m_bonusTexture)
	{
		m_bonusTexture = nullptr;
	}

	// Only draw comment if there not already one
	if (m_bonusDrawTimer == 0.0f)
	{
		m_bonusTexture = &m_bonus_Great;
	}

	m_bonusTimer = 10.0f;
	m_bonusDrawTimer = 4.0f;
}

bool ScoreUI::initTextures()
{
	// Only run this if player have Score Generations and NOT playing STH2006 Project
	if (!Common::IsModEnabled("Main", "DLLFile", "ScoreGenerations.dll") || Configuration::m_usingSTH2006Project)
	{
		return false;
	}

	std::wstring const dir = Application::getModDirWString();
	bool success = true;
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Bonus\\Bonus_Great.dds").c_str(), &m_bonus_Great);
	
	if (!success)
	{
		MessageBox(nullptr, TEXT("Failed to load assets for score bonus texts, they may not display correctly."), TEXT("Sonic 06 HUD"), MB_ICONWARNING);
	}

	applyPatches();
	return success;
}

void ScoreUI::draw()
{
	// At loading screen, clear all
	if (Common::IsAtLoadingScreen())
	{
		clearDraw();
		return;
	}

	if (m_bonus > 0 && m_bonusDrawTimer > 0)
	{
		static bool visible = true;
		ImGui::Begin("Bonus", &visible, UIContext::m_hudFlags);
		{
			std::string const bonusStr = std::to_string(m_bonus);
			ImVec2 size = ImGui::CalcTextSize(bonusStr.c_str());
			ImGui::SetWindowPos(ImVec2((float)*BACKBUFFER_WIDTH * 0.8f - size.x / 2, (float)*BACKBUFFER_HEIGHT * 0.295f - size.y / 2));
			ImGui::Text(bonusStr.c_str());
		}
		ImGui::End();

		if (m_bonusTexture)
		{
			ImGui::Begin("BonusComment", &visible, UIContext::m_hudFlags);
			{
				float sizeX = *BACKBUFFER_WIDTH * 300.0f / 1280.0f;
				float sizeY = *BACKBUFFER_HEIGHT * 50.0f / 720.0f;
				float posX = 0.64083f;
				float posY = 0.34052f;
				if (m_bonusDrawTimer > 3.9f)
				{
					posX = 1.0f - (1.0f - posX) * ((4.0f - m_bonusDrawTimer) / 0.1f);
				}

				ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * posX, *BACKBUFFER_HEIGHT * posY));
				ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
				ImGui::Image(*m_bonusTexture, ImVec2(sizeX, sizeY));
			}
			ImGui::End();
		}
	}
}

void ScoreUI::clearDraw()
{
	m_bonus = 0;
	m_bonusDrawTimer = 0.0f;
	m_rainbowRingChain = 0;
}
