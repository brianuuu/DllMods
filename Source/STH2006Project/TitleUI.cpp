#include "TitleUI.h"
#include "UIContext.h"

FUNCTION_PTR(void, __thiscall, TitleUI_TinyChangeState, 0x773250, void* This, SharedPtrTypeless& spState, const Hedgehog::Base::CSharedString name);

boost::shared_ptr<Sonic::CGameObjectCSD> m_spTitle;
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectTitle;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneTitle;

void TitleUI_PlayMotion(Chao::CSD::RCPtr<Chao::CSD::CScene>& scene, char const* motion, bool loop = false, float start = 0.0f, float end = -1.0f)
{
	if (!scene) return;
	scene->SetHideFlag(false);
	scene->SetMotion(motion);
	scene->SetMotionFrame(start);
	if (end > start)
	{
		scene->m_MotionEndFrame = end;
	}
	scene->m_MotionDisableFlag = false;
	scene->m_MotionSpeed = 1.0f;
	scene->m_MotionRepeatType = loop ? Chao::CSD::eMotionRepeatType_Loop : Chao::CSD::eMotionRepeatType_PlayOnce;
	scene->Update();
}

TitleState titleState = TitleState::TS_FadeIn;
HOOK(void, __fastcall, TitleUI_TitleCMainCState_WaitStartBegin, 0x571DB0, hh::fnd::CStateMachineBase::CStateBase* This)
{
	Sonic::CGameObject* gameObject = (Sonic::CGameObject*)(This->GetContextBase());
	Sonic::CCsdDatabaseWrapper wrapper(gameObject->m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());

	auto spCsdProject = wrapper.GetCsdProject("title_English");
	m_projectTitle = spCsdProject->m_rcProject;

	m_sceneTitle = m_projectTitle->CreateScene("Scene_Title");
	TitleUI_PlayMotion(m_sceneTitle, "Title_Open");

	if (m_projectTitle && !m_spTitle)
	{
		m_spTitle = boost::make_shared<Sonic::CGameObjectCSD>(m_projectTitle, 0.5f, "HUD", false);
		Sonic::CGameDocument::GetInstance()->AddGameObject(m_spTitle, "main", gameObject);
	}

	titleState = TitleState::TS_FadeIn;
	originalTitleUI_TitleCMainCState_WaitStartBegin(This);
}

HOOK(void, __fastcall, TitleUI_TitleCMainCState_WaitStartAdvance, 0x571F80, uint32_t This)
{
	originalTitleUI_TitleCMainCState_WaitStartAdvance(This);

	uint32_t caseIndex = *(uint32_t*)(This + 36);
	if (caseIndex == 6 && titleState == 1)
	{
		// Pressed start
		titleState = TitleState::TS_Confirm;
		TitleUI_PlayMotion(m_sceneTitle, "Title_Close_01");
	}
	else if (caseIndex == 1 && titleState == 1)
	{
		// Idle too long
		titleState = TitleState::TS_FadeOut;
		TitleUI_PlayMotion(m_sceneTitle, "Title_Close_03");
	}

	if (m_sceneTitle && m_sceneTitle->m_MotionDisableFlag)
	{
		switch (titleState)
		{
		case 0:
			titleState = TitleState::TS_Wait;
			TitleUI_PlayMotion(m_sceneTitle, "Title_Loop", true);
			break;
		case 2:
			titleState = TitleState::TS_FadeOut;
			TitleUI_PlayMotion(m_sceneTitle, "Title_Close_03");
			break;
		case 3:
			titleState = TitleState::TS_End;
			break;
		}
	}
}

