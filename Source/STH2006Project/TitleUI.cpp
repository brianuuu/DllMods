#include "TitleUI.h"
#include "UIContext.h"
#include "Application.h"
#include "LoadingUI.h"
#include "Configuration.h"

FUNCTION_PTR(void, __thiscall, TitleUI_TinyChangeState, 0x773250, void* This, SharedPtrTypeless& spState, const Hedgehog::Base::CSharedString name);

struct SaveLoadTestStruct
{
	INSERT_PADDING(0x34);
	bool m_saveCompleted;
};
boost::shared_ptr<SaveLoadTestStruct> m_spSave;
FUNCTION_PTR(void, __thiscall, TitleUI_CTitleOptionCStateOutroSaving, 0xD22A70, boost::shared_ptr<SaveLoadTestStruct>& spSave, void* a2);

boost::shared_ptr<Sonic::CGameObjectCSD> m_spTitle;
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectTitle;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneTitle;

boost::shared_ptr<Sonic::CGameObjectCSD> m_spTitleBG;
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectTitleBG;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneTitleBG;

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
MenuState m_menuState = MenuState::MS_FadeIn;
TitleUI::ReturnData m_returnData;

boost::shared_ptr<Sonic::CGameObjectCSD> m_spMenuBG;
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectMenuBG;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuBG;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuBG2;

int m_optionIndex = -1;
int m_optionAudioIndex = -1;
int m_optionOnOffIndex = -1;
boost::shared_ptr<Sonic::CGameObjectCSD> m_spOption;
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectOption;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneOptionBG;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneOptionText[OptionType::OT_COUNT];
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneOptionCursor;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneOptionAudio;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneOptionAudioBar1;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneOptionAudioBar2;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneOptionOnOff;

boost::shared_ptr<Sonic::CGameObjectCSD> m_spMenu;
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectMenu;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuBars;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuTextBG;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuTextCover;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuTitleBarEffect;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuTitleText;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuTitleText2;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuText[MenuType::MT_COUNT];
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneTrialText[TrialMenuType::TMT_COUNT];

TitleUI::CursorData m_cursor1Data;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuCursor1;
TitleUI::CursorData m_cursor2Data;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMenuCursor2;
int m_stageCursorIndex;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneStageCursor;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneStageCursor2;
int m_missionCursorIndex;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMissionCursor;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMissionPlate;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneMissionText;
TitleUI::StageData m_stageData;

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

bool m_allowStoryMode = true;
bool m_displayNonCompletedStage = false;
bool m_allowPlayNonCompletedStage = false;
bool m_drawActTrial = false;
float m_drawActTrialAlpha = -1.0f;
std::vector<size_t> m_actTrialVisibleID;
std::vector<TrialData> m_actTrialData;
bool m_drawTownTrial = false;
float m_drawTownTrialAlpha = -1.0f;
std::vector<size_t> m_townTrialVisibleID;
std::vector<TrialData> m_townTrialData;
bool m_drawModeSelect = false;
float m_drawModeSelectAlpha = -1.0f;

float m_fadeOutTime = 0.0f;

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

LanguageType* TitleUI_GetVoiceLanguageType()
{
	uint32_t voiceOverAddress = Common::GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0x10 });
	return (LanguageType*)voiceOverAddress;
}

LanguageType* TitleUI_GetUILanguageType()
{
	uint32_t uiAddress = Common::GetMultiLevelAddress(0x1E66B34, { 0x8 });
	return (LanguageType*)uiAddress;
}

float* TitleUI_GetMusicVolume()
{
	// range [0.0 - 0.63]
	uint32_t musicVolumeAddress = Common::GetMultiLevelAddress(0x1E77290, { 0x38 });
	return (float*)musicVolumeAddress;
}

float* TitleUI_GetEffectVolume()
{
	// range [0.0 - 0.63]
	uint32_t effectVolumeAddress = Common::GetMultiLevelAddress(0x1E77290, { 0x3C });
	return (float*)effectVolumeAddress;
}

uint32_t* TitleUI_GetOptionFlag()
{
	uint32_t flagsAddress = Common::GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0x18 });
	return (uint32_t*)flagsAddress;
}

void TitleUI_SetMusicVolume(float volume)
{
	uint32_t* pCSoundModuleManager = *(uint32_t**)0x1E77290;
	FUNCTION_PTR(void, __thiscall, SetMusicVolume, 0x75EEF0, void* This, int a2, float volume);

	SetMusicVolume(pCSoundModuleManager, 0, volume * 0.01f * 0.63f);
	*(uint8_t*)((uint32_t)TitleUI_GetVoiceLanguageType() + 1) = (uint8_t)volume;
}

void TitleUI_SetEffectVolume(float volume)
{
	uint32_t* pCSoundModuleManager = *(uint32_t**)0x1E77290;
	FUNCTION_PTR(void, __thiscall, SetMusicVolume, 0x75EEF0, void* This, int a2, float volume);

	SetMusicVolume(pCSoundModuleManager, 1, volume * 0.01f * 0.63f);
	*(uint8_t*)((uint32_t)TitleUI_GetVoiceLanguageType() + 2) = (uint8_t)volume;
}

HOOK(void, __fastcall, TitleUI_TitleCMainCState_InitBegin, 0x571370, hh::fnd::CStateMachineBase::CStateBase* This)
{
	Sonic::CGameObject* gameObject = (Sonic::CGameObject*)(This->GetContextBase());
	Sonic::CCsdDatabaseWrapper wrapper(gameObject->m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());

	//---------------------------------------------------------------
	auto spCsdProject = wrapper.GetCsdProject("background");
	m_projectTitleBG = spCsdProject->m_rcProject;

	m_sceneTitleBG = m_projectTitleBG->CreateScene("fileselect");

	if (m_projectTitleBG && !m_spTitleBG)
	{
		m_spTitleBG = boost::make_shared<Sonic::CGameObjectCSD>(m_projectTitleBG, 0.5f, "HUD", false);
		Sonic::CGameDocument::GetInstance()->AddGameObject(m_spTitleBG, "main", gameObject);
	}

	//---------------------------------------------------------------
	spCsdProject = wrapper.GetCsdProject(Configuration::m_titleLogo == Configuration::TitleLogoType::TLT_Original ? "title_English" : "title_English2");
	m_projectTitle = spCsdProject->m_rcProject;

	m_sceneTitle = m_projectTitle->CreateScene("Scene_Title");
	TitleUI_PlayMotion(m_sceneTitle, "Title_Open");

	if (m_projectTitle && !m_spTitle)
	{
		m_spTitle = boost::make_shared<Sonic::CGameObjectCSD>(m_projectTitle, 0.5f, "HUD", false);
		Sonic::CGameDocument::GetInstance()->AddGameObject(m_spTitle, "main", gameObject);
	}

	originalTitleUI_TitleCMainCState_InitBegin(This);

	// Revert demo menu end state
	WRITE_MEMORY(0xD77102, uint32_t, 0);
	WRITE_MEMORY(0xD7712E, uint32_t, 1);

	m_menuState = MenuState::MS_Idle;

	// Skip to previous menu we left off
	if (m_returnData.m_menuState != MenuState::MS_Idle)
	{
		LoadingUI::startNowLoading(0.0f, true);

		WRITE_NOP(0x571ED0, 5); // don't play title music
		WRITE_NOP(0x572035, 6); // auto press start
		WRITE_NOP(0x56F1EF, 6); // use previous controller ID
		WRITE_MEMORY(0x5720B0, uint8_t, 0xEB); // don't play sfx
	}
}

