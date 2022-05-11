#include "CustomHUD.h"

//---------------------------------------------------
// Utilities
//---------------------------------------------------
void CustomHUD::CreateScreen
(
    Chao::CSD::RCPtr<Chao::CSD::CProject>& project,
    boost::shared_ptr<Sonic::CGameObjectCSD>& object,
    Hedgehog::Base::CStringSymbol const& in_RenderableCategory,
    bool updateAtPause,
    Sonic::CGameObject* parent
)
{
    if (project && !object)
    {
        object = boost::make_shared<Sonic::CGameObjectCSD>(project, 0.5f, in_RenderableCategory, updateAtPause);
        Sonic::CGameDocument::GetInstance()->AddGameObject(object, "main", parent);
    }
}

void CustomHUD::KillScreen(boost::shared_ptr<Sonic::CGameObjectCSD>& object)
{
    if (object)
    {
        object->SendMessage(object->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
        object = nullptr;
    }
}

void CustomHUD::ToggleScreen
(
    Chao::CSD::RCPtr<Chao::CSD::CProject>& project,
    boost::shared_ptr<Sonic::CGameObjectCSD>& object,
    Hedgehog::Base::CStringSymbol const& in_RenderableCategory,
    bool updateAtPause,
    Sonic::CGameObject* parent
)
{
    // ms_IsRenderGameMainHud
    if (*(bool*)0x1A430D8)
    {
        CreateScreen(project, object, in_RenderableCategory, updateAtPause, parent);
    }
    else
    {
        KillScreen(object);
    }
}

void CustomHUD::SetScreenVisible
(
    bool const visible,
    boost::shared_ptr<Sonic::CGameObjectCSD>& object
)
{
    if (object)
    {
        object->SendMessage(object->m_ActorID, boost::make_shared<Sonic::Message::MsgSetVisible>(visible));
    }
}

//---------------------------------------------------
// Main Gameplay
//---------------------------------------------------
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectPlayScreen;
boost::shared_ptr<Sonic::CGameObjectCSD> m_spPlayScreen;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneLifeCount;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneLifeBG;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneTimeCount;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneRingCount;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneRingIcon;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneRingEffect;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_scenePowerBG;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_scenePowerBar;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_scenePowerEffect;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_scenePower;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneScore;

Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectCountdown;
boost::shared_ptr<Sonic::CGameObjectCSD> m_spCountdown;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneCountdown;

bool CustomHUD::m_scoreEnabled = false;
float CustomHUD::m_missionMaxTime = 0.0f;

void __fastcall CustomHUD::CHudSonicStageRemoveCallback(Sonic::CGameObject* This, void*, Sonic::CGameDocument* pGameDocument)
{
    KillScreen(m_spPlayScreen);
    KillScreen(m_spCountdown);

    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_sceneLifeCount);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_sceneLifeBG);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_sceneTimeCount);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_sceneRingCount);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_sceneRingIcon);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_sceneRingEffect);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_scenePowerBG);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_scenePowerBar);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_scenePowerEffect);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_scenePower);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_sceneScore);

    Chao::CSD::CProject::DestroyScene(m_projectCountdown.Get(), m_sceneCountdown);

    m_projectPlayScreen = nullptr;
    m_projectCountdown = nullptr;
}