HOOK(void, __fastcall, TitleUI_TitleCMainCState_WaitStartEnd, 0x571EE0, hh::fnd::CStateMachineBase::CStateBase* This)
{
	if (m_spTitle)
	{
		m_spTitle->SendMessage(m_spTitle->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
		m_spTitle = nullptr;
	}

	Chao::CSD::CProject::DestroyScene(m_projectTitle.Get(), m_sceneTitle);
	m_projectTitle = nullptr;

	originalTitleUI_TitleCMainCState_WaitStartEnd(This);
}

void __declspec(naked) TitleUI_TitleCMainFadeInCompleted()
{
	// Wait until title animation finishes
	static uint32_t returnAddress = 0x571FF2;
	__asm
	{
		cmp		titleState, TS_Wait
		jmp		[returnAddress]
	}
}

void __declspec(naked) TitleUI_TitleCMainToSelectMenu()
{
	// Wait until title animation finishes
	static uint32_t returnAddress = 0x57258D;
	__asm
	{
		cmp		titleState, TS_End
		jmp		[returnAddress]
	}
}

MenuState m_menuState = MenuState::MS_FadeIn;

boost::shared_ptr<Sonic::CGameObjectCSD> m_spMenuBG;
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectMenuBG;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuBG;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuBG2;

boost::shared_ptr<Sonic::CGameObjectCSD> m_spMenu;
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectMenu;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuBars;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuText;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuTextCover;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuTitleBarEffect;

TitleUI::CursorData m_cursor1Data;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuCursor1;
TitleUI::CursorData m_cursor2Data;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuCursor2;
std::vector<std::string> const m_cursorSelectName =
{
	"select_01",
	"select_02",
	"select_03",
	"select_04",
	"select_05",
};
std::vector<std::string> const m_cursorLoopName =
{
	"loop_01",
	"loop_02",
	"loop_03",
	"loop_04",
	"loop_05",
};

HOOK(void, __fastcall, TitleUI_TitleCMainCState_SelectMenuBegin, 0x572750, hh::fnd::CStateMachineBase::CStateBase* This)
{
	Sonic::CGameObject* gameObject = (Sonic::CGameObject*)(This->GetContextBase());
	Sonic::CCsdDatabaseWrapper wrapper(gameObject->m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());

	auto spCsdProject = wrapper.GetCsdProject("background");
	m_projectMenuBG = spCsdProject->m_rcProject;

	m_sceneMenuBG = m_projectMenuBG->CreateScene("mainmenu_back");
	m_sceneMenuBG2 = m_projectMenuBG->CreateScene("main_menu");

	if (m_projectMenuBG && !m_spMenuBG)
	{
		m_spMenuBG = boost::make_shared<Sonic::CGameObjectCSD>(m_projectMenuBG, 0.5f, "HUD_A1", false);
		Sonic::CGameDocument::GetInstance()->AddGameObject(m_spMenuBG, "main", gameObject);
	}

	spCsdProject = wrapper.GetCsdProject("main_menu");
	m_projectMenu = spCsdProject->m_rcProject;

	m_sceneMenuBars = m_projectMenu->CreateScene("main_menu_parts");
	m_sceneMenuBars->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
	m_sceneMenuText = m_projectMenu->CreateScene("text");
	m_sceneMenuText->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
	m_sceneMenuTextCover = m_projectMenu->CreateScene("text_cover");
	m_sceneMenuTextCover->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
	m_sceneMenuTitleBarEffect = m_projectMenu->CreateScene("titlebar_effect");
	m_sceneMenuTitleBarEffect->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;

	m_cursor1Data.m_index = 0;
	m_cursor1Data.m_hidden = true;
	m_sceneMenuCursor1 = m_projectMenu->CreateScene("main_menu_cursor");
	m_sceneMenuCursor1->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
	m_sceneMenuCursor1->SetHideFlag(true);

	m_cursor2Data.m_index = 0;
	m_cursor2Data.m_hidden = true;
	m_sceneMenuCursor2 = m_projectMenu->CreateScene("main_menu_cursor");
	m_sceneMenuCursor2->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
	m_sceneMenuCursor2->SetHideFlag(true);

	if (m_projectMenu && !m_spMenu)
	{
		m_spMenu = boost::make_shared<Sonic::CGameObjectCSD>(m_projectMenu, 0.5f, "HUD_A2", false);
		Sonic::CGameDocument::GetInstance()->AddGameObject(m_spMenu, "main", gameObject);
	}

	m_menuState = MenuState::MS_FadeIn;
	originalTitleUI_TitleCMainCState_SelectMenuBegin(This);
}

HOOK(void, __fastcall, TitleUI_TitleCMainCState_SelectMenuAdvance, 0x5728F0, hh::fnd::CStateMachineBase::CStateBase* This)
{
	//originalTitleUI_TitleCMainCState_SelectMenuAdvance(This);

	uint32_t* owner = (uint32_t*)(This->GetContextBase());
	uint32_t* outState = &owner[111];
	static SharedPtrTypeless spOutState;

	Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
	switch (m_menuState)
	{
	case MenuState::MS_FadeIn:
	{
		if (m_sceneMenuBars && m_sceneMenuBars->m_MotionDisableFlag)
		{
			// wait until bar finish animation then show cursor
			m_cursor1Data.m_hidden = false;
			TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, m_cursorSelectName);

			m_menuState = MenuState::MS_Main;
		}
		break;
	}
	case MenuState::MS_Main:
	{
		if (padState->IsTapped(Sonic::EKeyState::eKeyState_LeftStickUp) && m_cursor1Data.m_index > 0)
		{
			m_cursor1Data.m_index--;
			TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, m_cursorSelectName, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_LeftStickDown) && m_cursor1Data.m_index < MenuType::MT_COUNT - 1)
		{
			m_cursor1Data.m_index++;
			TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, m_cursorSelectName, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_A))
		{
			switch (m_cursor1Data.m_index)
			{
			case MenuType::MT_TrialSelect:
			{
				m_cursor1Data.m_hidden = true;
				TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, m_cursorSelectName, 1000005);

				m_cursor2Data.m_index = 0;
				m_cursor2Data.m_hidden = false;
				m_sceneMenuCursor2->SetPosition(99.0f, (m_cursor1Data.m_index + 1) * 60.0f);
				TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, m_cursorSelectName);

				m_menuState = MenuState::MS_TrialSelect;
				break;
			}
			case MenuType::MT_QuitGame:
			{
				*(bool*)0x1E5E2E8 = true;
				//*outState = 8;
				//TitleUI_TinyChangeState(This, spOutState, "ExecSubMenu");
				break;
			}
			}
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_B))
		{
			TitleUI_TinyChangeState(This, spOutState, "Init");
		}
		break;
	}
	case MenuState::MS_TrialSelect:
	{
		if (padState->IsTapped(Sonic::EKeyState::eKeyState_LeftStickUp) && m_cursor2Data.m_index > 0)
		{
			m_cursor2Data.m_index--;
			TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, m_cursorSelectName, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_LeftStickDown) && m_cursor2Data.m_index < TrialMenuType::TMT_COUNT - 1)
		{
			m_cursor2Data.m_index++;
			TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, m_cursorSelectName, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_B))
		{
			m_cursor2Data.m_hidden = true;
			TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, m_cursorSelectName, 1000003);

			m_cursor1Data.m_hidden = false;
			TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, m_cursorSelectName);

			m_menuState = MenuState::MS_Main;
		}
		break;
	}
	}

	TitleUI::cursorLoop(m_cursor1Data, m_sceneMenuCursor1, m_cursorLoopName);
	TitleUI::cursorLoop(m_cursor2Data, m_sceneMenuCursor2, m_cursorLoopName);

	// change D770FC to 2 to goto trial_menu, change it back to 0 before entering stage
}