HOOK(void, __fastcall, TitleUI_TitleCMainCState_WaitStartBegin, 0x571DB0, hh::fnd::CStateMachineBase::CStateBase* This)
{
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
		case TitleState::TS_FadeIn:
			titleState = TitleState::TS_Wait;
			TitleUI_PlayMotion(m_sceneTitle, "Title_Loop", true);
			break;
		case TitleState::TS_Confirm:
			titleState = TitleState::TS_FadeOut;
			TitleUI_PlayMotion(m_sceneTitle, "Title_Close_03");
			break;
		case TitleState::TS_FadeOut:
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

	if (m_spTitleBG)
	{
		m_spTitleBG->SendMessage(m_spTitleBG->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
		m_spTitleBG = nullptr;
	}

	Chao::CSD::CProject::DestroyScene(m_projectTitleBG.Get(), m_sceneTitleBG);
	m_projectTitleBG = nullptr;

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

HOOK(void, __fastcall, TitleUI_TitleCMainCState_SelectMenuBegin, 0x572750, hh::fnd::CStateMachineBase::CStateBase* This)
{
	Sonic::CGameObject* gameObject = (Sonic::CGameObject*)(This->GetContextBase());
	Sonic::CCsdDatabaseWrapper wrapper(gameObject->m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());

	uint32_t owner = (uint32_t)(This->GetContextBase());
	bool hasSaveFile = *(bool*)(owner + 0x1AC);
	bool isJapanese = Common::GetUILanguageType() == LT_Japanese;

	TitleUI::refreshTrialAvailability();

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
	spCsdProject = wrapper.GetCsdProject("option");
	m_projectOption = spCsdProject->m_rcProject;

	m_sceneOptionBG = m_projectOption->CreateScene("back");
	m_sceneOptionBG->SetHideFlag(true);

	for (int i = 0; i < OptionType::OT_COUNT; i++)
	{
		int patternIndex = 0;
		switch (i)
		{
		case OptionType::OT_Audio:
			patternIndex = isJapanese ? 1 : 0;
			break;
		case OptionType::OT_UI:
			patternIndex = isJapanese ? 3 : 2;
			break;
		case OptionType::OT_VO:
			patternIndex = isJapanese ? 5 : 4;
			break;
		case OptionType::OT_Dialog:
			patternIndex = isJapanese ? 7 : 6;
			break;
		case OptionType::OT_Subtitle:
			patternIndex = isJapanese ? 9 : 8;
			break;
		}

		m_sceneOptionText[i] = m_projectOption->CreateScene("option");
		m_sceneOptionText[i]->SetPosition(0.0f, i * 47.0f);
		m_sceneOptionText[i]->GetNode("text1")->SetPatternIndex(patternIndex);
		m_sceneOptionText[i]->SetHideFlag(true);
	}

	m_sceneOptionCursor = m_projectOption->CreateScene("optioncursor");
	m_sceneOptionCursor->SetHideFlag(true);
	m_sceneOptionAudio = m_projectOption->CreateScene("audio");
	m_sceneOptionAudio->SetHideFlag(true);
	m_sceneOptionAudioBar1 = m_projectOption->CreateScene("tumami01");
	m_sceneOptionAudioBar1->m_MotionSpeed = 0.0f;
	m_sceneOptionAudioBar1->SetHideFlag(true);
	m_sceneOptionAudioBar2 = m_projectOption->CreateScene("tumami02");
	m_sceneOptionAudioBar2->m_MotionSpeed = 0.0f;
	m_sceneOptionAudioBar2->SetHideFlag(true);
	m_sceneOptionOnOff = m_projectOption->CreateScene("jimaku");
	m_sceneOptionOnOff->SetHideFlag(true);

	if (m_projectOption && !m_spOption)
	{
		m_spOption = boost::make_shared<Sonic::CGameObjectCSD>(m_projectOption, 0.5f, "HUD", false);
		Sonic::CGameDocument::GetInstance()->AddGameObject(m_spOption, "main", gameObject);
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
	m_sceneMenuTitleText2 = m_projectMenu->CreateScene("title");
	m_sceneMenuTitleText2->SetHideFlag(true);

	for (int i = 0; i < MenuType::MT_COUNT; i++)
	{
		m_sceneMenuText[i] = m_projectMenu->CreateScene("episodeselect_select");
		size_t index = 0;
		switch (i)
		{
		case MT_NewGame:
			index = m_allowStoryMode ? 0 : 1;
			break;
		case MT_Continue:
			index = (hasSaveFile && m_allowStoryMode) ? 2 : 3;
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

	for (int i = 0; i < TrialMenuType::TMT_COUNT; i++)
	{
		m_sceneTrialText[i] = m_projectMenu->CreateScene("episodeselect_select");
		size_t index = 0;
		switch (i)
		{
		case TMT_Act:
			index = m_actTrialVisibleID.empty() ? 8 : 7;
			break;
		case TMT_Town:
			index = m_townTrialVisibleID.empty() ? 10 : 9;
			break;
		}
		m_sceneTrialText[i]->GetNode("episodeselect")->SetPatternIndex(index);
		m_sceneTrialText[i]->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
		m_sceneTrialText[i]->SetPosition(99.0f, (i + 3) * 60.0f);
		m_sceneTrialText[i]->SetHideFlag(true);
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

	m_sceneStageCursor = m_projectMenu->CreateScene("stage_cursor");
	m_sceneStageCursor->SetHideFlag(true);
	m_sceneStageCursor2 = m_projectMenu->CreateScene("stage_cursor2");
	m_sceneStageCursor2->SetHideFlag(true);
	m_sceneMissionCursor = m_projectMenu->CreateScene("mission_cursor");
	m_sceneMissionCursor->SetHideFlag(true);
	m_sceneMissionPlate = m_projectMenu->CreateScene("mission_plate");
	m_sceneMissionPlate->SetHideFlag(true);
	m_sceneMissionText = m_projectMenu->CreateScene("mission_text");
	m_sceneMissionText->SetHideFlag(true);

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

	// Skip to previous menu we left off
	if (m_returnData.m_menuState != MenuState::MS_Idle)
	{
		m_menuState = m_returnData.m_menuState;

		// Jump to the correct state
		switch (m_returnData.m_menuState)
		{
		case MenuState::MS_ModeSelect:
		case MenuState::MS_TownTrial:
		{
			for (int i = 0; i < MenuType::MT_COUNT; i++)
			{
				m_sceneMenuText[i]->SetHideFlag(true);
			}

			m_cursor1Data.m_index = m_returnData.m_cursor1Index;
			m_sceneMenuCursor2->SetPosition(99.0f, (m_cursor1Data.m_index + 1) * 60.0f);
			m_stageCursorIndex = m_returnData.m_stageCursorIndex;
			m_missionCursorIndex = m_returnData.m_missionCursorIndex;

			TitleUI::menuTitleSecondary(true, m_returnData.m_menuState == MenuState::MS_ModeSelect ? 8 : 10);
			m_sceneMenuTitleText->SetHideFlag(true);

			// populate itself so the data can refresh
			TitleUI::populateStageData(m_stageData.m_stage, m_stageData.m_stageID);

			TitleUI::cursorMission(m_missionCursorIndex);
			TitleUI_PlayMotion(m_sceneMissionPlate, "DefaultAnim");
			m_sceneMissionText->SetHideFlag(false);
			break;
		}
		}

		m_returnData.m_menuState = MenuState::MS_Idle;
		LoadingUI::stopNowLoading(0.0f, true);

		// Revert skipping code
		WRITE_MEMORY(0x571ED0, uint8_t, 0xE8, 0x4B, 0xE7, 0xFF, 0xFF);
		WRITE_MEMORY(0x572035, uint8_t, 0x0F, 0x8C, 0x9F, 0x00, 0x00, 0x00);
		WRITE_MEMORY(0x56F1EF, uint8_t, 0x56, 0xE8, 0x3B, 0xF3, 0xAE, 0x00);
		WRITE_MEMORY(0x5720B0, uint8_t, 0x75);
	}

	originalTitleUI_TitleCMainCState_SelectMenuBegin(This);
}

bool scrollHold = false;
float scrollDelay = 0.0f;
HOOK(void, __fastcall, TitleUI_TitleCMainCState_SelectMenuAdvance, 0x5728F0, hh::fnd::CStateMachineBase::CStateBase* This)
{
	//originalTitleUI_TitleCMainCState_SelectMenuAdvance(This);

	uint32_t owner = (uint32_t)(This->GetContextBase());
	uint32_t* outState = (uint32_t*)(owner + 0x1BC);
	bool hasSaveFile = *(bool*)(owner + 0x1AC);
	bool isJapanese = Common::GetUILanguageType() == LT_Japanese;

	static SharedPtrTypeless spOutState;
	static SharedPtrTypeless soundHandle;

	static float timePrev = 0.0f;
	float constexpr firstScrollDelay = 0.3f;
	float constexpr holdScrollDelay = 0.15f;
	Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();

	bool scrollUp = false;
	bool scrollDown = false;
	if (padState->IsDown(Sonic::EKeyState::eKeyState_LeftStickUp) || padState->IsDown(Sonic::EKeyState::eKeyState_LeftStickDown))
	{
		if (scrollDelay <= 0.0f)
		{
			scrollUp = padState->IsDown(Sonic::EKeyState::eKeyState_LeftStickUp);
			scrollDown = padState->IsDown(Sonic::EKeyState::eKeyState_LeftStickDown);
			scrollDelay = scrollHold ? holdScrollDelay : firstScrollDelay;
			scrollHold = true;
		}
		scrollDelay -= (This->m_Time - timePrev);
	}
	else
	{
		scrollHold = false;
		scrollDelay = 0.0f;
	}

	switch (m_menuState)
	{
	case MenuState::MS_FadeIn:
	{
		if (m_sceneMenuBars && m_sceneMenuBars->m_MotionDisableFlag)
		{
			// Go to apporiate index
			m_cursor1Data.m_index = hasSaveFile ? MenuType::MT_Continue : MenuType::MT_NewGame;
			m_cursor1Data.m_index = m_allowStoryMode ? m_cursor1Data.m_index : MenuType::MT_TrialSelect;

			// wait until bar finish animation then show cursor
			m_cursor1Data.m_hidden = false;
			TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1);

			m_menuState = MenuState::MS_Main;
		}
		break;
	}
	case MenuState::MS_Main:
	{
		if (scrollUp)
		{
			m_cursor1Data.m_index--;
			if (m_cursor1Data.m_index < 0)
			{
				m_cursor1Data.m_index = MenuType::MT_COUNT - 1;
			}
			TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, 1000004);
		}
		else if (scrollDown)
		{
			m_cursor1Data.m_index++;
			if (m_cursor1Data.m_index >= MenuType::MT_COUNT)
			{
				m_cursor1Data.m_index = 0;
			}
			TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_A))
		{
			bool hideCursor = true;
			switch (m_cursor1Data.m_index)
			{
			case MenuType::MT_NewGame:
			{
				if (!m_allowStoryMode)
				{
					hideCursor = false;
					Common::PlaySoundStatic(soundHandle, 1000007);
				}
				else if (!hasSaveFile)
				{
					hideCursor = false;
					Common::PlaySoundStatic(soundHandle, 1000005);

					*outState = 0;
					m_fadeOutTime = 0.0f;
					m_menuState = MenuState::MS_FadeOut;

					m_returnData.m_menuState = MenuState::MS_FadeIn;
				}
				else
				{
					TitleUI::EnableYesNoWindow(true, false, TitleUI::GetYesNoText(YesNoTextType::YNTT_NewGame));
					m_menuState = MenuState::MS_DeleteSaveYesNo;
				}
				break;
			}
			case MenuType::MT_Continue:
			{
				hideCursor = false;
				if (!hasSaveFile || !m_allowStoryMode)
				{
					Common::PlaySoundStatic(soundHandle, 1000007);
				}
				else
				{
					Common::PlaySoundStatic(soundHandle, 1000005);

					*outState = 1;
					m_fadeOutTime = 0.0f;
					m_menuState = MenuState::MS_FadeOut;

					m_returnData.m_menuState = MenuState::MS_FadeIn;
				}
				break;
			}
			case MenuType::MT_TrialSelect:
			{
				m_cursor2Data.m_index = 0;
				m_cursor2Data.m_hidden = false;
				m_sceneMenuCursor2->SetPosition(99.0f, (m_cursor1Data.m_index + 1) * 60.0f);
				TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2);

				TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_Option], true);
				TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_QuitGame], true);
				TitleUI::menuTextLeft(m_sceneTrialText[TrialMenuType::TMT_Act], false);
				TitleUI::menuTextLeft(m_sceneTrialText[TrialMenuType::TMT_Town], false);

				m_menuState = MenuState::MS_TrialSelect;
				break;
			}
			case MenuType::MT_Option:
			{
				TitleUI::menuTitleSecondary(true, 6);
				for (size_t i = 0; i < MenuType::MT_COUNT; i++)
				{
					TitleUI::menuTextLeft(m_sceneMenuText[i], true);
				}

				TitleUI::optionEnable(true);
				m_menuState = MenuState::MS_Option;
				break;
			}
			case MenuType::MT_QuitGame:
			{
				TitleUI::EnableYesNoWindow(true, false, TitleUI::GetYesNoText(YesNoTextType::YNTT_QuitGame));
				m_menuState = MenuState::MS_QuitYesNo;
				break;
			}
			}

			if (hideCursor)
			{
				m_cursor1Data.m_hidden = true;
				TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, 1000005);
			}
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_B))
		{
			m_cursor1Data.m_hidden = true;
			TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, 1000003);

			TitleUI::EnableYesNoWindow(true, true, TitleUI::GetYesNoText(YesNoTextType::YNTT_ReturnTitle));
			m_menuState = MenuState::MS_ReturnTitleYesNo;
		}
		break;
	}
	case MenuState::MS_TrialSelect:
	{
		if (scrollUp)
		{
			m_cursor2Data.m_index--;
			if (m_cursor2Data.m_index < 0)
			{
				m_cursor2Data.m_index = TrialMenuType::TMT_COUNT - 1;
			}
			TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, 1000004);
		}
		else if (scrollDown)
		{
			m_cursor2Data.m_index++;
			if (m_cursor2Data.m_index >= TrialMenuType::TMT_COUNT)
			{
				m_cursor2Data.m_index = 0;
			}
			TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_A))
		{
			switch (m_cursor2Data.m_index)
			{
			case TrialMenuType::TMT_Act:
			{
				if (m_actTrialVisibleID.empty())
				{
					Common::PlaySoundStatic(soundHandle, 1000007);
				}
				else
				{
					TitleUI::menuTitleSecondary(true, 8);

					m_cursor2Data.m_hidden = true;
					TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, 1000005);
					TitleUI::cursorStageSelect(0, false);

					TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_NewGame], true);
					TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_Continue], true);
					TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_TrialSelect], true);
					TitleUI::menuTextRight(m_sceneTrialText[TrialMenuType::TMT_Act], true);
					TitleUI::menuTextLeft(m_sceneTrialText[TrialMenuType::TMT_Town], true);

					m_menuState = MenuState::MS_ActTrial;
				}
				break;
			}
			case TrialMenuType::TMT_Town:
			{
				if (m_townTrialVisibleID.empty())
				{
					Common::PlaySoundStatic(soundHandle, 1000007);
				}
				else
				{
					TitleUI::menuTitleSecondary(true, 10);

					m_cursor2Data.m_hidden = true;
					TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, 1000005);
					TitleUI::cursorStageSelect(0, true);

					size_t id = m_townTrialVisibleID[m_missionCursorIndex];
					TrialData const& data = m_townTrialData[id];
					TitleUI::populateStageData(data.m_stage, data.m_stageID);

					TitleUI_PlayMotion(m_sceneMissionPlate, "DefaultAnim");
					m_sceneMissionText->SetHideFlag(false);

					TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_NewGame], true);
					TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_Continue], true);
					TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_TrialSelect], true);
					TitleUI::menuTextLeft(m_sceneTrialText[TrialMenuType::TMT_Act], true);
					TitleUI::menuTextRight(m_sceneTrialText[TrialMenuType::TMT_Town], true);

					m_menuState = MenuState::MS_TownTrial;
				}
				break;
			}
			}
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_B))
		{
			m_cursor2Data.m_hidden = true;
			TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, 1000003);

			m_cursor1Data.m_hidden = false;
			TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1);

			TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_Option], false);
			TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_QuitGame], false);
			TitleUI::menuTextLeft(m_sceneTrialText[TrialMenuType::TMT_Act], true);
			TitleUI::menuTextLeft(m_sceneTrialText[TrialMenuType::TMT_Town], true);

			m_menuState = MenuState::MS_Main;
		}
		break;
	}
	case MenuState::MS_ActTrial:
	{
		if (scrollUp)
		{
			TitleUI::cursorStageSelect(m_stageCursorIndex - 1, false);
			Common::PlaySoundStatic(soundHandle, 1000004);
		}
		else if (scrollDown)
		{
			TitleUI::cursorStageSelect(m_stageCursorIndex + 1, false);
			Common::PlaySoundStatic(soundHandle, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_A))
		{
			size_t id = m_actTrialVisibleID[m_stageCursorIndex];
			TrialData const& data = m_actTrialData[id];
			if (!data.m_playable)
			{
				Common::PlaySoundStatic(soundHandle, 1000007);
			}
			else
			{
				Common::PlaySoundStatic(soundHandle, 1000005);

				m_sceneStageCursor->SetHideFlag(true);
				m_sceneStageCursor2->SetHideFlag(true);

				m_missionCursorIndex = 0;
				TitleUI::cursorMission(m_missionCursorIndex);

				TitleUI_PlayMotion(m_sceneMissionPlate, "DefaultAnim");
				m_sceneMissionText->SetHideFlag(false);
				TitleUI::populateStageData(data.m_stage, data.m_stageID);

				m_menuState = MenuState::MS_ModeSelect;
			}
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_B))
		{
			TitleUI::menuTitleSecondary(false);

			m_cursor2Data.m_hidden = false;
			TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, 1000003);

			m_sceneStageCursor->SetHideFlag(true);
			m_sceneStageCursor2->SetHideFlag(true);

			TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_NewGame], false);
			TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_Continue], false);
			TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_TrialSelect], false);
			TitleUI::menuTextRight(m_sceneTrialText[TrialMenuType::TMT_Act], false);
			TitleUI::menuTextLeft(m_sceneTrialText[TrialMenuType::TMT_Town], false);

			m_menuState = MenuState::MS_TrialSelect;
		}
		break;
	}
	case MenuState::MS_ModeSelect:
	{
		size_t id = m_actTrialVisibleID[m_stageCursorIndex];
		TrialData const& data = m_actTrialData[id];
		if (m_stageData.m_isBoss && (scrollUp || scrollDown))
		{
			m_missionCursorIndex = (m_missionCursorIndex == 0) ? 1 : 0;
			TitleUI::cursorMission(m_missionCursorIndex);
			if (m_missionCursorIndex == 0)
			{
				TitleUI::populateStageData(data.m_stage, data.m_stageID);
			}
			else
			{
				TitleUI::populateStageData(data.m_stage | SMT_BossHard, data.m_stageID + "001");
			}
			
			Common::PlaySoundStatic(soundHandle, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_A))
		{
			if (m_missionCursorIndex == 1 && !data.m_hardModePlayable)
			{
				Common::PlaySoundStatic(soundHandle, 1000007);
			}
			else
			{
				Common::PlaySoundStatic(soundHandle, 1000005);

				// Force GetEndState() to be 2 to launch demo menu
				WRITE_MEMORY(0xD77102, uint32_t, 2);
				WRITE_MEMORY(0xD7712E, uint32_t, 2);

				*outState = 4;
				m_fadeOutTime = 0.0f;
				m_menuState = MenuState::MS_FadeOutStage;

				m_returnData.m_menuState = MenuState::MS_ModeSelect;
				m_returnData.m_cursor1Index = MenuType::MT_TrialSelect;
				m_returnData.m_stageCursorIndex = m_stageCursorIndex;
				m_returnData.m_missionCursorIndex = m_missionCursorIndex;
			}
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_B))
		{
			Common::PlaySoundStatic(soundHandle, 1000003);

			m_sceneMissionCursor->SetHideFlag(true);
			TitleUI::cursorStageSelect(m_stageCursorIndex, false);

			TitleUI_PlayMotion(m_sceneMissionPlate, "DefaultAnim", false, true);
			m_sceneMissionText->SetHideFlag(true);

			m_menuState = MenuState::MS_ActTrial;
		}
		break;
	}
	case MenuState::MS_TownTrial:
	{
		if (scrollUp)
		{
			TitleUI::cursorStageSelect(m_missionCursorIndex - 1, true);

			size_t id = m_townTrialVisibleID[m_missionCursorIndex];
			TrialData const& data = m_townTrialData[id];
			TitleUI::populateStageData(data.m_stage, data.m_stageID);

			Common::PlaySoundStatic(soundHandle, 1000004);
		}
		else if (scrollDown)
		{
			TitleUI::cursorStageSelect(m_missionCursorIndex + 1, true);

			size_t id = m_townTrialVisibleID[m_missionCursorIndex];
			TrialData const& data = m_townTrialData[id];
			TitleUI::populateStageData(data.m_stage, data.m_stageID);

			Common::PlaySoundStatic(soundHandle, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_A))
		{
			size_t id = m_townTrialVisibleID[m_missionCursorIndex];
			TrialData const& data = m_townTrialData[id];
			if (!data.m_playable)
			{
				Common::PlaySoundStatic(soundHandle, 1000007);
			}
			else
			{
				Common::PlaySoundStatic(soundHandle, 1000005);

				// Force GetEndState() to be 2 to launch demo menu
				WRITE_MEMORY(0xD77102, uint32_t, 2);
				WRITE_MEMORY(0xD7712E, uint32_t, 2);

				*outState = 4;
				m_fadeOutTime = 0.0f;
				m_menuState = MenuState::MS_FadeOutMission;

				m_returnData.m_menuState = MenuState::MS_TownTrial;
				m_returnData.m_cursor1Index = MenuType::MT_TrialSelect;
				m_returnData.m_missionCursorIndex = m_missionCursorIndex;
			}
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_B))
		{
			TitleUI::menuTitleSecondary(false);

			m_cursor2Data.m_hidden = false;
			TitleUI::cursorSelect(m_cursor2Data, m_sceneMenuCursor2, 1000003);

			m_sceneMissionCursor->SetHideFlag(true);
			m_sceneStageCursor2->SetHideFlag(true);

			TitleUI_PlayMotion(m_sceneMissionPlate, "DefaultAnim", false, true);
			m_sceneMissionText->SetHideFlag(true);

			TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_NewGame], false);
			TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_Continue], false);
			TitleUI::menuTextLeft(m_sceneMenuText[MenuType::MT_TrialSelect], false);
			TitleUI::menuTextLeft(m_sceneTrialText[TrialMenuType::TMT_Act], false);
			TitleUI::menuTextRight(m_sceneTrialText[TrialMenuType::TMT_Town], false);

			m_menuState = MenuState::MS_TrialSelect;
		}
		break;
	}
	case MenuState::MS_Option:
	{
		if (scrollUp)
		{
			if (m_optionIndex == 0)
			{
				TitleUI::optionSetIndex(OptionType::OT_COUNT - 1);
			}
			else
			{
				TitleUI::optionSetIndex(m_optionIndex - 1);
			}
			Common::PlaySoundStatic(soundHandle, 1000004);
		}
		else if (scrollDown)
		{
			if (m_optionIndex == OptionType::OT_COUNT - 1)
			{
				TitleUI::optionSetIndex(0);
			}
			else
			{
				TitleUI::optionSetIndex(m_optionIndex + 1);
			}
			Common::PlaySoundStatic(soundHandle, 1000004);
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_A))
		{
			Common::PlaySoundStatic(soundHandle, 1000005);
			TitleUI_PlayButtonMotion(false);

			switch (m_optionIndex)
			{
			case OptionType::OT_Audio:
				TitleUI::optionAudioSetIndex(0);
				m_menuState = MenuState::MS_OptionAudio;
				break;
			case OptionType::OT_UI:
				TitleUI::optionOnOffSetIndex(*TitleUI_GetUILanguageType() == LT_Japanese ? 1 : 0);
				m_menuState = MenuState::MS_OptionUI;
				break;
			case OptionType::OT_VO:
				TitleUI::optionOnOffSetIndex(*TitleUI_GetVoiceLanguageType() == LT_Japanese ? 1 : 0);
				m_menuState = MenuState::MS_OptionVO;
				break;
			case OptionType::OT_Dialog:
				TitleUI::optionOnOffSetIndex((*TitleUI_GetOptionFlag() & 0x2) == 0 ? 1 : 0);
				m_menuState = MenuState::MS_OptionDialog;
				break;
			case OptionType::OT_Subtitle:
				TitleUI::optionOnOffSetIndex((*TitleUI_GetOptionFlag() & 0x10) == 0 ? 1 : 0);
				m_menuState = MenuState::MS_OptionSubtitle;
				break;
			}
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_B))
		{
			m_cursor1Data.m_hidden = false;
			TitleUI::cursorSelect(m_cursor1Data, m_sceneMenuCursor1, 1000003);

			TitleUI::menuTitleSecondary(false);
			for (size_t i = 0; i < MenuType::MT_COUNT; i++)
			{
				TitleUI::menuTextLeft(m_sceneMenuText[i], false);
			}

			TitleUI::optionEnable(false);

			// Trigger saving
			TitleUI_CTitleOptionCStateOutroSaving(m_spSave, nullptr);
			m_menuState = MenuState::MS_OptionSaving;

		}
		break;
	}
	case MenuState::MS_OptionSaving:
	{
		if (!m_spSave || m_spSave->m_saveCompleted)
		{
			m_spSave = nullptr;
			m_menuState = MenuState::MS_Main;
		}
		break;
	}
	case MenuState::MS_OptionAudio:
	{
		float volumeChamge = Application::getDeltaTime() * 50.0f;
		if (scrollUp || scrollDown)
		{
			TitleUI::optionAudioSetIndex(m_optionAudioIndex == 0 ? 1 : 0);
			Common::PlaySoundStatic(soundHandle, 1000004);
		}
		else if (padState->IsDown(Sonic::EKeyState::eKeyState_LeftStickLeft))
		{
			if (m_optionAudioIndex == 0)
			{
				m_sceneOptionAudioBar1->m_MotionFrame = max(0.0f, m_sceneOptionAudioBar1->m_MotionFrame - volumeChamge);
				TitleUI_SetMusicVolume(m_sceneOptionAudioBar1->m_MotionFrame);
			}
			else
			{
				m_sceneOptionAudioBar2->m_MotionFrame = max(0.0f, m_sceneOptionAudioBar2->m_MotionFrame - volumeChamge);
				TitleUI_SetEffectVolume(m_sceneOptionAudioBar2->m_MotionFrame);
			}
		}
		else if (padState->IsDown(Sonic::EKeyState::eKeyState_LeftStickRight))
		{
			if (m_optionAudioIndex == 0)
			{
				m_sceneOptionAudioBar1->m_MotionFrame = min(100.0f, m_sceneOptionAudioBar1->m_MotionFrame + volumeChamge);
				TitleUI_SetMusicVolume(m_sceneOptionAudioBar1->m_MotionFrame);
			}
			else
			{
				m_sceneOptionAudioBar2->m_MotionFrame = min(100.0f, m_sceneOptionAudioBar2->m_MotionFrame + volumeChamge);
				TitleUI_SetEffectVolume(m_sceneOptionAudioBar2->m_MotionFrame);
			}
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_B))
		{
			Common::PlaySoundStatic(soundHandle, 1000003);
			TitleUI_PlayButtonMotion(true);

			TitleUI::optionSetIndex(OptionType::OT_Audio);
			TitleUI::optionAudioSetIndex(-1);

			m_menuState = MenuState::MS_Option;
		}
		break;
	}
	case MenuState::MS_OptionUI:
	case MenuState::MS_OptionVO:
	{
		if (scrollUp || scrollDown)
		{
			TitleUI::optionOnOffSetIndex(m_optionOnOffIndex == 0 ? 1 : 0);
			Common::PlaySoundStatic(soundHandle, 1000004);

			if (m_menuState == MenuState::MS_OptionUI)
			{
				*TitleUI_GetUILanguageType() = m_optionOnOffIndex == 0 ? LT_English : LT_Japanese;

				// Refresh language
				TitleUI_PlayButtonMotion(false);
				for (int i = 0; i < OptionType::OT_COUNT; i++)
				{
					int patternIndex = 0;
					switch (i)
					{
					case OptionType::OT_Audio:
						patternIndex = m_optionOnOffIndex == 0 ? 1 : 0;
						break;
					case OptionType::OT_UI:
						patternIndex = m_optionOnOffIndex == 0 ? 3 : 2;
						break;
					case OptionType::OT_VO:
						patternIndex = m_optionOnOffIndex == 0 ? 5 : 4;
						break;
					case OptionType::OT_Dialog:
						patternIndex = m_optionOnOffIndex == 0 ? 7 : 6;
						break;
					case OptionType::OT_Subtitle:
						patternIndex = m_optionOnOffIndex == 0 ? 9 : 8;
						break;
					}
					m_sceneOptionText[i]->GetNode("text1")->SetPatternIndex(patternIndex);
				}
			}
			else
			{
				*TitleUI_GetVoiceLanguageType() = m_optionOnOffIndex == 0 ? LT_English : LT_Japanese;
			}
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_B))
		{
			Common::PlaySoundStatic(soundHandle, 1000003);
			TitleUI_PlayButtonMotion(true);

			TitleUI::optionSetIndex(m_menuState == MenuState::MS_OptionUI ? OptionType::OT_UI : OptionType::OT_VO);
			m_menuState = MenuState::MS_Option;
		}
		break;
	}
	case MenuState::MS_OptionDialog:
	case MenuState::MS_OptionSubtitle:
	{
		if (scrollUp || scrollDown)
		{
			TitleUI::optionOnOffSetIndex(m_optionOnOffIndex == 0 ? 1 : 0);
			Common::PlaySoundStatic(soundHandle, 1000004);

			if (m_menuState == MenuState::MS_OptionDialog)
			{
				if (m_optionOnOffIndex == 0)
				{
					*TitleUI_GetOptionFlag() |= 0x2;
				}
				else
				{
					*TitleUI_GetOptionFlag() &= 0xFFFFFFFD;
				}
			}
			else
			{
				if (m_optionOnOffIndex == 0)
				{
					*TitleUI_GetOptionFlag() |= 0x10;
				}
				else
				{
					*TitleUI_GetOptionFlag() &= 0xFFFFFFEF;
				}
			}
		}
		else if (padState->IsTapped(Sonic::EKeyState::eKeyState_B))
		{
			Common::PlaySoundStatic(soundHandle, 1000003);
			TitleUI_PlayButtonMotion(true);

			TitleUI::optionSetIndex(m_menuState == MenuState::MS_OptionDialog ? OptionType::OT_Dialog : OptionType::OT_Subtitle);
			m_menuState = MenuState::MS_Option;
		}
		break;
	}
	case MenuState::MS_DeleteSaveYesNo:
	case MenuState::MS_ReturnTitleYesNo:
	case MenuState::MS_QuitYesNo:
	{
		bool returnToMainMenu = false;
		if (scrollUp && TitleUI::m_yesNoCursorPos == 1)
		{
			TitleUI::m_yesNoCursorPos = 0;
			TitleUI::SetYesNoCursor(TitleUI::m_yesNoCursorPos);
			Common::PlaySoundStatic(soundHandle, 1000004);
		}
		else if (scrollDown && TitleUI::m_yesNoCursorPos == 0)
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
				if (m_menuState == MenuState::MS_DeleteSaveYesNo)
				{
					TitleUI::EnableYesNoWindow(false);

					// fade out then use ExecSubMenu to delete save
					*outState = 0;
					m_fadeOutTime = 0.0f;
					m_menuState = MenuState::MS_FadeOut;
				}
				else if (m_menuState == MenuState::MS_QuitYesNo)
				{
					// wait a bit then close game
					This->m_Time = 0.0f;
					m_menuState = MenuState::MS_QuitYes;
				}
				else
				{
					TitleUI::EnableYesNoWindow(false);

					// fade out then return to title screen
					m_fadeOutTime = 0.0f;
					m_menuState = MenuState::MS_FadeOutTitle;
				}
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
	case MenuState::MS_FadeOut:
	case MenuState::MS_FadeOutTitle:
	case MenuState::MS_FadeOutStage:
	case MenuState::MS_FadeOutMission:
	{
		if (m_fadeOutTime >= 1.2f)
		{
			TitleUI_TinyChangeState(This, spOutState, m_menuState == MenuState::MS_FadeOutTitle ? "Init" : "ExecSubMenu");
		}
		break;
	}
	}

	TitleUI::cursorLoop(m_cursor1Data, m_sceneMenuCursor1);
	TitleUI::cursorLoop(m_cursor2Data, m_sceneMenuCursor2);

	timePrev = This->m_Time;
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
	if (m_spOption)
	{
		m_spOption->SendMessage(m_spOption->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
		m_spOption = nullptr;
	}

	Chao::CSD::CProject::DestroyScene(m_projectOption.Get(), m_sceneOptionBG);
	for (int i = 0; i < OptionType::OT_COUNT; i++)
	{
		Chao::CSD::CProject::DestroyScene(m_projectOption.Get(), m_sceneOptionText[i]);
	}
	Chao::CSD::CProject::DestroyScene(m_projectOption.Get(), m_sceneOptionCursor);
	Chao::CSD::CProject::DestroyScene(m_projectOption.Get(), m_sceneOptionAudio);
	Chao::CSD::CProject::DestroyScene(m_projectOption.Get(), m_sceneOptionAudioBar1);
	Chao::CSD::CProject::DestroyScene(m_projectOption.Get(), m_sceneOptionAudioBar2);
	Chao::CSD::CProject::DestroyScene(m_projectOption.Get(), m_sceneOptionOnOff);
	m_projectOption = nullptr;

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
	for (int i = 0; i < MenuType::MT_COUNT; i++)
	{
		Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuText[i]);
	}
	for (int i = 0; i < TrialMenuType::TMT_COUNT; i++)
	{
		Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneTrialText[i]);
	}
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuCursor1);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMenuCursor2);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneStageCursor);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneStageCursor2);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMissionCursor);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMissionPlate);
	Chao::CSD::CProject::DestroyScene(m_projectMenu.Get(), m_sceneMissionText);
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

