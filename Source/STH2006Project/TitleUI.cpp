#include "TitleUI.h"
#include "UIContext.h"
#include "Application.h"

FUNCTION_PTR(void, __thiscall, TitleUI_TinyChangeState, 0x773250, void* This, SharedPtrTypeless& spState, const Hedgehog::Base::CSharedString name);

boost::shared_ptr<Sonic::CGameObjectCSD> m_spTitle;
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectTitle;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneTitle;

void TitleUI_PlayMotion(Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, char const* motion, bool loop = false, bool reverse = false)
{
	if (!scene) return;
	scene->SetHideFlag(false);
	scene->SetMotion(motion);
	scene->m_PrevMotionFrame = reverse ? scene->m_MotionEndFrame : 0.0f;
	scene->m_MotionFrame = reverse ? scene->m_MotionEndFrame : 0.0f;
	*(uint32_t*)((uint32_t)scene.Get() + 0xB0) = reverse ? 0 : 1;
	*(uint32_t*)((uint32_t)scene.Get() + 0xB4) = reverse ? 0 : 1; // this stops the reverse animation
	scene->m_MotionDisableFlag = false;
	scene->m_MotionSpeed = reverse ? -1.0f : 1.0f;
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
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuTextBG;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuTextCover;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuTitleBarEffect;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuTitleText;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuTitleText2;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuText[MenuType::MT_COUNT];

TitleUI::CursorData m_cursor1Data;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuCursor1;
TitleUI::CursorData m_cursor2Data;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuCursor2;

boost::shared_ptr<Sonic::CGameObjectCSD> m_spButton;
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectButton;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneButton;

Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectYesNo;
boost::shared_ptr<Sonic::CGameObjectCSD> m_spYesNo;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneYesNoTop;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneYesNoBottom;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneYesNoCursor;

std::map<YesNoTextType, std::string> TitleUI::m_yesNoText;
int TitleUI::m_yesNoCursorPos = 0;
float TitleUI::m_yesNoColorTime = 0.0f;
std::string TitleUI::m_yesNoWindowText;

void TitleUI_PlayButtonMotion(bool twoButton)
{
	if (!m_sceneButton) return;

	float endFrame = 0.0f;
	size_t index = 0;
	if (Common::GetUILanguageType() == LT_Japanese)
	{
		endFrame = twoButton ? 32.0f : 21.0f;
		index = twoButton ? 1 : 3;
	}
	else
	{
		endFrame = twoButton ? 35.0f : 23.0f;
		index = twoButton ? 0 : 2;
	}

	m_sceneButton->GetNode("text")->SetPatternIndex(index);
	m_sceneButton->SetMotion("DefaultAnim");
	m_sceneButton->m_MotionFrame = 0.0f;
	m_sceneButton->m_MotionSpeed = 1.0f;
	m_sceneButton->m_MotionDisableFlag = false;
	m_sceneButton->m_MotionEndFrame = endFrame;
	m_sceneButton->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
	m_sceneButton->Update();
}

HOOK(void, __fastcall, TitleUI_TitleCMainCState_SelectMenuBegin, 0x572750, hh::fnd::CStateMachineBase::CStateBase* This)
{
	Sonic::CGameObject* gameObject = (Sonic::CGameObject*)(This->GetContextBase());
	Sonic::CCsdDatabaseWrapper wrapper(gameObject->m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());

	uint32_t owner = (uint32_t)(This->GetContextBase());
	bool hasSaveFile = *(bool*)(owner + 0x1AC);

	//---------------------------------------------------------------
	auto spCsdProject = wrapper.GetCsdProject("background");
	m_projectMenuBG = spCsdProject->m_rcProject;

	m_sceneMenuBG = m_projectMenuBG->CreateScene("mainmenu_back");
	m_sceneMenuBG2 = m_projectMenuBG->CreateScene("main_menu");

	if (m_projectMenuBG && !m_spMenuBG)
	{
		m_spMenuBG = boost::make_shared<Sonic::CGameObjectCSD>(m_projectMenuBG, 0.5f, "HUD", false);
		Sonic::CGameDocument::GetInstance()->AddGameObject(m_spMenuBG, "main", gameObject);
	}

	//---------------------------------------------------------------
	spCsdProject = wrapper.GetCsdProject("main_menu");
	m_projectMenu = spCsdProject->m_rcProject;

	m_sceneMenuBars = m_projectMenu->CreateScene("main_menu_parts");
	m_sceneMenuBars->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
	m_sceneMenuTextBG = m_projectMenu->CreateScene("text");
	m_sceneMenuTextBG->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
	m_sceneMenuTextCover = m_projectMenu->CreateScene("text_cover");
	m_sceneMenuTextCover->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
	m_sceneMenuTitleBarEffect = m_projectMenu->CreateScene("titlebar_effect");
	m_sceneMenuTitleBarEffect->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
	m_sceneMenuTitleText = m_projectMenu->CreateScene("title");
	m_sceneMenuTitleText->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;

	for (int i = 0; i < MenuType::MT_COUNT; i++)
	{
		m_sceneMenuText[i] = m_projectMenu->CreateScene("episodeselect_select");
		size_t index = 0;
		switch (i)
		{
		case MT_NewGame:
			index = 0;
			break;
		case MT_Continue:
			index = hasSaveFile ? 2 : 3;
			break;
		case MT_TrialSelect:
			index = 4;
			break;
		case MT_Option:
			index = 5;
			break;
		case MT_QuitGame:
			index = 6;
			break;
		}
		m_sceneMenuText[i]->GetNode("episodeselect")->SetPatternIndex(index);
		m_sceneMenuText[i]->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
		m_sceneMenuText[i]->SetPosition(0.0f, i * 59.5f);
	}

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
		m_spMenu = boost::make_shared<Sonic::CGameObjectCSD>(m_projectMenu, 0.5f, "HUD", false);
		Sonic::CGameDocument::GetInstance()->AddGameObject(m_spMenu, "main", gameObject);
	}

	//---------------------------------------------------------------
	spCsdProject = wrapper.GetCsdProject("button_window");
	m_projectButton = spCsdProject->m_rcProject;

	m_sceneButton = m_projectButton->CreateScene("Scene_0000");
	TitleUI_PlayButtonMotion(true);

	if (m_projectButton && !m_spButton)
	{
		m_spButton = boost::make_shared<Sonic::CGameObjectCSD>(m_projectButton, 0.5f, "HUD", false);
		Sonic::CGameDocument::GetInstance()->AddGameObject(m_spButton, "main", gameObject);
	}

	//---------------------------------------------------------------
	spCsdProject = wrapper.GetCsdProject("windowtest");
	m_projectYesNo = spCsdProject->m_rcProject;

	m_sceneYesNoTop = m_projectYesNo->CreateScene("Scene_0000");
	m_sceneYesNoTop->SetPosition(0.0f, -71.0f);
	m_sceneYesNoTop->GetNode("kuro")->SetScale(8.5f, 3.0f);
	m_sceneYesNoTop->GetNode("bou1")->SetScale(8.5f, 0.07f);
	m_sceneYesNoTop->GetNode("bou1")->SetPosition(640.0f, 167.0f);
	m_sceneYesNoTop->GetNode("bou2")->SetScale(8.5f, 0.07f);
	m_sceneYesNoTop->GetNode("bou2")->SetPosition(640.0f, 556.0f);
	m_sceneYesNoTop->GetNode("bou3")->SetScale(0.07f, 3.0f);
	m_sceneYesNoTop->GetNode("bou3")->SetPosition(97.0f, 360.0f);
	m_sceneYesNoTop->GetNode("bou4")->SetScale(0.07f, 3.0f);
	m_sceneYesNoTop->GetNode("bou4")->SetPosition(1190.0f, 360.0f);
	m_sceneYesNoTop->GetNode("kado1")->SetPosition(105.0f, 176.0f);
	m_sceneYesNoTop->GetNode("kado2")->SetPosition(105.0f, 544.0f);
	m_sceneYesNoTop->GetNode("kado3")->SetPosition(1176.0f, 176.0f);
	m_sceneYesNoTop->GetNode("kado4")->SetPosition(1176.0f, 544.0f);
	m_sceneYesNoTop->SetHideFlag(true);

	m_sceneYesNoBottom = m_projectYesNo->CreateScene("Scene_0000");
	m_sceneYesNoBottom->SetPosition(0.0f, 232.0f);
	m_sceneYesNoBottom->GetNode("kuro")->SetScale(8.5f, 1.0f);
	m_sceneYesNoBottom->GetNode("bou1")->SetScale(8.5f, 0.07f);
	m_sceneYesNoBottom->GetNode("bou1")->SetPosition(640.0f, 296.0f);
	m_sceneYesNoBottom->GetNode("bou2")->SetScale(8.5f, 0.07f);
	m_sceneYesNoBottom->GetNode("bou2")->SetPosition(640.0f, 429.0f);
	m_sceneYesNoBottom->GetNode("bou3")->SetScale(0.07f, 1.0f);
	m_sceneYesNoBottom->GetNode("bou3")->SetPosition(97.0f, 360.0f);
	m_sceneYesNoBottom->GetNode("bou4")->SetScale(0.07f, 1.0f);
	m_sceneYesNoBottom->GetNode("bou4")->SetPosition(1190.0f, 360.0f);
	m_sceneYesNoBottom->GetNode("kado1")->SetPosition(105.0f, 305.0f);
	m_sceneYesNoBottom->GetNode("kado2")->SetPosition(105.0f, 415.0f);
	m_sceneYesNoBottom->GetNode("kado3")->SetPosition(1176.0f, 305.0f);
	m_sceneYesNoBottom->GetNode("kado4")->SetPosition(1176.0f, 415.0f);
	m_sceneYesNoBottom->SetHideFlag(true);

	m_sceneYesNoCursor = m_projectYesNo->CreateScene("cursor");
	m_sceneYesNoCursor->SetHideFlag(true);

	if (m_projectYesNo && !m_spYesNo)
	{
		m_spYesNo = boost::make_shared<Sonic::CGameObjectCSD>(m_projectYesNo, 0.5f, "HUD", false);
		Sonic::CGameDocument::GetInstance()->AddGameObject(m_spYesNo, "main", gameObject);
	}

	m_menuState = MenuState::MS_FadeIn;
	originalTitleUI_TitleCMainCState_SelectMenuBegin(This);
}