void TitleUI_TitleCMainCState_SelectMenuEnd(hh::fnd::CStateMachineBase::CStateBase* This)
{
	if (m_spMenuBG)
	{
		m_spMenuBG->SendMessage(m_spMenuBG->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
		m_spMenuBG = nullptr;
	}

	Chao::CSD::CProject::DestroyScene(m_projectMenuBG.Get(), m_sceneMenuBG);
	Chao::CSD::CProject::DestroyScene(m_projectMenuBG.Get(), m_sceneMenuBG2);
	m_projectMenuBG = nullptr;

	if (m_spMenu)
	{
		m_spMenu->SendMessage(m_spMenu->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
		m_spMenu = nullptr;
	}

	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuBars);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuText);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuTextCover);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuTitleBarEffect);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuCursor1);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuCursor2);
	m_projectMenu = nullptr;
}

void TitleUI::applyPatches()
{
	// Fix menu sounds
	WRITE_MEMORY(0x11D219A, uint32_t, 1000002); // Menu oppen
	WRITE_MEMORY(0x11D20EA, uint32_t, 1000003); // Menu close
	WRITE_MEMORY(0x11D1D7A, uint32_t, 1000003); // Cancel
	WRITE_MEMORY(0x11D1E2A, uint32_t, 1000016); // Stage scroll

	// Press Start
	INSTALL_HOOK(TitleUI_TitleCMainCState_WaitStartBegin);
	INSTALL_HOOK(TitleUI_TitleCMainCState_WaitStartAdvance);
	INSTALL_HOOK(TitleUI_TitleCMainCState_WaitStartEnd);

	WRITE_JUMP(0x571FEB, TitleUI_TitleCMainFadeInCompleted);
	WRITE_MEMORY(0x571FF3, uint8_t, 0x85); // je -> jne
	WRITE_JUMP(0x572586, TitleUI_TitleCMainToSelectMenu);
	WRITE_MEMORY(0x57258E, uint8_t, 0x85); // je -> jne

	// Main Menu
	WRITE_MEMORY(0x5727DC, char*, "Menu");
	INSTALL_HOOK(TitleUI_TitleCMainCState_SelectMenuBegin);
	INSTALL_HOOK(TitleUI_TitleCMainCState_SelectMenuAdvance);
	WRITE_MEMORY(0x16E129C, void*, TitleUI_TitleCMainCState_SelectMenuEnd);
}

void TitleUI::cursorSelect(CursorData& data, Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, std::vector<std::string> const& selectNames, uint32_t soundCueID)
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

void TitleUI::cursorLoop(CursorData const& data, Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, std::vector<std::string> const& loopNames)
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

std::vector<std::string> menuStrings =
{
	"NEW GAME",
	"CONTINUE",
	"TRIAL SELECT",
	"OPTION",
	"QUIT GAME"
};

void TitleUI::drawMenu()
{
	if (!m_projectMenu) return;

	static bool visible = true;
	ImGui::Begin("Test", &visible, UIContext::m_hudFlags);
	{
		float posX = 0.1130f;
		float posY = 0.1667f;
		float constexpr yDist = 60.0f / 720.0f;

		for (int i = 0; i < menuStrings.size(); i++)
		{
			ImGui::SetCursorPos(ImVec2((float)*BACKBUFFER_WIDTH * posX, (float)*BACKBUFFER_HEIGHT * posY));
			ImGui::Text(menuStrings[i].c_str());
			posY += yDist;
		}
	}
	ImGui::End();
}