HOOK(void, __fastcall, CustomHUD_CHudSonicStageInit, 0x109A8D0, Sonic::CGameObject* This)
{
    originalCustomHUD_CHudSonicStageInit(This);
    CustomHUD::CHudSonicStageRemoveCallback(This, nullptr, nullptr);

    Sonic::CCsdDatabaseWrapper wrapper(This->m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());

    boost::shared_ptr<Sonic::CCsdProject> spCsdProject;
    wrapper.GetCsdProject(spCsdProject, "maindisplay");
    m_projectPlayScreen = spCsdProject->m_rcProject;
    wrapper.GetCsdProject(spCsdProject, "time");
    m_projectCountdown = spCsdProject->m_rcProject;

    size_t& flags = ((size_t*)This)[151];

    if (flags & 0x1 || Common::IsCurrentStageMission())
    {
        m_sceneLifeCount = m_projectPlayScreen->CreateScene("life");
        m_sceneLifeCount->SetMotionTime(m_sceneLifeCount->m_MotionEndTime);
        m_sceneLifeCount->m_MotionSpeed = 1.0f;
        m_sceneLifeCount->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
        int iconIndex = 0;
        switch (S06DE_API::GetModelType())
        {
        case S06DE_API::ModelType::Blaze: iconIndex = 8;
        }
        m_sceneLifeCount->GetNode("character_icon")->SetPatternIndex(iconIndex);

        m_sceneLifeBG = m_projectPlayScreen->CreateScene("life_ber_anime");
        m_sceneLifeBG->SetMotionTime(m_sceneLifeBG->m_MotionEndTime);
        m_sceneLifeBG->m_MotionSpeed = 0.0f;
    }

    if (flags & 0x2)
    {
        m_sceneTimeCount = m_projectPlayScreen->CreateScene("time");
        m_sceneTimeCount->m_MotionSpeed = 0.0f;
    }

    if (flags & 0x4)
    {
        m_sceneRingCount = m_projectPlayScreen->CreateScene("ring");
        m_sceneRingCount->m_MotionSpeed = 0.0f;

        m_sceneRingIcon = m_projectPlayScreen->CreateScene("ring_anime");
        m_sceneRingIcon->SetMotionTime(m_sceneRingIcon->m_MotionEndTime);
        m_sceneRingIcon->m_MotionSpeed = 1.0f;
        m_sceneRingIcon->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;

        m_sceneRingEffect = m_projectPlayScreen->CreateScene("ring_000_effect");
        m_sceneRingEffect->m_MotionSpeed = 1.0f;
        m_sceneRingEffect->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_Loop;
    }

    if (flags & 0x200) // Boost Gauge
    {
        m_scenePowerBG = m_projectPlayScreen->CreateScene("power");
        m_scenePowerBG->m_MotionSpeed = 0.0f;

        m_scenePowerBar = m_projectPlayScreen->CreateScene("bar_ue");
        m_scenePowerBar->m_MotionSpeed = 0.0f;

        m_scenePowerEffect = m_projectPlayScreen->CreateScene("power_bar_effect");
        m_scenePowerEffect->m_MotionSpeed = 1.0f;
        m_scenePowerEffect->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_Loop;

        m_scenePower = m_projectPlayScreen->CreateScene("power_bar_anime");
        m_scenePower->m_MotionSpeed = 0.0f;
    }

    if (flags & 0x2000) // Countdown
    {
        m_sceneCountdown = m_projectCountdown->CreateScene("Scene_0001");
        m_sceneCountdown->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_Loop;
    }

    if (CustomHUD::m_scoreEnabled) // score
    {
        m_sceneScore = m_projectPlayScreen->CreateScene("score");
        m_sceneScore->m_MotionSpeed = 0.0f;
    }

    // Mask to prevent crash when game tries accessing the elements we disabled later on
    flags &= ~(0x1 | 0x2 | 0x4 | 0x200 | 0x800);
}

void __declspec(naked) CustomHUD_GetScoreEnabled()
{
    static uint32_t returnAddress = 0x109C254;
    __asm
    {
        mov		CustomHUD::m_scoreEnabled, 1
        jmp     [returnAddress]
    }
}

void CustomHUD_GetTime(Sonic::CGameDocument* pGameDocument, size_t* minutes, size_t* seconds, size_t* milliseconds)
{
    static uint32_t returnAddress = 0xD61570;
    __asm
    {
        mov     ecx, minutes
        mov     edi, seconds
        mov     esi, milliseconds
        mov     eax, pGameDocument
        call    [returnAddress]
    }
}