HOOK(void, __fastcall, TitleUI_CDemoMenuObjectAdvance, 0x576470, uint32_t* This, void* Edx, int a2)
{
	// override stageID
	WRITE_STRING(0x15BB8A8, m_stageData.m_stageID.c_str())

	// set mission ID
	*(uint32_t*)Common::GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x80, 0x28 }) = (m_stageData.m_stage >> 8);

	originalTitleUI_CDemoMenuObjectAdvance(This, Edx, a2);
}

HOOK(void, __fastcall, TitleUI_CGameplayFlowStage_CStateGoalBegin, 0xCFD550, void* This)
{
	// Force saving at result screen
	TitleUI_CTitleOptionCStateOutroSaving(m_spSave, nullptr);

	originalTitleUI_CGameplayFlowStage_CStateGoalBegin(This);
}

void TitleUI::applyPatches()
{
	populateTrialData();

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
	INSTALL_HOOK(TitleUI_TitleCMainCState_InitBegin);
	INSTALL_HOOK(TitleUI_TitleCMainCState_SelectMenuBegin);
	INSTALL_HOOK(TitleUI_TitleCMainCState_SelectMenuAdvance);
	WRITE_MEMORY(0x16E129C, void*, TitleUI_TitleCMainCState_SelectMenuEnd);
	WRITE_JUMP(0x572D3A, (void*)0x57326B); // Don't create delete save dialog
	WRITE_JUMP(0x5732D3, (void*)0x573337); // Always delete save on new game
	WRITE_NOP(0x573391, 5);

	// Hijack CTitleOption::CStateOutro for saving
	INSTALL_HOOK(TitleUI_CGameplayFlowStage_CStateGoalBegin);
	WRITE_JUMP(0xD22A83, (void*)0xD22B84);
	WRITE_MEMORY(0xD22CE8, uint8_t, 0);
	WRITE_NOP(0xD22CC9, 5); // don't display auto save icon here

	// Yes No Window
	m_yesNoText[YesNoTextType::YNTT_QuitGame] = "Quit  the  game.\nThe  progress  of  the  game  from  the  last  saved\npoint  will  not  be  saved.  OK?";
	m_yesNoText[YesNoTextType::YNTT_QuitGameJP] = u8"ゲームを終了します\n最後にセーブしたところから\nここまでの進行は保存されませんが\nそれでもよろしいですか？";
	m_yesNoText[YesNoTextType::YNTT_ReturnTitle] = "Return  to  Title  Screen.\nThe  progress  of  the  game  from  the  last  saved\npoint  will  not  be  saved.  OK?";
	m_yesNoText[YesNoTextType::YNTT_ReturnTitleJP] = u8"ゲームを終了して、タイトルに戻ります\n最後にセーブしたところから\nここまでの進行は保存されませんが\nそれでもよろしいですか？";
	m_yesNoText[YesNoTextType::YNTT_NewGame] = "Are  you  sure  you  want  to  start  a  new  game?\nAny  previous  saved  game\nprogress  will  be  lost.";
	m_yesNoText[YesNoTextType::YNTT_NewGameJP] = u8"これまでの記録は消えてしまいますが\nよろしいですか？";
	m_yesNoText[YesNoTextType::YNTT_COUNT] = "MISSING TEXT"; // need this for dummy text

	// CDemoMenuObject
	WRITE_NOP(0x5764BE, 6); // skip waiting for animation
	WRITE_NOP(0x5764DD, 6); // skip waiting for animation
	WRITE_NOP(0x57656B, 5); // no confirm sfx
	WRITE_JUMP(0x576528, (void*)0x576561); // immediately confirm
	INSTALL_HOOK(TitleUI_CDemoMenuObjectAdvance);
}

