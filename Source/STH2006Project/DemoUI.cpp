#include "DemoUI.h"

Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectMenu;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuBG;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuBG2;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuBars;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuText;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuTextCover;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuTitleBarEffect;

DemoUI::CursorData m_cursor1Data;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuCursor1;
std::vector<std::string> const m_cursor1SelectName =
{
	"select_01",
	"select_02",
	"select_03",
	"select_04",
	"select_05",
};
std::vector<std::string> const m_cursor1LoopName =
{
	"loop_01",
	"loop_02",
	"loop_03",
	"loop_04",
	"loop_05",
};

DemoUI::CursorData m_cursor2Data;
std::vector<std::string> const m_cursor2SelectName =
{
	"select_episodeselect",
	"select_trialselect",
	"select_goldmedal",
	"select_download",
};
std::vector<std::string> const m_cursor2LoopName =
{
	"loop_episodeselect",
	"loop_trialselect",
	"loop_goldmedal",
	"loop_download",
};

Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuCursor2;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuCursor3;

HOOK(void, __fastcall, DemoUI_CGameplayFlowTrialMenuCallback, 0x11FB380, uint32_t* This)
{
	if (This[29] && *(bool*)(This[35] + 173))
	{
		if (m_projectMenu)
		{
			Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuBG);
			Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuBG2);
			Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuBars);
			Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuText);
			Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuTextCover);
			Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuTitleBarEffect);

			Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuCursor1);
			Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuCursor2);
			Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuCursor3);

			m_projectMenu = nullptr;
		}
	}

	originalDemoUI_CGameplayFlowTrialMenuCallback(This);
}

void DemoUI_CreateSingleScene(Chao::CSD::RCPtr<Chao::CSD::CScene>& scene, char const* name, bool loop = false)
{
	if (m_projectMenu)
	{
		scene = m_projectMenu->CreateScene(name);
		scene->m_MotionRepeatType = loop ? Chao::CSD::eMotionRepeatType_Loop : Chao::CSD::eMotionRepeatType_PlayOnce;
	}
}

HOOK(void, __fastcall, DemoUI_CDemoMenuObjectAdvance, 0x576470, uint32_t* This, void* Edx, const Hedgehog::Universe::SUpdateInfo& updateInfo)
{
	if (!m_projectMenu)
	{
		auto* originalProject = (Chao::CSD::RCPtr<Chao::CSD::CProject>*)(&This[45]);
		if (originalProject->Get())
		{
			m_projectMenu = *originalProject;

			DemoUI_CreateSingleScene(m_sceneMenuBG, "mainmenu_back");
			DemoUI_CreateSingleScene(m_sceneMenuBG2, "main_menu_back2", true);
			DemoUI_CreateSingleScene(m_sceneMenuBars, "main_menu_parts");
			DemoUI_CreateSingleScene(m_sceneMenuText, "text");
			DemoUI_CreateSingleScene(m_sceneMenuTextCover, "text_cover");
			DemoUI_CreateSingleScene(m_sceneMenuTitleBarEffect, "titlebar_effect");

			m_cursor1Data.m_index = 0;
			m_cursor1Data.m_hidden = false;
			DemoUI_CreateSingleScene(m_sceneMenuCursor1, "main_menu_cursor");

			m_cursor2Data.m_index = 0;
			m_cursor2Data.m_hidden = true;
			DemoUI_CreateSingleScene(m_sceneMenuCursor2, "main_menu_cursor2");
			m_sceneMenuCursor2->SetHideFlag(true);
		}
	}

	Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
	if (!m_cursor1Data.m_hidden)
	{
		if (padState->IsTapped(Sonic::EKeyState::eKeyState_LeftStickUp) && m_cursor1Data.m_index > 0)
		{
			m_cursor1Data.m_index--;
			DemoUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, m_cursor1SelectName, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_LeftStickDown) && m_cursor1Data.m_index < m_cursor1SelectName.size() - 1)
		{
			m_cursor1Data.m_index++;
			DemoUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, m_cursor1SelectName, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_A))
		{
			m_cursor1Data.m_hidden = true;
			DemoUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, m_cursor1SelectName, 1000005);

			m_cursor2Data.m_index = 0;
			m_cursor2Data.m_hidden = false;
			m_sceneMenuCursor2->SetPosition(0.0f, m_cursor1Data.m_index * 60.0f);
			DemoUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, m_cursor2SelectName);
		}
	}
	else if (!m_cursor2Data.m_hidden)
	{
		if (padState->IsTapped(Sonic::EKeyState::eKeyState_LeftStickUp) && m_cursor2Data.m_index > 0)
		{
			m_cursor2Data.m_index--;
			DemoUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, m_cursor2SelectName, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_LeftStickDown) && m_cursor2Data.m_index < m_cursor2SelectName.size() - 1)
		{
			m_cursor2Data.m_index++;
			DemoUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, m_cursor2SelectName, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_B))
		{
			m_cursor2Data.m_hidden = true;
			DemoUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, m_cursor2SelectName, 1000003);

			m_cursor1Data.m_hidden = false;
			DemoUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, m_cursor1SelectName);
		}
	}

	DemoUI::cursorLoop(m_cursor1Data, m_sceneMenuCursor1, m_cursor1LoopName);
	DemoUI::cursorLoop(m_cursor2Data, m_sceneMenuCursor2, m_cursor2LoopName);

	originalDemoUI_CDemoMenuObjectAdvance(This, Edx, updateInfo);
}

void DemoUI::applyPatches()
{
	INSTALL_HOOK(DemoUI_CGameplayFlowTrialMenuCallback);
	INSTALL_HOOK(DemoUI_CDemoMenuObjectAdvance);

	// Ignore original menu
	WRITE_JUMP(0x576523, (void*)0x576C61);
}

void DemoUI::cursorSelect(CursorData& data, Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, std::vector<std::string> const& selectNames, uint32_t soundCueID)
{
	if (!scene) return;

	data.m_index = max(0, min(selectNames.size() - 1, data.m_index));

	scene->SetMotion(selectNames[data.m_index].c_str());
	scene->m_MotionDisableFlag = false;
	scene->m_PrevMotionFrame = data.m_hidden ? scene->m_MotionEndFrame : 0.0f;
	scene->m_MotionFrame = data.m_hidden ? scene->m_MotionEndFrame : 0.0f;
	*(uint32_t*)((uint32_t)scene.Get() + 0xB0) = data.m_hidden ? 0 : 1;
	*(uint32_t*)((uint32_t)scene.Get() + 0xB4) = data.m_hidden ? 0 : 1; // this stops the reverse animation
	scene->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
	scene->m_MotionSpeed = data.m_hidden ? -1.0f : 1.0f;
	scene->Update();
	scene->SetHideFlag(false);

	if (soundCueID != 0xFFFFFFFF)
	{
		static SharedPtrTypeless soundHandle;
		Common::PlaySoundStatic(soundHandle, soundCueID);
	}
}

void DemoUI::cursorLoop(CursorData const& data, Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, std::vector<std::string> const& loopNames)
{
	if (!scene) return;

	if (scene->m_MotionDisableFlag)
	{
		scene->SetMotion(loopNames[data.m_index].c_str());
		if (data.m_hidden)
		{
			scene->SetHideFlag(true);
			return;
		}

		scene->m_MotionDisableFlag = false;
		scene->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_Loop;
	}
}