HOOK(void, __fastcall, CustomHUD_CHudSonicStageUpdate, 0x1098A50, Sonic::CGameObject* This, void* Edx, const hh::fnd::SUpdateInfo& in_rUpdateInfo)
{
    originalCustomHUD_CHudSonicStageUpdate(This, Edx, in_rUpdateInfo);

    // Always clamp boost to 100
    *Common::GetPlayerBoost() = min(*Common::GetPlayerBoost(), 100.0f);

    CustomHUD::ToggleScreen(m_projectPlayScreen, m_spPlayScreen, "HUD", false, This);
    CustomHUD::ToggleScreen(m_projectCountdown, m_spCountdown, "HUD", false, This);

    if (!m_spPlayScreen)
    {
        return;
    }

    char text[256];

    if (m_sceneLifeCount)
    {
        const size_t liveCountAddr = Common::GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0x9FDC });
        if (liveCountAddr)
        {
            sprintf(text, "%d", *(size_t*)liveCountAddr);
            m_sceneLifeCount->GetNode("life_text")->SetText(text);
        }
    }

    if (m_sceneTimeCount)
    {
        size_t minutes, seconds, milliseconds;
        CustomHUD_GetTime(**This->m_pMember->m_pGameDocument, &minutes, &seconds, &milliseconds);

        sprintf(text, "%02d'%02d\"%03d", minutes, seconds, milliseconds);
        m_sceneTimeCount->GetNode("time_text")->SetText(text);
    }

    const auto playerContext = Sonic::Player::CPlayerSpeedContext::GetInstance();

    if (m_sceneRingCount && playerContext)
    {
        if (playerContext->m_RingCount == 0)
        {
            m_sceneRingCount->GetNode("ring_text")->SetText("aaa");
            m_sceneRingEffect->SetHideFlag(false);
        }
        else
        {
            sprintf(text, "%03d", min(999, playerContext->m_RingCount));
            m_sceneRingCount->GetNode("ring_text")->SetText(text);
            m_sceneRingEffect->SetHideFlag(true);
        }

        // Mission ring count
        if (*(bool*)((uint32_t)This + 620))
        {
            uint32_t prevMissionRingCount = *(uint32_t*)((uint32_t)This + 768);
            if (prevMissionRingCount != playerContext->m_RingCount)
            {
                *(uint32_t*)((uint32_t)This + 772) = prevMissionRingCount;
                *(uint32_t*)((uint32_t)This + 768) = playerContext->m_RingCount;;
                *(uint32_t*)((uint32_t)This + 764) = 1;
            }
        }
    }

    if (m_scenePower && playerContext)
    {
        float boost = min(100.0f, *Common::GetPlayerBoost());
        m_scenePower->SetMotionTime(boost);

        if (boost < 100.0f)
        {
            m_scenePowerEffect->SetHideFlag(true);
            m_scenePowerEffect->SetMotionTime(0.0f);
        }
        else
        {
            m_scenePowerEffect->SetHideFlag(false);
        }
    }

    if (m_sceneCountdown)
    {
        m_sceneCountdown->SetPosition(0.0f, -296.0f);
        const auto pMember = (uint8_t*)This->m_pMember->m_pGameDocument->m_pMember;
        const float elapsedTime = max(0, max(0, *(float*)(pMember + 0x184)) + *(float*)(pMember + 0x18C));
        const float remainingTime = max(0, CustomHUD::m_missionMaxTime - elapsedTime);

        size_t minutes = (size_t)remainingTime / 60;
        size_t seconds = (size_t)remainingTime % 60;
        size_t milliseconds = (size_t)(remainingTime * 1000.0f) % 1000;

        char text[16];
        sprintf(text, "%02d'%02d\"%03d", minutes, seconds, milliseconds);
        m_sceneCountdown->GetNode("Text_0001")->SetText(text);

        if (minutes > 0 || seconds >= 10)
        {
            m_sceneCountdown->SetMotionTime(0.0f);
            m_sceneCountdown->m_MotionSpeed = 0.0f;
            m_sceneCountdown->m_MotionDisableFlag = true;
        }
        else if (m_sceneCountdown->m_MotionDisableFlag)
        {
            m_sceneCountdown->m_MotionSpeed = 1.0f;
            m_sceneCountdown->m_MotionDisableFlag = false;
        }
    }
}