void TitleUI::populateTrialData()
{
	const INIReader reader(Application::getModDirString() + "Assets\\Title\\trialData.ini");
	m_allowStoryMode = reader.GetBoolean("Settings", "allowStoryMode", true);
	m_displayNonCompletedStage = reader.GetBoolean("Settings", "displayNonCompletedStage", false);
	m_allowPlayNonCompletedStage = reader.GetBoolean("Settings", "allowPlayNonCompletedStage", false);

	struct SectionSort
	{
		std::string m_section;
		int m_sort;
	};
	std::vector<SectionSort> sectionSorted;
	for (std::string const& section : reader.Sections())
	{
		if (reader.GetBoolean(section, "enabled", false))
		{
			sectionSorted.push_back({ section, reader.GetInteger(section, "sort", 0) });
		}
	}
	std::sort(sectionSorted.begin(), sectionSorted.end(), [](SectionSort const& a, SectionSort const& b) {return a.m_sort < b.m_sort; });

	for (SectionSort const& sectionSort : sectionSorted)
	{
		TrialData data;
		data.m_stage = std::stoi(sectionSort.m_section);

		data.m_header = reader.Get(sectionSort.m_section, "header", "");
		data.m_stageID = reader.Get(sectionSort.m_section, "stageID", "");
		if (data.m_stageID.empty()) continue;

		data.m_actName = reader.Get(sectionSort.m_section, "actName", "");
		data.m_actName = std::regex_replace(data.m_actName, std::regex(" "), "  ");
		data.m_actName = std::regex_replace(data.m_actName, std::regex("\\\\n"), "\n");
		data.m_missionName = reader.Get(sectionSort.m_section, "missionName", "");
		data.m_missionName = std::regex_replace(data.m_missionName, std::regex(" "), "  ");
		data.m_missionName = std::regex_replace(data.m_missionName, std::regex("\\\\n"), "\n");
		data.m_missionNameJP = reader.Get(sectionSort.m_section, "missionNameJP", "");
		if (data.m_actName.empty() && (data.m_missionName.empty() || data.m_missionNameJP.empty())) continue;

		if (!data.m_actName.empty())
		{
			m_actTrialData.push_back(data);
		}
		else
		{
			m_townTrialData.push_back(data);
		}
	}
}

