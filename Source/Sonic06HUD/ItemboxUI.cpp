#include "ItemboxUI.h"
#include "Application.h"
#include "UIContext.h"

std::deque<ItemboxGUI> ItemboxUI::m_guiData;
PDIRECT3DTEXTURE9 ItemboxUI::m_item_10ring = nullptr;
PDIRECT3DTEXTURE9 ItemboxUI::m_item_1up = nullptr;

HOOK(void, __fastcall, ItemboxUI_GetSuperRing, 0x11F2F10, uint32_t* This, void* Edx, void* message)
{
	ItemboxUI::addItemToGui(ItemboxType::IT_10ring);
	originalItemboxUI_GetSuperRing(This, Edx, message);
}

HOOK(void, __fastcall, ItemboxUI_Get1up, 0xFFF810, uint32_t* This, void* Edx, void* message)
{
	ItemboxUI::addItemToGui(ItemboxType::IT_1up);
	originalItemboxUI_Get1up(This, Edx, message);
}

void ItemboxUI::applyPatches()
{
	INSTALL_HOOK(ItemboxUI_GetSuperRing);
	INSTALL_HOOK(ItemboxUI_Get1up);

	// Make CPlayerSpeedStatePluginRingCountUp get all rings immediately
	WRITE_NOP(0xE4155E, 2);
	WRITE_NOP(0xE415C8, 2);
	WRITE_MEMORY(0xE4157A, int, -1);
}

void ItemboxUI::addItemToGui(ItemboxType type)
{
	if (type >= ItemboxType::IT_COUNT) return;

	// Initialize GUI data
	m_guiData.push_front(ItemboxGUI(type));

	// Only allow 5 to display at max
	if (m_guiData.size() > 5)
	{
		m_guiData.pop_back();
	}
}

bool ItemboxUI::initTextures()
{
	// Only run this if player have Sonic 06 Definitive Experience and NOT playing STH2006 Project
	if (GetModuleHandle(TEXT("Sonic06DefinitiveExperience.dll")) == nullptr || GetModuleHandle(TEXT("STH2006Project.dll")) != nullptr)
	{
		return false;
	}

	std::wstring const dir = Application::getModDirWString();
	bool success = true;
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_10ring.dds").c_str(), &m_item_10ring);
	success &= UIContext::loadTextureFromFile((dir + L"Assets\\Item\\Item_1up.dds").c_str(), &m_item_1up);

	if (!success)
	{
		MessageBox(nullptr, TEXT("Failed to load assets for itembox icons, they will not be displayed."), TEXT("Sonic 06 HUD"), MB_ICONWARNING);
	}
	else
	{
		applyPatches();
	}

	return success;
}

void ItemboxUI::draw()
{
	// At loading screen, clear all
	if (Common::IsAtLoadingScreen())
	{
		m_guiData.clear();
		return;
	}

	// Remove frames that has timed out
	while (!m_guiData.empty() && m_guiData.back().m_frame > 200.0f)
	{
		m_guiData.pop_back();
	}

	// 0-8: zoom in
	// 9: zoom to size
	// 10-200: stay
	for (int i = m_guiData.size() - 1; i >= 0; i--)
	{
		ItemboxGUI& data = m_guiData[i];

		PDIRECT3DTEXTURE9* texture = nullptr;
		switch (data.m_type)
		{
		case IT_10ring:	 texture = &m_item_10ring;	break;
		case IT_1up:	 texture = &m_item_1up;		break;
		}

		if (data.m_frame > 0.0f && texture)
		{
			static bool visible = true;
			ImGui::Begin((std::string("Itembox") + std::to_string(i)).c_str(), &visible, UIContext::m_hudFlags);
			{
				float sizeX = *BACKBUFFER_WIDTH * 192.0f / 1280.0f;
				float sizeY = *BACKBUFFER_HEIGHT * 192.0f / 720.0f;
				if (data.m_frame <= 8.0f)
				{
					float scale = data.m_frame / 8.0f * 1.12f;
					sizeX *= scale;
					sizeY *= scale;
				}
				else if (data.m_frame < 10.0f)
				{
					float scale = ((10.0f - data.m_frame) * 0.12f) + 1.0f;
					sizeX *= scale;
					sizeY *= scale;
				}

				float targetPos = 0.5f - (0.091f * i);
				data.m_pos += (targetPos - data.m_pos) * 0.15f * Application::getHudDeltaTime() * 60.0f;
				float posX = floorf(*BACKBUFFER_WIDTH * data.m_pos - sizeX * 0.5f - 8.0f);
				float posY = floorf(*BACKBUFFER_HEIGHT * 0.847f - sizeX * 0.5f - 8.0f);
				ImGui::SetWindowFocus();
				ImGui::SetWindowPos(ImVec2(posX, posY));
				ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
				ImGui::Image(*texture, ImVec2(sizeX, sizeY));
			}
			ImGui::End();
		}

		data.m_frame += Application::getHudDeltaTime() * 60.0f;
	}
}