HOOK(void, __fastcall, CustomHUD_MsgSetPinballHud, 0x1095D40, Sonic::CGameObject* This, void* Edx, MsgSetPinballHud const* message)
{
    // Update score
    if ((message->m_flag & 1) && CustomHUD::m_scoreEnabled)
    {
        char text[16];
        sprintf(text, "%d", message->m_score);
        m_sceneScore->GetNode("score_text")->SetText(text);
    }

    originalCustomHUD_MsgSetPinballHud(This, Edx, message);
}

HOOK(void, __fastcall, CustomHUD_MsgNotifySonicHud, 0x1097400, Sonic::CGameObject* This, void* Edx, void* message)
{
    // Ring collect animation
    if (m_sceneRingIcon && m_sceneRingIcon->m_MotionDisableFlag)
    {
        m_sceneRingIcon->SetMotionTime(0.0f);
        m_sceneRingIcon->m_MotionDisableFlag = 0;
        m_sceneRingIcon->Update(0.0f);
    }

    originalCustomHUD_MsgNotifySonicHud(This, Edx, message);
}

HOOK(void, __fastcall, CustomHUD_MsgGetMissionLimitTime, 0xD0F0E0, Sonic::CGameObject* This, void* Edx, hh::fnd::Message& in_rMsg)
{
    originalCustomHUD_MsgGetMissionLimitTime(This, Edx, in_rMsg);
    CustomHUD::m_missionMaxTime = *(float*)((char*)&in_rMsg + 16);
}

//---------------------------------------------------
// Pause
//---------------------------------------------------
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectPause;
boost::shared_ptr<Sonic::CGameObjectCSD> m_spPause;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_scenePauseMenu;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_scenePauseCursor;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_scenePauseText;

uint32_t CustomHUD::m_cursorPos = 0;
bool CustomHUD::m_canRestart = true;
char const* cursorAnimations[] =
{
    "select_01",
    "select_02",
    "select_02", // dummy for Controls
    "select_03",
    "select_04",
    "select_05",
    "select_06"
};
char const* cursorLoopAnimations[] =
{
    "select_01_loop",
    "select02_loop",
    "select02_loop", // dummy for Controls
    "select03_loop",
    "select04_loop",
    "select05_loop",
    "select06_loop"
};

void __fastcall CustomHUD::CPauseRemoveCallback(Sonic::CGameObject* This, void*, Sonic::CGameDocument* pGameDocument)
{
    KillScreen(m_spPause);

    Chao::CSD::CProject::DestroyScene(m_projectPause.Get(), m_scenePauseMenu);
    Chao::CSD::CProject::DestroyScene(m_projectPause.Get(), m_scenePauseCursor);
    Chao::CSD::CProject::DestroyScene(m_projectPause.Get(), m_scenePauseText);

    m_projectPause = nullptr;
}

HOOK(int*, __fastcall, CustomHUD_CPauseUpdate, 0x10A18D0, void* This, void* Edx, Hedgehog::Universe::SUpdateInfo* a2)
{
    if (m_scenePauseMenu)
    {
        // Hide when pause is finished
        if (m_scenePauseMenu->m_MotionSpeed == -1.0f && m_scenePauseMenu->m_MotionTime <= 0.0f)
        {
            m_scenePauseMenu->SetHideFlag(true);
        }
    }

    if (m_scenePauseCursor)
    {
        if (m_scenePauseCursor->m_MotionRepeatType == Chao::CSD::eMotionRepeatType_PlayOnce && m_scenePauseCursor->m_MotionTime >= m_scenePauseCursor->m_MotionEndTime)
        {
            m_scenePauseCursor->SetMotion(cursorLoopAnimations[CustomHUD::m_cursorPos]);
            m_scenePauseCursor->SetMotionTime(0.0f);
            m_scenePauseCursor->m_MotionDisableFlag = false;
            m_scenePauseCursor->m_MotionSpeed = 1.0f;
            m_scenePauseCursor->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_Loop;
            m_scenePauseCursor->Update(0.0f);
        }
    }

    return originalCustomHUD_CPauseUpdate(This, Edx, a2);
}