HOOK(void, __fastcall, TitleUI_TitleCMainCState_SelectMenuAdvance, 0x5728F0, hh::fnd::CStateMachineBase::CStateBase* This)
{
	//originalTitleUI_TitleCMainCState_SelectMenuAdvance(This);

	uint32_t owner = (uint32_t)(This->GetContextBase());
	uint32_t* outState = (uint32_t*)(owner + 0x1BC);
	bool hasSaveFile = *(bool*)(owner + 0x1AC);

	static SharedPtrTypeless spOutState;
	static SharedPtrTypeless soundHandle;

	Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();
	switch (m_menuState)
	{
	case MenuState::MS_FadeIn:
	{
		if (m_sceneMenuBars && m_sceneMenuBars->m_MotionDisableFlag)
		{
			// wait until bar finish animation then show cursor
			m_cursor1Data.m_hidden = false;
			TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1);

			m_menuState = MenuState::MS_Main;
		}
		break;
	}
	case MenuState::MS_Main:
	{
		if (padState->IsTapped(Sonic::EKeyState::eKeyState_LeftStickUp) && m_cursor1Data.m_index > 0)
		{
			m_cursor1Data.m_index--;
			TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_LeftStickDown) && m_cursor1Data.m_index < MenuType::MT_COUNT - 1)
		{
			m_cursor1Data.m_index++;
			TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_A))
		{
			switch (m_cursor1Data.m_index)
			{
			case MenuType::MT_NewGame:
			{
				if (!hasSaveFile)
				{
					Common::PlaySoundStatic(soundHandle, 1000005);

					*outState = 0;
					TitleUI_TinyChangeState(This, spOutState, "Finish");
				}
				else
				{
					// TODO: delete save prompt
				}
				break;
			}
			case MenuType::MT_Continue:
			{
				if (!hasSaveFile)
				{
					Common::PlaySoundStatic(soundHandle, 1000007);
				}
				else
				{
					*outState = 1;
					TitleUI_TinyChangeState(This, spOutState, "Finish");
				}
				break;
			}
			case MenuType::MT_TrialSelect:
			{
				m_cursor1Data.m_hidden = true;
				TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, 1000005);

				m_cursor2Data.m_index = 0;
				m_cursor2Data.m_hidden = false;
				m_sceneMenuCursor2->SetPosition(99.0f, (m_cursor1Data.m_index + 1) * 60.0f);
				TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2);

				m_menuState = MenuState::MS_TrialSelect;
				break;
			}
			case MenuType::MT_QuitGame:
			{
				m_cursor1Data.m_hidden = true;
				TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, 1000005);

				TitleUI::EnableYesNoWindow(true, TitleUI::GetYesNoText(YesNoTextType::YNTT_QuitGame));
				m_menuState = MenuState::MS_QuitYesNo;
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
			TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_LeftStickDown) && m_cursor2Data.m_index < TrialMenuType::TMT_COUNT - 1)
		{
			m_cursor2Data.m_index++;
			TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_B))
		{
			m_cursor2Data.m_hidden = true;
			TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, 1000003);

			m_cursor1Data.m_hidden = false;
			TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1);

			m_menuState = MenuState::MS_Main;
		}
		break;
	}
	case MenuState::MS_QuitYesNo:
	{
		bool returnToMainMenu = false;
		if (padState->IsTapped(Sonic::EKeyState::eKeyState_LeftStickUp) && TitleUI::m_yesNoCursorPos == 1)
		{
			TitleUI::m_yesNoCursorPos = 0;
			TitleUI::SetYesNoCursor(TitleUI::m_yesNoCursorPos);
			Common::PlaySoundStatic(soundHandle, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_LeftStickDown) && TitleUI::m_yesNoCursorPos == 0)
		{
			TitleUI::m_yesNoCursorPos = 1;
			TitleUI::SetYesNoCursor(TitleUI::m_yesNoCursorPos);
			Common::PlaySoundStatic(soundHandle, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_A))
		{
			if (TitleUI::m_yesNoCursorPos == 1)
			{
				returnToMainMenu = true;
			}
			else
			{
				This->m_Time = 0.0f;
				m_menuState = MenuState::MS_QuitYes;
			}
			Common::PlaySoundStatic(soundHandle, 1000005);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_B))
		{
			returnToMainMenu = true;
			Common::PlaySoundStatic(soundHandle, 1000003);
		}

		if (returnToMainMenu)
		{
			TitleUI::EnableYesNoWindow(false);

			m_cursor1Data.m_hidden = false;
			TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1);

			m_menuState = MenuState::MS_Main;
		}

		if (m_sceneYesNoCursor && m_sceneYesNoCursor->m_MotionDisableFlag)
		{
			m_sceneYesNoCursor->SetMotion("cursor_set");
			m_sceneYesNoCursor->SetMotionFrame(0.0f);
			m_sceneYesNoCursor->m_MotionDisableFlag = false;
			m_sceneYesNoCursor->m_MotionSpeed = 1.0f;
			m_sceneYesNoCursor->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_Loop;
			m_sceneYesNoCursor->Update();
		}
		break;
	}
	case MenuState::MS_QuitYes:
	{
		if (This->m_Time > 0.5f)
		{
			// Quit Game
			*(bool*)0x1E5E2E8 = true;
		}
		break;
	}
	}

	TitleUI::cursorLoop(m_cursor1Data, m_sceneMenuCursor1);
	TitleUI::cursorLoop(m_cursor2Data, m_sceneMenuCursor2);

	// change D770FC to 2 to goto trial_menu, change it back to 0 before entering stage
}