void TitleUI::refreshTrialAvailability()
{
	m_actTrialVisibleID.clear();
	int id = 0;
	for (TrialData& data : m_actTrialData)
	{
		data.m_playable = m_allowPlayNonCompletedStage || Common::IsStageCompleted(data.m_stage);
		if (m_displayNonCompletedStage || data.m_playable)
		{
			m_actTrialVisibleID.push_back(id);
		}
		id++;

		// Boss hard mode
		data.m_hardModePlayable = false;
		uint8_t stageFirstByte = data.m_stage & 0xFF;
		if (stageFirstByte >= SMT_bms && stageFirstByte <= SMT_blb)
		{
			data.m_hardModePlayable = Common::IsStageCompleted(data.m_stage);
		}
	}

	m_townTrialVisibleID.clear();
	id = 0;
	for (TrialData& data : m_townTrialData)
	{
		data.m_playable = m_allowPlayNonCompletedStage || Common::IsStageCompleted(data.m_stage);
		if (m_displayNonCompletedStage || data.m_playable)
		{
			m_townTrialVisibleID.push_back(id);
		}
		id++;
	}
}

void TitleUI::populateStageData(size_t stage, std::string stageID)
{
	float bestTime, bestTime2, bestTime3;
	uint32_t bestRank;
	m_stageData = StageData();
	if (!Common::GetStageData
	(
		stage,
		m_stageData.m_bestScore,
		bestTime,
		bestTime2,
		bestTime3,
		bestRank,
		m_stageData.m_silverMedalCount
	)) return;

	m_stageData.m_stage = stage;
	m_stageData.m_stageID = stageID;

	uint8_t stageFirstByte = stage & 0xFF;
	m_stageData.m_isBoss = stageFirstByte >= SMT_bms && stageFirstByte <= SMT_blb;

	// Time
	uint32_t minutes, seconds, milliseconds;
	uint32_t totalMilliseconds = bestTime * 1000.0f;
	minutes = totalMilliseconds / 60000;
	if (bestTime > 0.0f && minutes <= 99)
	{
		seconds = (totalMilliseconds % 60000) / 1000;
		milliseconds = (totalMilliseconds % 60000) % 1000;
	}
	else
	{
		minutes = 99;
		seconds = 59;
		milliseconds = 999;
	}
	sprintf(m_stageData.m_bestTime, "%02d'%02d\"%03d", minutes, seconds, milliseconds);

	// Rank
	if (m_sceneMissionText)
	{
		m_sceneMissionText->GetNode("rank")->SetPatternIndex(bestRank);
	}

	// Hide silver medal if mission or boss
	m_stageData.m_isMission = (stage & 0xFF00) > 0 && (stage & 0xFF) <= 0x11;
	m_sceneMissionText->GetNode("item_icon")->SetHideFlag(m_stageData.m_isMission || m_stageData.m_isBoss);
}