HOOK(bool, __fastcall, CustomHUD_CPauseVisualActInit, 0x109FAD0, uint32_t* This)
{
    bool result = originalCustomHUD_CPauseVisualActInit(This);

    Sonic::CGameObject* gameObject = (Sonic::CGameObject*)This[1];
    CustomHUD::CPauseRemoveCallback(gameObject, nullptr, nullptr);

    Sonic::CCsdDatabaseWrapper wrapper(gameObject->m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());

    boost::shared_ptr<Sonic::CCsdProject> spCsdProject;
    wrapper.GetCsdProject(spCsdProject, "pausemenu");
    m_projectPause = spCsdProject->m_rcProject;

    m_scenePauseMenu = m_projectPause->CreateScene("pause_menu");
    m_scenePauseMenu->SetHideFlag(true);
    m_scenePauseMenu->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;

    m_scenePauseCursor = m_projectPause->CreateScene("pause_menu_cursor");
    m_scenePauseCursor->SetHideFlag(true);

    bool isJapanese = *(uint8_t*)Common::GetMultiLevelAddress(0x1E66B34, { 0x8 }) == 1;
    m_scenePauseText = m_projectPause->CreateScene("text");
    m_scenePauseText->SetHideFlag(true);
    m_scenePauseText->GetNode("text1")->SetPatternIndex(isJapanese);
    m_scenePauseText->GetNode("text2")->SetPatternIndex(isJapanese * 2);
    m_scenePauseText->GetNode("text3")->SetPatternIndex(isJapanese);

    CustomHUD::CreateScreen(m_projectPause, m_spPause, "HUD_Pause", true, gameObject);

    return result;
}

HOOK(int, __fastcall, CustomHUD_CPauseVisualActCase, 0x109F910, uint32_t* This, void* Edx, int Case)
{
    uint32_t prevCursorPos = CustomHUD::m_cursorPos;
    CustomHUD::m_cursorPos = This[3];

    switch (Case)
    {
    case 0: // Start
    case 5: // Back from confirm dialog
    {
        m_scenePauseMenu->SetHideFlag(false);
        m_scenePauseMenu->SetMotion("pause3");
        m_scenePauseMenu->SetMotionTime(0.0f);
        m_scenePauseMenu->m_MotionDisableFlag = false;
        m_scenePauseMenu->m_MotionSpeed = 1.0f;
        m_scenePauseMenu->Update(0.0f);

        m_scenePauseText->SetHideFlag(false);
        size_t* liveCountAddr = (size_t*)Common::GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0x9FDC });
        if (liveCountAddr)
        {
            CustomHUD::m_canRestart = (*liveCountAddr != 0);
            bool isJapanese = *(uint8_t*)Common::GetMultiLevelAddress(0x1E66B34, { 0x8 }) == 1;
            if (CustomHUD::m_canRestart)
            {
                m_scenePauseText->GetNode("text2")->SetPatternIndex(isJapanese * 2);
            }
            else
            {
                m_scenePauseText->GetNode("text2")->SetPatternIndex(isJapanese * 2 + 1);
            }
        }

        CustomHUD::RefreshPauseCursor(true);
        break;
    }
    case 1: // End
    {
        m_scenePauseMenu->m_MotionDisableFlag = false;
        m_scenePauseMenu->m_PrevMotionTime = m_scenePauseMenu->m_MotionEndTime;
        m_scenePauseMenu->m_MotionTime = m_scenePauseMenu->m_MotionEndTime;
        *(uint32_t*)((uint32_t)m_scenePauseMenu.Get() + 0xB0) = 0;
        *(uint32_t*)((uint32_t)m_scenePauseMenu.Get() + 0xB4) = 0; // this stops the reverse animation
        m_scenePauseMenu->m_MotionSpeed = -1.0f;
        m_scenePauseMenu->Update(0.0f);

        m_scenePauseCursor->SetHideFlag(true);
        m_scenePauseText->SetHideFlag(true);
        break;
    }
    case 3: // New cursor pos
    {
        // 06 doesn't have Controls option
        if (CustomHUD::m_cursorPos == 2)
        {
            if (prevCursorPos < 2)
            {
                This[3]++;
            }
            else
            {
                This[3]--;
            }
        }
        CustomHUD::m_cursorPos = This[3];
        CustomHUD::RefreshPauseCursor(false);
        break;
    }
    case 4: // Confirm
    {
        break;
    }
    default: break;
    }

    return originalCustomHUD_CPauseVisualActCase(This, Edx, Case);
}