void TitleUI_TitleCMainCState_SelectMenuEnd(hh::fnd::CStateMachineBase::CStateBase* This)
{
	//---------------------------------------------------------------
	if (m_spMenuBG)
	{
		m_spMenuBG->SendMessage(m_spMenuBG->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
		m_spMenuBG = nullptr;
	}

	Chao::CSD::CProject::DestroyScene(m_projectMenuBG.Get(), m_sceneMenuBG);
	Chao::CSD::CProject::DestroyScene(m_projectMenuBG.Get(), m_sceneMenuBG2);
	m_projectMenuBG = nullptr;

	//---------------------------------------------------------------
	if (m_spMenu)
	{
		m_spMenu->SendMessage(m_spMenu->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
		m_spMenu = nullptr;
	}

	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuBars);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuTextBG);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuTextCover);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuTitleBarEffect);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuTitleText);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuTitleText2);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuCursor1);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuCursor2);
	for (int i = 0; i < MenuType::MT_COUNT; i++)
	{
		Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuText[i]);
	}
	m_projectMenu = nullptr;

	//---------------------------------------------------------------
	if (m_spButton)
	{
		m_spButton->SendMessage(m_spButton->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
		m_spButton = nullptr;
	}

	Chao::CSD::CProject::DestroyScene(m_projectButton.Get(), m_sceneButton);
	m_projectButton = nullptr;

	//---------------------------------------------------------------
	if (m_spYesNo)
	{
		m_spYesNo->SendMessage(m_spYesNo->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
		m_spYesNo = nullptr;
	}

	Chao::CSD::CProject::DestroyScene(m_projectYesNo.Get(), m_sceneYesNoTop);
	Chao::CSD::CProject::DestroyScene(m_projectYesNo.Get(), m_sceneYesNoBottom);
	Chao::CSD::CProject::DestroyScene(m_projectYesNo.Get(), m_sceneYesNoCursor);
	m_projectYesNo = nullptr;
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

	// Yes No Window
	m_yesNoText[YesNoTextType::YNTT_QuitGame] = "Quit  the  game.\nThe  progress  of  the  game  from  the  last  saved\npoint  will  not  be  saved.  OK?";
	m_yesNoText[YesNoTextType::YNTT_QuitGameJP] = u8"ゲームを終了します\n最後にセーブしたところから\nここまでの進行は保存されませんが\nそれでもよろしいですか？";
	m_yesNoText[YesNoTextType::YNTT_COUNT] = "MISSING TEXT"; // need this for dummy text
}

void TitleUI::cursorSelect(CursorData& data, Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, uint32_t soundCueID)
{
	if (!scene) return;

	static char const* cursorSelectName[] =
	{
		"select_01",
		"select_02",
		"select_03",
		"select_04",
		"select_05",
	};

	TitleUI_PlayMotion(scene, cursorSelectName[data.m_index], false, data.m_hidden);
	if (soundCueID != 0xFFFFFFFF)
	{
		static SharedPtrTypeless soundHandle;
		Common::PlaySoundStatic(soundHandle, soundCueID);
	}
}

void TitleUI::cursorLoop(CursorData const& data, Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene)
{
	if (!scene) return;

	static char const* cursorLoopName[] =
	{
		"loop_01",
		"loop_02",
		"loop_03",
		"loop_04",
		"loop_05",
	};

	if (scene->m_MotionDisableFlag)
	{
		scene->SetMotion(cursorLoopName[data.m_index]);
		if (data.m_hidden)
		{
			scene->SetHideFlag(true);
			return;
		}

		scene->m_MotionDisableFlag = false;
		scene->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_Loop;
	}
}

void TitleUI::EnableYesNoWindow(bool enabled, std::string const& text)
{
	if (!m_sceneYesNoTop || !m_sceneYesNoBottom) return;

	m_sceneYesNoTop->SetHideFlag(!enabled);
	m_sceneYesNoBottom->SetHideFlag(!enabled);
	m_sceneYesNoCursor->SetHideFlag(!enabled);

	if (enabled)
	{
		m_yesNoWindowText = text;
		m_yesNoCursorPos = 0;
		m_yesNoColorTime = 0.0f;

		SetYesNoCursor(m_yesNoCursorPos);
		m_sceneYesNoCursor->SetMotion("cursor_select");
		m_sceneYesNoCursor->SetMotionFrame(0.0f);
		m_sceneYesNoCursor->m_MotionDisableFlag = false;
		m_sceneYesNoCursor->m_MotionSpeed = 1.0f;
		m_sceneYesNoCursor->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
		m_sceneYesNoCursor->Update();
	}
	else
	{
		m_yesNoWindowText = "";
	}
}

void TitleUI::SetYesNoCursor(int pos)
{
	if (m_sceneYesNoCursor)
	{
		m_sceneYesNoCursor->SetPosition(-375.0f, 148.0f + m_yesNoCursorPos * 42.7f);
	}
}

std::string const& TitleUI::GetYesNoText(YesNoTextType type)
{
	if (type < YesNoTextType::YNTT_COUNT - 1)
	{
		if (Common::GetUILanguageType() == LT_Japanese)
		{
			type = (YesNoTextType)(type + 1);
		}
		return m_yesNoText[type];
	}
	else
	{
		return m_yesNoText[YesNoTextType::YNTT_COUNT];
	}
}

void TitleUI::drawMenu()
{
	if (!m_projectMenu) return;

	static bool visible = true;
	ImGui::Begin("Test", &visible, UIContext::m_hudFlags);
	{
		float posX = 0.1130f;
		float posY = 0.1667f;
		float constexpr yDist = 60.0f / 720.0f;

		for (size_t i = 0; i < menuStrings.size(); i++)
		{
			ImGui::SetCursorPos(ImVec2((float)*BACKBUFFER_WIDTH * posX, (float)*BACKBUFFER_HEIGHT * posY));
			ImGui::Text(menuStrings[i].c_str());
			posY += yDist;
		}
	}
	ImGui::End();
}

void TitleUI::drawYesNoWindow()
{
	if (!m_yesNoWindowText.empty())
	{
		static bool visible = true;
		ImGui::Begin("YesNoCaption", &visible, UIContext::m_hudFlags);
		{
			float sizeX = *BACKBUFFER_WIDTH * 1090.0f / 1280.0f;
			float sizeY = *BACKBUFFER_HEIGHT * 382.0f / 720.0f;

			ImVec2 textSize = ImGui::CalcTextSize(m_yesNoWindowText.c_str());

			ImGui::SetWindowFocus();
			ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), m_yesNoWindowText.c_str());
			ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * 0.5f - textSize.x / 2.0f, *BACKBUFFER_HEIGHT * 0.4024f - textSize.y / 2.0f));
		}
		ImGui::End();

		bool isJapanese = Common::GetUILanguageType() == LT_Japanese;
		float blueGreenFactor = (m_yesNoColorTime < 0.5f) ? (1.0f - m_yesNoColorTime * 2.0f) : (m_yesNoColorTime - 0.5f) * 2.0f;
		ImVec4 color(1.0f, blueGreenFactor, blueGreenFactor, 1.0f);

		ImGui::Begin("Yes", &visible, UIContext::m_hudFlags);
		{
			ImGui::SetWindowFocus();
			char const* yes = isJapanese ? u8"はい" : "Yes";
			ImGui::TextColored(m_yesNoCursorPos == 0 ? color : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), yes);
			ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * 0.5f - ImGui::CalcTextSize(yes).x / 2.0f, *BACKBUFFER_HEIGHT * 0.7642f));
		}
		ImGui::End();

		ImGui::Begin("No", &visible, UIContext::m_hudFlags);
		{
			ImGui::SetWindowFocus();
			char const* no = isJapanese ? u8"いいえ" : "No";
			ImGui::TextColored(m_yesNoCursorPos != 0 ? color : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), no);
			ImGui::SetWindowPos(ImVec2(*BACKBUFFER_WIDTH * 0.5f - ImGui::CalcTextSize(no).x / 2.0f, *BACKBUFFER_HEIGHT * 0.8235f));
		}
		ImGui::End();

		m_yesNoColorTime += Application::getDeltaTime();
		if (m_yesNoColorTime >= 1.0f)
		{
			m_yesNoColorTime -= 1.0f;
		}
	}
}