void TitleUI::cursorStageSelect(int index, bool isMission)
{
	std::vector<size_t> const& visibleID = isMission ? m_townTrialVisibleID : m_actTrialVisibleID;
	int& cursorIndex = isMission ? m_missionCursorIndex : m_stageCursorIndex;

	cursorIndex = index;
	if (cursorIndex < 0)
	{
		cursorIndex = visibleID.size() - 1;
	}
	else if (cursorIndex >= visibleID.size())
	{
		cursorIndex = 0;
	}

	int visualCursorIndex = cursorIndex;
	if (visibleID.size() > 7 && cursorIndex > 2)
	{
		if (cursorIndex < visibleID.size() - 3)
		{
			visualCursorIndex = 3;
		}
		else
		{
			visualCursorIndex = cursorIndex - (visibleID.size() - 7);
		}
	}

	if (isMission)
	{
		cursorMission(visualCursorIndex);
	}
	else
	{
		cursorStage(visualCursorIndex);
	}
}

void TitleUI::cursorMission(int index)
{
	static char const* cursorSelectName[] =
	{
		"stagecursor_loop",
		"stagecursor_loop02",
		"stagecursor_loop03",
		"stagecursor_loop04",
		"stagecursor_loop05",
		"stagecursor_loop06",
		"stagecursor_loop07",
	};
	TitleUI_PlayMotion(m_sceneMissionCursor, cursorSelectName[index]);

	cursorStageArrow(index);
}

void TitleUI::cursorStage(int index)
{
	static char const* cursorSelectName[] =
	{
		"stagecursor_loop",
		"stagecursor02",
		"stagecursor03",
		"stagecursor04",
		"stagecursor05",
		"stagecursor06",
		"stagecursor07",
	};
	TitleUI_PlayMotion(m_sceneStageCursor, cursorSelectName[index]);

	cursorStageArrow(index);
}