HOOK(bool, __fastcall, CustomHUD_CPauseVisualActOpened, 0x109F8F0, uint32_t* This)
{
    return m_scenePauseMenu->m_MotionDisableFlag == 1;
}

HOOK(void, __fastcall, CustomHUD_CPauseCStateSelectAdvance, 0x42A520, uint32_t* This)
{
    originalCustomHUD_CPauseCStateSelectAdvance(This);

    // 06 do not have Controls option
    uint32_t owner = This[2];
    if (owner)
    {
        // internal cursor pos
        int* v4 = *(int**)(owner + 224);
        v4[5] = CustomHUD::m_cursorPos;
    }
}

HOOK(void, __fastcall, CustomHUD_CPauseCStateCloseBegin, 0x42A710, uint32_t* This)
{
    // Prevent playing two sounds when quiting pause
    uint32_t owner = This[2];
    if (owner)
    {
        int* v4 = *(int**)(owner + 224);
        if (*((bool*)v4 + 92) || *((bool*)v4 + 93))
        {
            // Decide or Close
            WRITE_MEMORY(0x11D1F8A, int, -1);
        }
    }

    originalCustomHUD_CPauseCStateCloseBegin(This);

    // Revert original sfx
    WRITE_MEMORY(0x11D1F8A, uint32_t, 1000003);
}

HOOK(void, __fastcall, CustomHUD_CGameplayFlowStageCStateStageUpdating, 0xCFF280, uint32_t* This)
{
    uint32_t* owner = (uint32_t*)(This[2]);
    uint32_t prevPauseFlag = owner[61];

    originalCustomHUD_CGameplayFlowStageCStateStageUpdating(This);

    if (m_scenePauseMenu)
    {
        if (m_scenePauseMenu->m_MotionDisableFlag == 0)
        {
            owner[61] = 0;
        }
        else if (prevPauseFlag != owner[61] && prevPauseFlag == 0)
        {
            FUNCTION_PTR(void, __stdcall, UnpauseSound, 0x10A1480, int CPause);
            UnpauseSound((owner[32]));
        }
    }
}

void CustomHUD::RefreshPauseCursor(bool bPauseStart)
{
    m_scenePauseCursor->SetHideFlag(false);
    m_scenePauseCursor->SetMotion(cursorAnimations[CustomHUD::m_cursorPos]);
    m_scenePauseCursor->SetMotionTime(bPauseStart ? -30.0f : 0.0f);
    m_scenePauseCursor->m_MotionDisableFlag = false;
    m_scenePauseCursor->m_MotionSpeed = 1.0f;
    m_scenePauseCursor->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
    m_scenePauseCursor->Update(0.0f);
}

void __declspec(naked) CustomHUD_PreventRestart()
{
    static uint32_t failedAddress = 0x42A635;
    static uint32_t returnAddress = 0x42A5B3;
    static uint32_t sub_11D1BF0 = 0x11D1BF0; // NG sfx
    __asm
    {
        cmp     CustomHUD::m_cursorPos, 1
        jne     jump

        cmp     CustomHUD::m_canRestart, 0
        jne     jump

        // No life, can't restart
        call    [sub_11D1BF0]
        jmp     [failedAddress]

    jump:
        push    ecx
        mov     ecx, esp
        push    [0x168F940] // DECIDE
        jmp     [returnAddress]
    }
}

void CustomHUD::applyPatches()
{
    //---------------------------------------------------
    // Main Gameplay
    //---------------------------------------------------
    INSTALL_HOOK(CustomHUD_CHudSonicStageInit);
    INSTALL_HOOK(CustomHUD_CHudSonicStageUpdate);
    INSTALL_HOOK(CustomHUD_MsgSetPinballHud);
    INSTALL_HOOK(CustomHUD_MsgNotifySonicHud);
    INSTALL_HOOK(CustomHUD_MsgGetMissionLimitTime);
    WRITE_MEMORY(0x16A467C, void*, CHudSonicStageRemoveCallback);

    // Disable original HUD
    WRITE_JUMP(0x109B1AA, (void*)0x109B485); // life init
    WRITE_JUMP(0x109B496, (void*)0x109B5A1); // time init
    WRITE_JUMP(0x109B5B3, (void*)0x109B69B); // ring init
    WRITE_JUMP(0x109B8FB, (void*)0x109BA9D); // boost gauge init
    WRITE_JUMP(0x109BC8E, (void*)0x109BDF2); // boost button init
    WRITE_JUMP(0x109C1DC, CustomHUD_GetScoreEnabled); // score init
    //WRITE_JUMP(0x109BEF6, (void*)0x109C05A); // countdown init

    // Don't hide HUD at pause, don't run MsgAppearActStageHud
    WRITE_MEMORY(0x10BC141, uint8_t, 0xEB);
    WRITE_JUMP(0x10A1596, (void*)0x10A168E);

    //---------------------------------------------------
    // Pause
    //---------------------------------------------------
    INSTALL_HOOK(CustomHUD_CPauseUpdate);
    INSTALL_HOOK(CustomHUD_CPauseVisualActInit);
    INSTALL_HOOK(CustomHUD_CPauseVisualActCase);
    INSTALL_HOOK(CustomHUD_CPauseVisualActOpened);
    INSTALL_HOOK(CustomHUD_CPauseCStateSelectAdvance);
    INSTALL_HOOK(CustomHUD_CPauseCStateCloseBegin);
    INSTALL_HOOK(CustomHUD_CGameplayFlowStageCStateStageUpdating);
    WRITE_MEMORY(0x16A41A4, void*, CPauseRemoveCallback);

    // Allow UI to goto Start Over even with 0 lives
    WRITE_JUMP(0x10A0FCA, (void*)0x10A1131);

    // Don't unpause sound immediately when unpause
    WRITE_NOP(0x42A5F3, 1); // Pressed B quit pause
    WRITE_NOP(0x42A5FB, 5);
    WRITE_NOP(0x42A6D6, 1); // Pressed A to continue
    WRITE_NOP(0x42A6DD, 5);
    WRITE_NOP(0x10A19EB, 1); // Pressed Start to quit pause
    WRITE_NOP(0x10A19F2, 5);

    // Don't show mission objective at pause
    WRITE_MEMORY(0xD00A46, uint8_t, 0);
    WRITE_MEMORY(0xD07489, uint8_t, 0);

    // Use correct pause SFX
    WRITE_NOP(0x42A41C, 2); // Open
    WRITE_NOP(0x42A75B, 2); // Close
    WRITE_MEMORY(0x11D1D7A, uint32_t, 1000003); // Cancel

    // Prevent restarting when no life
    WRITE_JUMP(0x42A5AB, CustomHUD_PreventRestart);
}