void TitleUI::cursorStageArrow(int index)
{
	static char const* cursorArrowName[] =
	{
		"stagecursor2_loop",
		"stagecursor2_loop02",
		"stagecursor2_loop03",
		"stagecursor2_loop04",
		"stagecursor2_loop05",
		"stagecursor2_loop06",
		"stagecursor2_loop07",
	};
	TitleUI_PlayMotion(m_sceneStageCursor2, cursorArrowName[index], true);
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

void TitleUI::optionEnable(bool enable)
{
	TitleUI_PlayMotion(m_sceneOptionBG, "back_in", false, !enable);
	for (int i = 0; i < OptionType::OT_COUNT; i++)
	{
		TitleUI_PlayMotion(m_sceneOptionText[i], "option_in", false, !enable);
	}

	if (enable)
	{
		// Force layout refresh
		m_optionIndex = -1;
	}
	optionSetIndex(enable ? 0 : -1);
}

void TitleUI::optionSetIndex(int index)
{
	for (int i = 0; i < OptionType::OT_COUNT; i++)
	{
		m_sceneOptionText[i]->GetNode("op_ita01")->SetPatternIndex(index == i ? 1 : 0);
	}

	if (m_optionIndex != index)
	{
		// Hide previous option menu
		switch (m_optionIndex)
		{
		case OptionType::OT_Audio:
			TitleUI_PlayMotion(m_sceneOptionAudio, "audio_in", false, true);
			m_sceneOptionAudioBar1->SetHideFlag(true);
			m_sceneOptionAudioBar2->SetHideFlag(true);
			break;
		case OptionType::OT_UI:
		case OptionType::OT_VO:
		case OptionType::OT_Dialog:
		case OptionType::OT_Subtitle:
			TitleUI_PlayMotion(m_sceneOptionOnOff, "jimaku_in", false, true);
			break;
		}

		// Set new option menu
		switch (index)
		{
		case OptionType::OT_Audio:
			TitleUI_PlayMotion(m_sceneOptionAudio, "audio_in", false, false);
			m_sceneOptionAudioBar1->SetHideFlag(false);
			m_sceneOptionAudioBar2->SetHideFlag(false);
			m_sceneOptionAudioBar1->m_MotionFrame = *TitleUI_GetMusicVolume() / 0.63f * 100.0f;
			m_sceneOptionAudioBar2->m_MotionFrame = *TitleUI_GetEffectVolume() / 0.63f * 100.0f;
			break;
		case OptionType::OT_UI:
		case OptionType::OT_VO:
			TitleUI_PlayMotion(m_sceneOptionOnOff, "jimaku_in", false, false);
			m_sceneOptionOnOff->GetNode("text_on")->SetPatternIndex(1);
			m_sceneOptionOnOff->GetNode("text_off")->SetPatternIndex(1);
			if (index == OptionType::OT_UI)
			{
				optionOnOffSetIndex(*TitleUI_GetUILanguageType() == LT_Japanese ? 1 : 0);
			}
			else
			{
				optionOnOffSetIndex(*TitleUI_GetVoiceLanguageType() == LT_Japanese ? 1 : 0);
			}
			break;
		case OptionType::OT_Dialog:
		case OptionType::OT_Subtitle:
			TitleUI_PlayMotion(m_sceneOptionOnOff, "jimaku_in", false, false);
			m_sceneOptionOnOff->GetNode("text_on")->SetPatternIndex(0);
			m_sceneOptionOnOff->GetNode("text_off")->SetPatternIndex(0);
			if (index == OptionType::OT_Dialog)
			{
				optionOnOffSetIndex((*TitleUI_GetOptionFlag() & 0x2) == 0 ? 1 : 0);
			}
			else
			{
				optionOnOffSetIndex((*TitleUI_GetOptionFlag() & 0x10) == 0 ? 1 : 0);
			}
			break;
		}
	}

	if (index == -1)
	{
		optionCursorSetIndex(m_optionIndex, true);
	}
	else
	{
		optionCursorSetIndex(index);
	}

	m_optionIndex = index;
}

void TitleUI::optionAudioSetIndex(int index)
{
	m_optionAudioIndex = index;

	m_sceneOptionAudio->GetNode("au011")->SetPatternIndex(index == 0 ? 1 : 0);
	m_sceneOptionAudio->GetNode("au012")->SetPatternIndex(index == 0 ? 1 : 0);
	m_sceneOptionAudio->GetNode("au013")->SetPatternIndex(index == 0 ? 1 : 0);
	m_sceneOptionAudio->GetNode("au021")->SetPatternIndex(index == 1 ? 1 : 0);
	m_sceneOptionAudio->GetNode("au022")->SetPatternIndex(index == 1 ? 1 : 0);
	m_sceneOptionAudio->GetNode("au023")->SetPatternIndex(index == 1 ? 1 : 0);

	if (m_optionAudioIndex >= 0)
	{
		optionCursorSetIndex(m_optionIndex, false, m_optionAudioIndex, true);
	}
}

void TitleUI::optionOnOffSetIndex(int index)
{
	m_optionOnOffIndex = index;

	m_sceneOptionOnOff->GetNode("jimaku011")->SetPatternIndex(index == 0 ? 1 : 0);
	m_sceneOptionOnOff->GetNode("jimaku012")->SetPatternIndex(index == 0 ? 1 : 0);
	m_sceneOptionOnOff->GetNode("jimaku013")->SetPatternIndex(index == 0 ? 1 : 0);
	m_sceneOptionOnOff->GetNode("jimaku021")->SetPatternIndex(index == 1 ? 1 : 0);
	m_sceneOptionOnOff->GetNode("jimaku022")->SetPatternIndex(index == 1 ? 1 : 0);
	m_sceneOptionOnOff->GetNode("jimaku023")->SetPatternIndex(index == 1 ? 1 : 0);

	if (m_optionOnOffIndex >= 0)
	{
		optionCursorSetIndex(m_optionIndex, false, m_optionOnOffIndex, false);
	}
}

void TitleUI::optionCursorSetIndex(int index, bool out, int subIndex, bool subOptionAudio)
{
	// cursor
	switch (index)
	{
	case 0:
		TitleUI_PlayMotion(m_sceneOptionCursor, "select_l01", false, out);
		break;
	case 1:
		TitleUI_PlayMotion(m_sceneOptionCursor, "select_l02", false, out);
		break;
	case 2:
		TitleUI_PlayMotion(m_sceneOptionCursor, "select_l03", false, out);
		break;
	case 3:
		TitleUI_PlayMotion(m_sceneOptionCursor, "select_l04", false, out);
		break;
	case 4:
		TitleUI_PlayMotion(m_sceneOptionCursor, "select_l05", false, out);
		break;
	}

	if (subIndex >= 0)
	{
		if (subOptionAudio)
		{
			TitleUI_PlayMotion(m_sceneOptionCursor, subIndex == 0 ? "select_t01" : "select_t02");
		}
		else
		{
			TitleUI_PlayMotion(m_sceneOptionCursor, subIndex == 0 ? "select_r01" : "select_r02");
		}
	}
}

void TitleUI::menuTitleSecondary(bool to2nd, size_t patternIndex)
{
	TitleUI_PlayMotion(m_sceneMenuTitleText, "mainmenu_out", false, !to2nd);

	if (to2nd)
	{
		m_sceneMenuTitleText2->GetNode("title_menu")->SetPatternIndex(patternIndex);
		TitleUI_PlayMotion(m_sceneMenuTitleBarEffect, "DefaultAnim");
	}
	TitleUI_PlayMotion(m_sceneMenuTitleText2, "main_start", false, !to2nd);
}

void TitleUI::menuTextLeft(Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, bool out)
{
	TitleUI_PlayMotion(scene, "episodeselect_select", false, out);
}

void TitleUI::menuTextRight(Chao::CSD::RCPtr<Chao::CSD::CScene> const& scene, bool out)
{
	TitleUI_PlayMotion(scene, "episodeselect_out", false, !out);
}

void TitleUI::EnableYesNoWindow(bool enabled, bool defaultYes, std::string const& text)
{
	if (!m_sceneYesNoTop || !m_sceneYesNoBottom) return;

	m_sceneYesNoTop->SetHideFlag(!enabled);
	m_sceneYesNoBottom->SetHideFlag(!enabled);
	m_sceneYesNoCursor->SetHideFlag(!enabled);

	if (enabled)
	{
		m_yesNoWindowText = text;
		m_yesNoCursorPos = defaultYes ? 0 : 1;
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
	static bool visible = true;

	//-------------------------------------------------------------
	// Act Trial
	//-------------------------------------------------------------
	if (m_menuState == MenuState::MS_ActTrial && !m_drawActTrial)
	{
		m_drawActTrial = true;
		m_drawActTrialAlpha = 0.0f;
	}
	else if (m_menuState != MenuState::MS_ActTrial && m_drawActTrial)
	{
		m_drawActTrial = false;
	}

	if (Common::IsAtLoadingScreen() || m_menuState == MenuState::MS_Idle)
	{
		m_drawActTrialAlpha = -1.0f;
	}

	if (m_drawActTrial || m_drawActTrialAlpha >= 0.0f)
	{
		ImGui::Begin("ActTrial", &visible, UIContext::m_hudFlags);
		{
			ImGui::SetWindowFocus();
			float posX = 0.1130f; // header pos
			float posX2 = posX + 0.125f; // act name pos
			float posY = 0.1667f; // final pos
			float posYDiff = 10.0f / 720.0f; // fade out pos
			float constexpr yDist = 50.0f / 720.0f;

			int startIndex = 0;
			if (m_actTrialVisibleID.size() > 7 && m_stageCursorIndex > 3)
			{
				if (m_stageCursorIndex < m_actTrialVisibleID.size() - 3)
				{
					startIndex = m_stageCursorIndex - 3;
				}
				else
				{
					startIndex = m_actTrialVisibleID.size() - 7;
				}
			}

			for (int i = startIndex; i < m_actTrialVisibleID.size() && i < startIndex + 7; i++)
			{
				size_t id = m_actTrialVisibleID[i];
				TrialData const& data = m_actTrialData[id];
				ImVec4 color = data.m_playable
					? ImVec4(1.0f, 1.0f, 1.0f, m_drawActTrialAlpha)
					: ImVec4(0.6f, 0.6f, 0.6f, m_drawActTrialAlpha);

				float posYFinal = posY + posYDiff * (1.0f - m_drawActTrialAlpha);
				ImGui::SetCursorPos(ImVec2(*BACKBUFFER_WIDTH * posX, *BACKBUFFER_HEIGHT * posYFinal));
				ImGui::TextColored(color, data.m_header.c_str());
				ImGui::SetCursorPos(ImVec2(*BACKBUFFER_WIDTH * posX2, *BACKBUFFER_HEIGHT * posYFinal));
				ImGui::TextColored(color, data.m_actName.c_str());
				posY += yDist;
			}
		}
		ImGui::End();

		if (m_drawActTrial)
		{
			m_drawActTrialAlpha = min(1.0f, m_drawActTrialAlpha + Application::getDeltaTime() * 60.0f / 5.0f);
		}
		else
		{
			m_drawActTrialAlpha -= Application::getDeltaTime() * 60.0f / 5.0f;
		}
	}

	//-------------------------------------------------------------
	// Town Trial
	//-------------------------------------------------------------
	bool drawTownTrial = m_menuState == MenuState::MS_TownTrial || m_menuState == MenuState::MS_FadeOutMission;
	if (drawTownTrial && !m_drawTownTrial)
	{
		m_drawTownTrial = true;
		m_drawTownTrialAlpha = 0.0f;
	}
	else if (!drawTownTrial && m_drawTownTrial)
	{
		m_drawTownTrial = false;
	}

	if (Common::IsAtLoadingScreen() || m_menuState == MenuState::MS_Idle)
	{
		m_drawTownTrialAlpha = -1.0f;
	}

	if (m_drawTownTrial || m_drawTownTrialAlpha >= 0.0f)
	{
		ImGui::Begin("TownTrial", &visible, UIContext::m_hudFlags);
		{
			ImGui::SetWindowFocus();
			float posX = 0.1130f; // header pos
			float posY = 0.1667f; // final pos
			float posYDiff = 10.0f / 720.0f; // fade out pos
			float constexpr yDist = 50.0f / 720.0f;

			int startIndex = 0;
			if (m_townTrialVisibleID.size() > 7 && m_missionCursorIndex > 3)
			{
				if (m_missionCursorIndex < m_townTrialVisibleID.size() - 3)
				{
					startIndex = m_missionCursorIndex - 3;
				}
				else
				{
					startIndex = m_townTrialVisibleID.size() - 7;
				}
			}

			for (int i = startIndex; i < m_townTrialVisibleID.size() && i < startIndex + 7; i++)
			{
				size_t id = m_townTrialVisibleID[i];
				TrialData const& data = m_townTrialData[id];
				ImVec4 color = data.m_playable
					? ImVec4(1.0f, 1.0f, 1.0f, m_drawTownTrialAlpha)
					: ImVec4(0.6f, 0.6f, 0.6f, m_drawTownTrialAlpha);

				float posYFinal = posY + posYDiff * (1.0f - m_drawTownTrialAlpha);
				ImGui::SetCursorPos(ImVec2(*BACKBUFFER_WIDTH * posX, *BACKBUFFER_HEIGHT * posYFinal));
				ImGui::TextColored(color, (std::string("MISSION  ") + std::to_string(i + 1)).c_str());
				posY += yDist;
			}
		}
		ImGui::End();

		if (m_drawTownTrial)
		{
			m_drawTownTrialAlpha = min(1.0f, m_drawTownTrialAlpha + Application::getDeltaTime() * 60.0f / 5.0f);
		}
		else
		{
			m_drawTownTrialAlpha -= Application::getDeltaTime() * 60.0f / 5.0f;
		}
	}

	//-------------------------------------------------------------
	// Mode Select
	//-------------------------------------------------------------
	bool drawModeSelect = m_menuState == MenuState::MS_ModeSelect || m_menuState == MenuState::MS_FadeOutStage;
	if (drawModeSelect && !m_drawModeSelect)
	{
		m_drawModeSelect = true;
		m_drawModeSelectAlpha = 0.0f;
	}
	else if (!drawModeSelect && m_drawModeSelect)
	{
		m_drawModeSelect = false;
	}

	if (Common::IsAtLoadingScreen() || m_menuState == MenuState::MS_Idle)
	{
		m_drawModeSelectAlpha = -1.0f;
	}

	if (m_drawModeSelect || m_drawModeSelectAlpha >= 0.0f)
	{
		ImGui::Begin("ModeSelect", &visible, UIContext::m_hudFlags);
		{
			ImGui::SetWindowFocus();
			float posX = 0.1130f; // header pos
			float posY = 0.1667f; // final pos
			float posYDiff = 10.0f / 720.0f; // fade out pos
			float constexpr yDist = 50.0f / 720.0f;

			for (int i = 0; i < (m_stageData.m_isBoss ? 2 : 1); i++)
			{
				size_t id = m_actTrialVisibleID[m_stageCursorIndex];
				TrialData const& data = m_actTrialData[id];
				ImVec4 color = ((i == 0 && data.m_playable) || (i == 1 && data.m_hardModePlayable))
					? ImVec4(1.0f, 1.0f, 1.0f, m_drawModeSelectAlpha)
					: ImVec4(0.6f, 0.6f, 0.6f, m_drawModeSelectAlpha);

				float posYFinal = posY + posYDiff * (1.0f - m_drawModeSelectAlpha);
				ImGui::SetCursorPos(ImVec2(*BACKBUFFER_WIDTH * posX, *BACKBUFFER_HEIGHT * posYFinal));
				ImGui::TextColored(color, m_stageData.m_isBoss ? (i == 0 ? "NORMAL" : "HARD") : "MISSION");
				posY += yDist;
			}
		}
		ImGui::End();

		if (m_drawModeSelect)
		{
			m_drawModeSelectAlpha = min(1.0f, m_drawModeSelectAlpha + Application::getDeltaTime() * 60.0f / 5.0f);
		}
		else
		{
			m_drawModeSelectAlpha -= Application::getDeltaTime() * 60.0f / 5.0f;
		}
	}
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

void TitleUI::drawStageData()
{
	static bool visible = true;

	//-------------------------------------------------------------
	// Stage Data
	//-------------------------------------------------------------
	if (m_drawModeSelect || m_drawTownTrial)
	{
		ImGui::Begin("StageData", &visible, UIContext::m_hudFlags);
		{
			ImGui::SetWindowFocus();
			size_t stageID = m_drawTownTrial ? m_townTrialVisibleID[m_missionCursorIndex] : m_actTrialVisibleID[m_stageCursorIndex];
			TrialData const& data = m_drawTownTrial ? m_townTrialData[stageID] : m_actTrialData[stageID];

			ImVec2 offset = ImVec2(0.0385416666666667f, 0.0722222222f);
			ImGui::SetCursorPos(ImVec2(*BACKBUFFER_WIDTH * (0.46875f - offset.x), *BACKBUFFER_HEIGHT * (0.2638889f - offset.y)));
			ImGui::Text
			(
				m_drawTownTrial ? 
				(
					data.m_playable ?
					(
						Common::GetUILanguageType() == LT_Japanese
						? data.m_missionNameJP.c_str()
						: data.m_missionName.c_str()
					)
					: "???"
				) 
				: data.m_actName.c_str()
			);
			ImGui::SetCursorPos(ImVec2(*BACKBUFFER_WIDTH * (0.46875f - offset.x), *BACKBUFFER_HEIGHT * (0.3888889f - offset.y)));
			ImGui::Text("BEST  TIME");
			ImGui::SetCursorPos(ImVec2(*BACKBUFFER_WIDTH * (0.5726563f - offset.x), *BACKBUFFER_HEIGHT * (0.43888888f - offset.y)));
			ImGui::Text(m_stageData.m_bestTime);
			ImGui::SetCursorPos(ImVec2(*BACKBUFFER_WIDTH * (0.46875f - offset.x), *BACKBUFFER_HEIGHT * (0.4875f - offset.y)));
			ImGui::Text("BEST  SCORE");
			ImGui::SetCursorPos(ImVec2(*BACKBUFFER_WIDTH * (0.5726563f - offset.x), *BACKBUFFER_HEIGHT * (0.5375f - offset.y)));
			ImGui::Text(std::to_string(m_stageData.m_bestScore).c_str());
			
			if (!m_stageData.m_isMission && !m_stageData.m_isBoss)
			{
				ImGui::SetCursorPos(ImVec2(*BACKBUFFER_WIDTH * (0.46875f - offset.x), *BACKBUFFER_HEIGHT * (0.5875f - offset.y)));
				ImGui::Text("SILVER MEDAL");
				ImGui::SetCursorPos(ImVec2(*BACKBUFFER_WIDTH * (0.5726563f - offset.x), *BACKBUFFER_HEIGHT * (0.6421112f - offset.y)));
				ImGui::Text(std::to_string(m_stageData.m_silverMedalCount).c_str());
			}
		}
		ImGui::End();
	}

	//-------------------------------------------------------------
	// Fade Out
	//-------------------------------------------------------------
	if (m_menuState == MenuState::MS_FadeOut 
	|| m_menuState == MenuState::MS_FadeOutTitle
	|| m_menuState == MenuState::MS_FadeOutStage
	|| m_menuState == MenuState::MS_FadeOutMission)
	{
		ImGui::Begin("FadeOut", &visible, UIContext::m_hudFlags);
		{
			ImGui::SetWindowFocus();
			ImGui::Image(LoadingUI::m_backgroundTexture, ImVec2(*BACKBUFFER_WIDTH * 1.1f, *BACKBUFFER_HEIGHT * 1.1f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, min(1.0f, m_fadeOutTime)));
			ImGui::SetWindowPos(ImVec2(-10, -10));
		}
		ImGui::End();

		m_fadeOutTime += Application::getDeltaTime();
		if (Common::IsAtLoadingScreen())
		{
			m_menuState = MenuState::MS_Idle;
		}
	}
}
