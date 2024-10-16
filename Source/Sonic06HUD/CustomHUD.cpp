#include "CustomHUD.h"
#include "UIContext.h"
#include "Application.h"
#include "Configuration.h"

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
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneInfo;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_scenePowerBG;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_scenePowerBar;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_scenePowerEffect;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_scenePower;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneScore;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneGem;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneBossBG;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneBossBar;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneCustomBar;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneCustomLevel;

Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectCountdown;
boost::shared_ptr<Sonic::CGameObjectCSD> m_spCountdown;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneCountdown;

bool CustomHUD::m_scoreEnabled = false;
float CustomHUD::m_missionMaxTime = 0.0f;
CustomHUD::SonicGemType CustomHUD::m_sonicGemType = SGT_COUNT;
std::map<CustomHUD::SonicGemType, bool> CustomHUD::m_sonicGemEnabled =
{
    {SGT_Blue,      false},
    {SGT_Red,       false},
    {SGT_Green,     false},
    {SGT_Purple,    false},
    {SGT_Sky,       false},
    {SGT_White,     false},
    {SGT_Yellow,    false},
    {SGT_COUNT,     false},
};

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
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_sceneInfo);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_scenePowerBG);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_scenePowerBar);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_scenePowerEffect);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_scenePower);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_sceneScore);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_sceneGem);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_sceneBossBG);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_sceneBossBar);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_sceneCustomBar);
    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_sceneCustomLevel);

    Chao::CSD::CProject::DestroyScene(m_projectCountdown.Get(), m_sceneCountdown);

    m_projectPlayScreen = nullptr;
    m_projectCountdown = nullptr;
}

void CustomHUD::ScrollSonicGem(bool toRight, bool ignoreNone)
{
    if (!m_sceneGem) return;
    m_sonicGemEnabled[SGT_COUNT] = !ignoreNone;

    // Do another sanity check if any gems are enabled
    bool enabled = false;
    for (auto const& iter : m_sonicGemEnabled)
    {
        if (iter.second)
        {
            enabled = true;
            break;
        }
    }
    if (!enabled)
    {
        m_sceneGem->SetHideFlag(true);
        return;
    }

    // Scroll gems
    do
    {
        if (toRight)
        {
            m_sonicGemType = (SonicGemType)((int)m_sonicGemType + 1);
            if (m_sonicGemType > SGT_COUNT)
            {
                m_sonicGemType = (SonicGemType)0;
            }
        }
        else
        {
            m_sonicGemType = (SonicGemType)((int)m_sonicGemType - 1);
            if (m_sonicGemType < (SonicGemType)0)
            {
                m_sonicGemType = SGT_COUNT;
            }
        }
    } while (m_sonicGemEnabled[m_sonicGemType] == false);

    if (m_sonicGemType == SGT_COUNT)
    {
        m_sceneGem->SetHideFlag(true);
        return;
    }

    m_sceneGem->SetHideFlag(false);
    int patternIndex = 0;
    switch (m_sonicGemType)
    {
    case SGT_Blue:   patternIndex = 2; break;
    case SGT_Red:    patternIndex = 1; break;
    case SGT_Green:  patternIndex = 0; break;
    case SGT_Purple: patternIndex = 5; break;
    case SGT_Sky:    patternIndex = 3; break;
    case SGT_White:  patternIndex = 6; break;
    case SGT_Yellow: patternIndex = 4; break;
    default: return;
    }

    m_sceneGem->GetNode("custom_gem")->SetPatternIndex(patternIndex);
}

void CustomHUD::RestartSonicGem()
{
    // Reset Gem type when restarting stage/dying
    m_sonicGemType = SGT_COUNT;
    if (m_sceneGem)
    {
        m_sceneGem->SetHideFlag(true);
    }

    // Scroll to the first available gem
    if (m_sonicGemEnabled[m_sonicGemType] == false)
    {
        ScrollSonicGem(true, true);
    }
}

void CustomHUD::SetShadowChaosLevel(uint8_t level, float maturity)
{
    if (!m_sceneCustomBar || !m_sceneCustomLevel)
    {
        return;
    }
    
    m_sceneCustomLevel->GetNode("custom_level")->SetPatternIndex(min(3, level));
    m_sceneCustomBar->SetMotionFrame(min(100.0f, maturity));
}

HOOK(int, __fastcall, CustomHUD_MsgRestartStage, 0x1096A40, uint32_t* This, void* Edx, void* message)
{
    CustomHUD::RestartSonicGem();
    CustomHUD::SetShadowChaosLevel(0, 0.0f);
    if (m_sceneInfo)
    {
        m_sceneInfo->SetHideFlag(true);
    }

    return originalCustomHUD_MsgRestartStage(This, Edx, message);
}

int iconIndex = 0;
HOOK(void, __fastcall, CustomHUD_CHudSonicStageInit, 0x109A8D0, Sonic::CGameObject* This)
{
    CustomHUD::m_scoreEnabled = false;
    originalCustomHUD_CHudSonicStageInit(This);
    CustomHUD::CHudSonicStageRemoveCallback(This, nullptr, nullptr);

    std::string playScreenName = "maindisplay";
    if (Configuration::m_uiColor)
    {
        switch (S06DE_API::GetModelType())
        {
        case S06DE_API::ModelType::Shadow: playScreenName = "maindisplay_sh"; break;
        }
    }

    Sonic::CCsdDatabaseWrapper wrapper(This->m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());
    m_projectPlayScreen = wrapper.GetCsdProject(playScreenName.c_str())->m_rcProject;
    m_projectCountdown = wrapper.GetCsdProject("time")->m_rcProject;

    size_t& flags = ((size_t*)This)[151];

    // Change character life icon, overrided by S06DE
    iconIndex = Configuration::m_characterIcon;
    switch (S06DE_API::GetModelType())
    {
    case S06DE_API::ModelType::Sonic:       iconIndex = 0; break;
    case S06DE_API::ModelType::SonicElise:  iconIndex = 0; break;
    case S06DE_API::ModelType::Blaze:       iconIndex = 8; break;
    case S06DE_API::ModelType::Shadow:      iconIndex = 1; break;
    }

    if (flags & 0x1 || (Common::IsCurrentStageMission() && (flags & 0x8) == 0)) // Life
    {
        m_sceneLifeCount = m_projectPlayScreen->CreateScene("life");
        m_sceneLifeCount->SetMotionFrame(m_sceneLifeCount->m_MotionEndFrame);
        m_sceneLifeCount->m_MotionSpeed = 1.0f;
        m_sceneLifeCount->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
        m_sceneLifeCount->GetNode("character_icon")->SetPatternIndex(iconIndex);

        m_sceneLifeBG = m_projectPlayScreen->CreateScene("life_ber_anime");
        if (Configuration::m_uiColor)
        {
            if (iconIndex == 1)
            {
                m_sceneLifeBG->SetMotion("shadow_in");
            }
            else if (iconIndex == 2)
            {
                m_sceneLifeBG->SetMotion("silver_in");
            }
        }
        m_sceneLifeBG->SetMotionFrame(m_sceneLifeBG->m_MotionEndFrame);
        m_sceneLifeBG->m_MotionSpeed = 0.0f;
    }

    if (flags & 0x2) // Time
    {
        m_sceneTimeCount = m_projectPlayScreen->CreateScene("time");
        m_sceneTimeCount->m_MotionSpeed = 0.0f;
        if (Configuration::m_uiColor && iconIndex <= 2)
        {
            m_sceneTimeCount->m_MotionFrame = (float)iconIndex;
        }
    }

    if (flags & 0x4 || flags & 0x400000) // Ring
    {
        m_sceneRingCount = m_projectPlayScreen->CreateScene("ring");
        m_sceneRingCount->m_MotionSpeed = 0.0f; 
        if (Configuration::m_uiColor && iconIndex <= 2)
        {
            m_sceneRingCount->m_MotionFrame = (float)iconIndex;
        }

        m_sceneRingIcon = m_projectPlayScreen->CreateScene("ring_anime");
        m_sceneRingIcon->SetMotionFrame(m_sceneRingIcon->m_MotionEndFrame);
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
        if (Configuration::m_uiColor && iconIndex <= 2)
        {
            m_scenePowerBG->m_MotionFrame = (float)iconIndex;
        }

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

    // info_custom, info_custom_wisp, info_custom_chao, pin_medal, final boss
    if (flags & 0x40 || flags & 0x80 || flags & 0x100 || flags & 0x10000 || (Common::GetCurrentStageID() & 0xFF) == SMT_blb)
    {
        m_sceneInfo = m_projectPlayScreen->CreateScene("life_ber_anime");
        if (Configuration::m_uiColor)
        {
            if (iconIndex == 1)
            {
                m_sceneInfo->SetMotion("shadow_in");
            }
            else if (iconIndex == 2)
            {
                m_sceneInfo->SetMotion("silver_in");
            }
        }
        m_sceneInfo->SetPosition(0.0f, (flags & 0x10000) ? -100.0f : 50.0f);

        // Don't play animation immediately for info_custom and info_custom_wisp
        m_sceneInfo->m_MotionSpeed = 0.0f;
        if (flags & 0x100 || flags & 0x10000)
        {
            m_sceneInfo->SetMotionFrame(m_sceneInfo->m_MotionEndFrame);
        }
    }

    if (CustomHUD::m_scoreEnabled) // score
    {
        m_sceneScore = m_projectPlayScreen->CreateScene("score");
        m_sceneScore->m_MotionSpeed = 0.0f;
        if (Configuration::m_uiColor && iconIndex <= 2)
        {
            m_sceneScore->m_MotionFrame = (float)iconIndex;
        }
    }

    if (S06DE_API::GetModelType() == S06DE_API::ModelType::Sonic)
    {
        bool enabled = false;
        for (auto const& iter : CustomHUD::m_sonicGemEnabled)
        {
            if (iter.first != CustomHUD::SonicGemType::SGT_COUNT && iter.second)
            {
                enabled = true;
                break;
            }
        }

        if (enabled)
        {
            m_sceneGem = m_projectPlayScreen->CreateScene("custom_gem");
            CustomHUD::RestartSonicGem();
        }
    }
    else if (S06DE_API::GetModelType() == S06DE_API::ModelType::Shadow)
    {
        m_sceneCustomBar = m_projectPlayScreen->CreateScene("custom_bar_anime");
        m_sceneCustomBar->m_MotionSpeed = 0.0f;

        m_sceneCustomLevel = m_projectPlayScreen->CreateScene("custom_level");
        m_sceneCustomLevel->m_MotionSpeed = 0.0f;
        m_sceneCustomLevel->GetNode("custom_level")->SetPatternIndex(0);
    }

    if (Common::IsCurrentStageBoss() && (Common::GetCurrentStageID() & 0xFF) != SMT_bsd)
    {
        m_sceneBossBG = m_projectPlayScreen->CreateScene("boss_gauge");
        m_sceneBossBar = m_projectPlayScreen->CreateScene("boss_gauge_anime");
        m_sceneBossBar->m_MotionSpeed = 0.0f;
    }

    // Mask to prevent crash when game tries accessing the elements we disabled later on
    flags &= ~(0x1 | 0x2 | 0x4 | 0x200 | 0x800 | 0x400000);
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

    CustomHUD::ToggleScreen(m_projectPlayScreen, m_spPlayScreen, "HUD_B1", false, This);
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
            sprintf(text, "%d", min(99, max(0, *(int*)liveCountAddr)));
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
                *(uint32_t*)((uint32_t)This + 768) = playerContext->m_RingCount;
                *(uint32_t*)((uint32_t)This + 764) = 1;
            }
        }
    }

    if (m_scenePower && playerContext)
    {
        float boost = min(100.0f, *Common::GetPlayerBoost());
        m_scenePower->SetMotionFrame(boost);

        if (boost < 100.0f)
        {
            m_scenePowerEffect->SetHideFlag(true);
            m_scenePowerEffect->SetMotionFrame(0.0f);
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
            m_sceneCountdown->SetMotionFrame(0.0f);
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
        m_sceneScore->Update();
    }

    originalCustomHUD_MsgSetPinballHud(This, Edx, message);
}

HOOK(void, __fastcall, CustomHUD_MsgNotifySonicHud, 0x1097400, Sonic::CGameObject* This, void* Edx, void* message)
{
    // Ring collect animation
    if (m_sceneRingIcon && m_sceneRingIcon->m_MotionDisableFlag)
    {
        m_sceneRingIcon->SetMotionFrame(0.0f);
        m_sceneRingIcon->m_MotionDisableFlag = 0;
        m_sceneRingIcon->Update();
    }

    originalCustomHUD_MsgNotifySonicHud(This, Edx, message);
}

void CustomHUD::PlayInfoHUD(bool intro, bool instantStart)
{
    if (!m_sceneInfo) return;

    m_sceneInfo->SetMotion(intro ? "sonic_in" : "sonic_out");
    if (Configuration::m_uiColor)
    {
        if (iconIndex == 1)
        {
            m_sceneInfo->SetMotion(intro ? "shadow_in" : "shadow_out");
        }
        else if (iconIndex == 2)
        {
            m_sceneInfo->SetMotion(intro ? "silver_in" : "silver_out");
        }
    }
    m_sceneInfo->SetHideFlag(false);
    m_sceneInfo->SetMotionFrame(instantStart ? 0.0f : (intro ? -5.0f : -10.0f));
    m_sceneInfo->m_MotionSpeed = 1.0f;
    m_sceneInfo->m_MotionDisableFlag = false;
    m_sceneInfo->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
    m_sceneInfo->Update();
}

HOOK(void, __fastcall, CustomHUD_MsgChangeWispHud, 0x1096050, uint32_t* This, void* Edx, uint32_t* message)
{
    int spriteCount = This[143];
    if (spriteCount == 0)
    {
        int type = message[4];
        bool isSpike = message[5];
        switch (type)
        {
        case 0: // get_wisp
            CustomHUD::PlayInfoHUD(true, true);
            break;
        case 1: // use_wisp
            if (!isSpike)
                CustomHUD::PlayInfoHUD(false, true);
            break;
        case 2: // release_wisp_spike
        case 3: // outro
            CustomHUD::PlayInfoHUD(false, true);
            break;
        }
    }

    originalCustomHUD_MsgChangeWispHud(This, Edx, message);
}

HOOK(void, __fastcall, CustomHUD_MsgChangeCustomHud, 0x1096FF0, uint32_t* This, void* Edx, uint32_t* message)
{
    bool hasWisp = This[145];
    if (m_sceneInfo && *(uint32_t*)((uint32_t)This + 0x1CC) && !hasWisp)
    {
        int spriteIndex = message[4];
        int spriteCount = message[5];
        int spriteCountPrev = This[143];

        if (spriteCount > 0)
        {
            if (*((bool*)This + 580) || spriteCount > spriteCountPrev)
            {
                CustomHUD::PlayInfoHUD(true, false);
            }
        }
        else
        {
            // Run out of info_custom or info_custom_chao
            if (spriteCountPrev > 0)
            {
                CustomHUD::PlayInfoHUD(false, false);
            }
        }
    }

    originalCustomHUD_MsgChangeCustomHud(This, Edx, message);
}

HOOK(void, __fastcall, CustomHUD_MsgGetMissionLimitTime, 0xD0F0E0, Sonic::CGameObject* This, void* Edx, hh::fnd::Message& in_rMsg)
{
    originalCustomHUD_MsgGetMissionLimitTime(This, Edx, in_rMsg);
    CustomHUD::m_missionMaxTime = *(float*)((char*)&in_rMsg + 16);
}

HOOK(void, __fastcall, CustomHUD_GpSonicSafeCLastBossGaugeNew, 0x124EA10, void* This)
{
    CustomHUD::PlayInfoHUD(true, false);
    originalCustomHUD_GpSonicSafeCLastBossGaugeNew(This);
}

HOOK(bool, __fastcall, CustomHUD_CLastBossCaptionCreationCallback, 0xB16860, uint32_t This, void* Edx, int a2, int a3, int a4)
{
    bool result = originalCustomHUD_CLastBossCaptionCreationCallback(This, Edx, a2, a3, a4);

    Chao::CSD::RCPtr<Chao::CSD::CScene>* captionScene = (Chao::CSD::RCPtr<Chao::CSD::CScene>*)(This + 280);
    if (captionScene->Get())
    {
        captionScene->Get()->GetNode("Null_bg")->SetHideFlag(true);
    }

    return result;
}

//---------------------------------------------------
// Pause
//---------------------------------------------------
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectPause;
boost::shared_ptr<Sonic::CGameObjectCSD> m_spPause;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_scenePauseMenu;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_scenePauseCursor;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_scenePauseText;

int CustomHUD::m_cursorPos = 0;
bool CustomHUD::m_isPamPause = false;
bool CustomHUD::m_canRestart = true;
char const* cursorAnimations[] =
{
    "select_01",
    "select_02",
    "select_02", // dummy for Controls
    "select_03",
};
char const* cursorLoopAnimations[] =
{
    "select_01_loop",
    "select02_loop",
    "select02_loop", // dummy for Controls
    "select03_loop",
};
char const* cursorPamAnimations[] =
{
    "select_01",
    "select_03",
};
char const* cursorPamLoopAnimations[] =
{
    "select_01_loop",
    "select03_loop",
};

void __fastcall CustomHUD::CPauseRemoveCallback(Sonic::CGameObject* This, void*, Sonic::CGameDocument* pGameDocument)
{
    KillScreen(m_spPause);

    Chao::CSD::CProject::DestroyScene(m_projectPause.Get(), m_scenePauseMenu);
    Chao::CSD::CProject::DestroyScene(m_projectPause.Get(), m_scenePauseCursor);
    Chao::CSD::CProject::DestroyScene(m_projectPause.Get(), m_scenePauseText);

    m_projectPause = nullptr;
}

void CustomHUD::CreatePauseScreen(uint32_t* This, bool isPamPause)
{
    Sonic::CGameObject* gameObject = (Sonic::CGameObject*)This[1];
    CustomHUD::CPauseRemoveCallback(gameObject, nullptr, nullptr);
    CustomHUD::m_isPamPause = isPamPause;

    Sonic::CCsdDatabaseWrapper wrapper(gameObject->m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());
    m_projectPause = wrapper.GetCsdProject("pausemenu")->m_rcProject;

    m_scenePauseMenu = m_projectPause->CreateScene("pause_menu");
    m_scenePauseMenu->SetHideFlag(true);
    m_scenePauseMenu->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;

    m_scenePauseCursor = m_projectPause->CreateScene("pause_menu_cursor");
    m_scenePauseCursor->SetHideFlag(true);

    bool isJapanese = Common::GetUILanguageType() == LT_Japanese;
    m_scenePauseText = m_projectPause->CreateScene("text");
    m_scenePauseText->SetHideFlag(true);
    m_scenePauseText->GetNode("text1")->SetPatternIndex(isJapanese);
    m_scenePauseText->GetNode("text2")->SetPatternIndex(isJapanese * 2);
    m_scenePauseText->GetNode("text3")->SetPatternIndex(isJapanese);

    CustomHUD::CreateScreen(m_projectPause, m_spPause, "HUD_Pause", true, gameObject);
}

void CustomHUD::OpenPauseScreen()
{
    m_scenePauseMenu->SetHideFlag(false);
    m_scenePauseMenu->SetMotion("pause3");
    m_scenePauseMenu->SetMotionFrame(0.0f);
    m_scenePauseMenu->m_MotionDisableFlag = false;
    m_scenePauseMenu->m_MotionSpeed = 1.0f;
    m_scenePauseMenu->Update();

    m_scenePauseText->SetHideFlag(false);
    size_t* liveCountAddr = (size_t*)Common::GetMultiLevelAddress(0x1E66B34, { 0x4, 0x1B4, 0x7C, 0x9FDC });
    if (liveCountAddr)
    {
        CustomHUD::m_canRestart = (*liveCountAddr != 0) && !CustomHUD::m_isPamPause;
        bool isJapanese = Common::GetUILanguageType() == LT_Japanese;
        if (CustomHUD::m_canRestart)
        {
            m_scenePauseText->GetNode("text2")->SetPatternIndex(isJapanese * 2);
        }
        else
        {
            m_scenePauseText->GetNode("text2")->SetPatternIndex(isJapanese * 2 + 1);
        }
    }
}

void CustomHUD::ClosePauseScreen()
{
    m_scenePauseMenu->m_MotionDisableFlag = false;
    m_scenePauseMenu->m_PrevMotionFrame = m_scenePauseMenu->m_MotionEndFrame;
    m_scenePauseMenu->m_MotionFrame = m_scenePauseMenu->m_MotionEndFrame;
    *(uint32_t*)((uint32_t)m_scenePauseMenu.Get() + 0xB0) = 0;
    *(uint32_t*)((uint32_t)m_scenePauseMenu.Get() + 0xB4) = 0; // this stops the reverse animation
    m_scenePauseMenu->m_MotionSpeed = -1.0f;
    m_scenePauseMenu->Update();

    m_scenePauseCursor->SetHideFlag(true);
    m_scenePauseText->SetHideFlag(true);
}

void CustomHUD::RefreshPauseCursor()
{
    m_scenePauseCursor->SetHideFlag(false);
    m_scenePauseCursor->SetMotion(CustomHUD::m_isPamPause ? cursorPamAnimations[CustomHUD::m_cursorPos] : cursorAnimations[CustomHUD::m_cursorPos]);
    m_scenePauseCursor->SetMotionFrame(0.0f);
    m_scenePauseCursor->m_MotionDisableFlag = false;
    m_scenePauseCursor->m_MotionSpeed = 1.0f;
    m_scenePauseCursor->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
    m_scenePauseCursor->Update();
}

HOOK(int*, __fastcall, CustomHUD_CPauseUpdate, 0x10A18D0, void* This, void* Edx, Hedgehog::Universe::SUpdateInfo* a2)
{
    if (m_scenePauseMenu)
    {
        // Hide when pause is finished
        if (m_scenePauseMenu->m_MotionSpeed == -1.0f && m_scenePauseMenu->m_MotionFrame <= 0.0f)
        {
            m_scenePauseMenu->SetHideFlag(true);
        }
    }

    if (m_scenePauseCursor)
    {
        if (m_scenePauseCursor->m_MotionRepeatType == Chao::CSD::eMotionRepeatType_PlayOnce && m_scenePauseCursor->m_MotionDisableFlag)
        {
            m_scenePauseCursor->SetMotion(CustomHUD::m_isPamPause ? cursorPamLoopAnimations[CustomHUD::m_cursorPos] : cursorLoopAnimations[CustomHUD::m_cursorPos]);
            m_scenePauseCursor->SetMotionFrame(0.0f);
            m_scenePauseCursor->m_MotionDisableFlag = false;
            m_scenePauseCursor->m_MotionSpeed = 1.0f;
            m_scenePauseCursor->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_Loop;
            m_scenePauseCursor->Update();
        }
    }

    return originalCustomHUD_CPauseUpdate(This, Edx, a2);
}

HOOK(bool, __fastcall, CustomHUD_CPauseVisualActInit, 0x109FAD0, uint32_t* This)
{
    CustomHUD::CreatePauseScreen(This, false);
    return originalCustomHUD_CPauseVisualActInit(This);;
}

HOOK(bool, __fastcall, CustomHUD_CPauseVisualActOpened, 0x109F8F0, uint32_t* This)
{
    return m_scenePauseMenu->m_MotionDisableFlag == 1;
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
        CustomHUD::OpenPauseScreen();
        break;
    }
    case 1: // End
    {
        CustomHUD::ClosePauseScreen();
        break;
    }
    case 2: // Start animation complete
    {
        CustomHUD::RefreshPauseCursor();
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
        CustomHUD::RefreshPauseCursor();
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

HOOK(bool, __fastcall, CustomHUD_CPauseVisualPamInit, 0x109E9D0, uint32_t* This)
{
    CustomHUD::CreatePauseScreen(This, true);
    return originalCustomHUD_CPauseVisualPamInit(This);;
}

HOOK(bool, __fastcall, CustomHUD_CPauseVisualPamOpened, 0x109E720, uint32_t* This)
{
    return m_scenePauseMenu->m_MotionDisableFlag == 1;
}

HOOK(int, __fastcall, CustomHUD_CPauseVisualPamCase, 0x109E740, uint32_t* This, void* Edx, int Case)
{
    CustomHUD::m_cursorPos = This[3];
    switch (Case)
    {
    case 0: // Start
    case 5: // Back from confirm dialog
    {
        CustomHUD::OpenPauseScreen();
        break;
    }
    case 1: // End
    {
        CustomHUD::ClosePauseScreen();
        break;
    }
    case 2: // Start animation complete
    case 3: // New cursor pos
    {
        CustomHUD::RefreshPauseCursor();
        break;
    }
    case 4: // Confirm
    {
        break;
    }
    default: break;
    }

    return originalCustomHUD_CPauseVisualPamCase(This, Edx, Case);
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

void __declspec(naked) CustomHUD_PreventRestart()
{
    static uint32_t failedAddress = 0x42A635;
    static uint32_t returnAddress = 0x42A5B3;
    static uint32_t sub_11D1BF0 = 0x11D1BF0; // NG sfx
    __asm
    {
        cmp     CustomHUD::m_cursorPos, 1
        jne     jump

        cmp     CustomHUD::m_isPamPause, 0
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

//---------------------------------------------------
// Yes & No Window
//---------------------------------------------------
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectYesNo;
boost::shared_ptr<Sonic::CGameObjectCSD> m_spYesNo;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneYesNoTop;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneYesNoBottom;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneYesNoCursor;

int CustomHUD::m_yesNoCursorPos = 0;
float CustomHUD::m_yesNoColorTime = 0.0f;
std::string CustomHUD::m_yesNoWindowText;

void __fastcall CustomHUD::CWindowImplRemoveCallback(Sonic::CGameObject* This, void*, Sonic::CGameDocument* pGameDocument)
{
    CustomHUD::m_yesNoColorTime = 0.0f;
    CustomHUD::m_yesNoWindowText.clear();

    KillScreen(m_spYesNo);

    Chao::CSD::CProject::DestroyScene(m_projectYesNo.Get(), m_sceneYesNoTop);
    Chao::CSD::CProject::DestroyScene(m_projectYesNo.Get(), m_sceneYesNoBottom);
    Chao::CSD::CProject::DestroyScene(m_projectYesNo.Get(), m_sceneYesNoCursor);

    m_projectYesNo = nullptr;
}

void CustomHUD::RefreshYesNoCursor()
{
    if (m_sceneYesNoCursor)
    {
        m_sceneYesNoCursor->SetPosition(-375.0f, 148.0f + m_yesNoCursorPos * 42.7f);
    }
}

HOOK(int, __fastcall, CustomHUD_CWindowImplCState_Open, 0x439120, hh::fnd::CStateMachineBase::CStateBase* This)
{
    if (!CustomHUD::m_yesNoWindowText.empty())
    {
        uint32_t owner = (uint32_t)(This->GetContextBase());
        Chao::CSD::RCPtr<Chao::CSD::CScene> scene = *(Chao::CSD::RCPtr<Chao::CSD::CScene>*)(owner + 936);
        scene->SetPosition(0.0f, 800.0f);

        // Don't play menu open sfx
        WRITE_NOP(0x43921B, 5);
    }

    int result = originalCustomHUD_CWindowImplCState_Open(This);

    // Restore sfx
    WRITE_MEMORY(0x43921B, uint8_t, 0xE8, 0x50, 0x8F, 0xD9, 0x00);

    return result;
}

HOOK(void, __fastcall, CustomHUD_CPauseCStateWindowBegin, 0x42ABA0, hh::fnd::CStateMachineBase::CStateBase* This)
{
    Sonic::CGameObject* parent = (Sonic::CGameObject*)(This->GetContextBase());
    CustomHUD::CWindowImplRemoveCallback(parent, nullptr, nullptr);

    bool isJapanese = Common::GetUILanguageType() == LT_Japanese;
    if (CustomHUD::m_cursorPos == 1)
    {
        if (CustomHUD::m_isPamPause)
        {
            CustomHUD::m_yesNoWindowText = isJapanese
                ? u8"ゲームを終了して、タイトルに戻ります\n最後にセーブしたところから\nここまでの進行は保存されませんが\nそれでもよろしいですか？"
                : "Exit  the  game.\nThe  progress  of  the  game  from  the  last  saved\npoint  will  not  be  saved.  OK?";
        }
        else
        {
            CustomHUD::m_yesNoWindowText = isJapanese
                ? u8"ステージの始めからプレイしなおします\n最後にセーブしたところから\nここまでの進行は保存されませんが\nそれでもよろしいですか？"
                : "Restart  from  the  beginning  of  the  stage.\nThe  progress  of  the  game  from  the  last  saved\npoint  will  not  be  saved.  OK?";
        }
    }
    else if (CustomHUD::m_cursorPos == 3)
    {
        CustomHUD::m_yesNoWindowText = isJapanese
            ? u8"ステージを終了します\n最後にセーブしたところから\nここまでの進行は保存されませんが\nそれでもよろしいですか？"
            : "Exit  the  stage.\nThe  progress  of  the  game  from  the  last  saved\npoint  will  not  be  saved.  OK?";
    }

    originalCustomHUD_CPauseCStateWindowBegin(This);

    Sonic::CCsdDatabaseWrapper wrapper(parent->m_pMember->m_pGameDocument->m_pMember->m_spDatabase.get());
    m_projectYesNo = wrapper.GetCsdProject("windowtest")->m_rcProject;

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

    m_sceneYesNoCursor = m_projectYesNo->CreateScene("cursor");
    m_sceneYesNoCursor->SetMotion("cursor_select");
    m_sceneYesNoCursor->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_PlayOnce;
    CustomHUD::RefreshYesNoCursor();

    CustomHUD::CreateScreen(m_projectYesNo, m_spYesNo, "HUD_Pause", true, parent);
}

HOOK(int*, __fastcall, CustomHUD_CPauseCStateWindowAdvance, 0x42AEE0, hh::fnd::CStateMachineBase::CStateBase* This)
{
    if (m_sceneYesNoCursor)
    {
        if (m_sceneYesNoCursor->m_MotionRepeatType == Chao::CSD::eMotionRepeatType_PlayOnce && m_sceneYesNoCursor->m_MotionDisableFlag)
        {
            m_sceneYesNoCursor->SetMotion("cursor_set");
            m_sceneYesNoCursor->SetMotionFrame(0.0f);
            m_sceneYesNoCursor->m_MotionDisableFlag = false;
            m_sceneYesNoCursor->m_MotionSpeed = 1.0f;
            m_sceneYesNoCursor->m_MotionRepeatType = Chao::CSD::eMotionRepeatType_Loop;
            m_sceneYesNoCursor->Update();
        }
    }

    return originalCustomHUD_CPauseCStateWindowAdvance(This);
}

HOOK(void, __fastcall, CustomHUD_CPauseCStateWindowEnd, 0x42AED0, hh::fnd::CStateMachineBase::CStateBase* This)
{
    originalCustomHUD_CPauseCStateWindowEnd(This);

    Sonic::CGameObject* parent = (Sonic::CGameObject*)(This->GetContextBase());
    CustomHUD::CWindowImplRemoveCallback(parent, nullptr, nullptr);
}

HOOK(bool, __fastcall, CustomHUD_CWindowImplSetCursor, 0x438500, void* This, void* Edx, uint32_t pos)
{
    CustomHUD::m_yesNoCursorPos = pos;
    CustomHUD::m_yesNoColorTime = 0.0f;
    CustomHUD::RefreshYesNoCursor();

    return originalCustomHUD_CWindowImplSetCursor(This, Edx, pos);
}

//---------------------------------------------------
// Boss Health
//---------------------------------------------------
void CustomHUD_BossSetHealth(float health, float maxHealth)
{
    if (m_sceneBossBar)
    {
        float motionFrame = 100.0f - health * 100.0f / maxHealth;
        Common::ClampFloat(motionFrame, 0.0f, 100.0f);
        m_sceneBossBar->SetMotionFrame(motionFrame);
    }
}

void __declspec(naked) CustomHUD_CRivalMetalSonicLastHit()
{
    static uint32_t returnAddress = 0xCC8B73;
    __asm
    {
        add     eax, 0FFFFFFFFh
        mov     [edi + 200h], eax
        fldz
        sub     esp, 8
        jmp     [returnAddress]
    }
}

int CustomHUD_MetalSonicMaxHealth = 0;
HOOK(bool, __fastcall, CustomHUD_CRivalMetalSonicProcessMessage, 0xCD2760, hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message, bool flag)
{
    bool result = originalCustomHUD_CRivalMetalSonicProcessMessage(This, Edx, message, flag);

    if (flag)
    {
        int health = ((int**)((uint32_t)This - 0x28))[104][128] + 2;

        if (std::strstr(message.GetType(), "MsgDummy") != nullptr)
        {
            CustomHUD_MetalSonicMaxHealth = health;
            printf("[CustomHUD] Metal Sonic max health = %d\n", CustomHUD_MetalSonicMaxHealth);
        }

        CustomHUD_BossSetHealth(health, CustomHUD_MetalSonicMaxHealth);
    }

    return result;
}

int CustomHUD_DeathEggMaxHealth = 0;
HOOK(int, __fastcall, CustomHUD_CBossDeathEggSetMaxHealth, 0xC46DA0, void* This, void* Edx, int a2, int a3)
{
    CustomHUD_DeathEggMaxHealth = *(int*)(*(int*)(a3 + 4) + 4);
    return originalCustomHUD_CBossDeathEggSetMaxHealth(This, Edx, a2, a3);
}

HOOK(bool, __fastcall, CustomHUD_CBossDeathEggProcessMessage, 0xC67350, hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message, bool flag)
{
    bool result = originalCustomHUD_CBossDeathEggProcessMessage(This, Edx, message, flag);

    if (flag)
    {
        int health = ((int*)((uint32_t)This - 0x28))[121];
        CustomHUD_BossSetHealth(health, CustomHUD_DeathEggMaxHealth);
    }

    return result;
}

HOOK(bool, __fastcall, CustomHUD_CBossPerfectChaosProcessMessage, 0xC122D0, hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message, bool flag)
{
    bool result = originalCustomHUD_CBossPerfectChaosProcessMessage(This, Edx, message, flag);

    if (flag)
    {
        int* pGameObject = (int*)((uint32_t)This - 0x28);
        int health = pGameObject[121];
        bool finalHitInstance = *(bool*)((int)pGameObject + 500);

        // Only handles the first two hits
        if (!finalHitInstance && health > 0)
        {
            CustomHUD_BossSetHealth(health + 1, 4);
        }
    }

    return result;
}

HOOK(bool, __fastcall, CustomHUD_CBossPerfectChaosCStateDamageToFinalAttack, 0x5D1B10, void* This)
{
    CustomHUD_BossSetHealth(1, 4);
    return originalCustomHUD_CBossPerfectChaosCStateDamageToFinalAttack(This);
}

HOOK(void, __fastcall, CustomHUD_CBossPerfectChaosCStateDefeated, 0x5D1C70, void* This)
{
    CustomHUD_BossSetHealth(0, 4);
    originalCustomHUD_CBossPerfectChaosCStateDefeated(This);
}

HOOK(bool, __fastcall, CustomHUD_CBossEggDragoonProcessMessage, 0xC3F500, hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message, bool flag)
{
    bool result = originalCustomHUD_CBossEggDragoonProcessMessage(This, Edx, message, flag);

    if (flag)
    {
        int* pGameObject = (int*)((uint32_t)This - 0x28);
        int maxHealth = pGameObject[121];
        int headHealth = pGameObject[124];
        int bellyHealth = pGameObject[125];
        CustomHUD_BossSetHealth(headHealth + bellyHealth, maxHealth);
    }

    return result;
}

int CustomHUD_SilverMaxHealth = 0;
HOOK(bool, __fastcall, CustomHUD_CRivalSilverProcessMessage, 0xC8B8F0, hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message, bool flag)
{
    bool result = originalCustomHUD_CRivalSilverProcessMessage(This, Edx, message, flag);

    if (flag)
    {
        int health = ((int**)((uint32_t)This - 0x28))[104][104] + 2;

        if (std::strstr(message.GetType(), "MsgDummy") != nullptr)
        {
            CustomHUD_SilverMaxHealth = health;
            printf("[CustomHUD] Silver max health = %d\n", CustomHUD_SilverMaxHealth);
        }

        CustomHUD_BossSetHealth(health, CustomHUD_SilverMaxHealth);
    }

    return result;
}

int CustomHUD_TimeEaterMaxHealth = 0;
HOOK(bool, __fastcall, CustomHUD_CBossTimeEaterProcessMessage, 0xBFF390, hh::fnd::CMessageActor* This, void* Edx, hh::fnd::Message& message, bool flag)
{
    bool result = originalCustomHUD_CBossTimeEaterProcessMessage(This, Edx, message, flag);

    if (flag)
    {
        int health = ((int*)((uint32_t)This - 0x28))[121] + 1;

        if (std::strstr(message.GetType(), "MsgDummy") != nullptr)
        {
            CustomHUD_TimeEaterMaxHealth = health;
            printf("[CustomHUD] Time Eater max health = %d\n", CustomHUD_TimeEaterMaxHealth);
        }

        CustomHUD_BossSetHealth(health, CustomHUD_TimeEaterMaxHealth);
    }

    return result;
}

void CustomHUD::applyPatches()
{
    //---------------------------------------------------
    // Main Gameplay
    //---------------------------------------------------
    WRITE_MEMORY(0x16A467C, void*, CHudSonicStageRemoveCallback);
    INSTALL_HOOK(CustomHUD_CHudSonicStageInit);
    INSTALL_HOOK(CustomHUD_CHudSonicStageUpdate);
    INSTALL_HOOK(CustomHUD_MsgSetPinballHud);
    INSTALL_HOOK(CustomHUD_MsgNotifySonicHud);
    INSTALL_HOOK(CustomHUD_MsgChangeCustomHud);
    INSTALL_HOOK(CustomHUD_MsgChangeWispHud);
    INSTALL_HOOK(CustomHUD_MsgGetMissionLimitTime);
    INSTALL_HOOK(CustomHUD_MsgRestartStage);

    // Disable original HUD
    WRITE_JUMP(0x109B1AA, (void*)0x109B485); // life init
    WRITE_JUMP(0x109B496, (void*)0x109B5A1); // time init
    WRITE_JUMP(0x109B5B3, (void*)0x109B69B); // ring init
    WRITE_JUMP(0x109B8FB, (void*)0x109BA9D); // boost gauge init
    WRITE_JUMP(0x109BC8E, (void*)0x109BDF2); // boost button init
    WRITE_JUMP(0x109C1DC, CustomHUD_GetScoreEnabled); // score init
    //WRITE_JUMP(0x109BEF6, (void*)0x109C05A); // countdown init

    // Final Boss
    INSTALL_HOOK(CustomHUD_GpSonicSafeCLastBossGaugeNew);
    INSTALL_HOOK(CustomHUD_CLastBossCaptionCreationCallback);

    // Don't hide HUD at pause, don't run MsgAppearActStageHud
    WRITE_MEMORY(0x10BC141, uint8_t, 0xEB);
    WRITE_JUMP(0x10A1596, (void*)0x10A168E);

    //---------------------------------------------------
    // Pause
    //---------------------------------------------------
    INSTALL_HOOK(CustomHUD_CPauseUpdate);

    WRITE_MEMORY(0x16A41A4, void*, CPauseRemoveCallback);
    INSTALL_HOOK(CustomHUD_CPauseVisualActInit);
    INSTALL_HOOK(CustomHUD_CPauseVisualActOpened);
    INSTALL_HOOK(CustomHUD_CPauseVisualActCase);

    INSTALL_HOOK(CustomHUD_CPauseVisualPamInit);
    INSTALL_HOOK(CustomHUD_CPauseVisualPamOpened);
    INSTALL_HOOK(CustomHUD_CPauseVisualPamCase);

    INSTALL_HOOK(CustomHUD_CPauseCStateSelectAdvance);
    INSTALL_HOOK(CustomHUD_CPauseCStateCloseBegin);
    INSTALL_HOOK(CustomHUD_CGameplayFlowStageCStateStageUpdating);

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
    WRITE_MEMORY(0x11D219A, uint32_t, 1000002); // Menu oppen
    WRITE_MEMORY(0x11D20EA, uint32_t, 1000003); // Menu close
    WRITE_MEMORY(0x11D1D7A, uint32_t, 1000003); // Cancel
    WRITE_MEMORY(0x11D1E2A, uint32_t, 1000016); // Stage scroll

    // Prevent restarting when no life
    WRITE_JUMP(0x42A5AB, CustomHUD_PreventRestart);

    // Use stage scroll stx for pause scroll
    WRITE_MEMORY(0x42A630, uint8_t, 0xE8, 0xCB, 0x77, 0xDA, 0x00);

    //---------------------------------------------------
    // Yes & No Window
    //---------------------------------------------------
    INSTALL_HOOK(CustomHUD_CPauseCStateWindowBegin);
    INSTALL_HOOK(CustomHUD_CPauseCStateWindowAdvance);
    INSTALL_HOOK(CustomHUD_CPauseCStateWindowEnd);

    INSTALL_HOOK(CustomHUD_CWindowImplSetCursor);
    INSTALL_HOOK(CustomHUD_CWindowImplCState_Open);
    WRITE_MEMORY(0x16A6D84, void*, CWindowImplRemoveCallback);

    //---------------------------------------------------
    // Boss Health
    //---------------------------------------------------
    WRITE_JUMP(0xCC8B6E, CustomHUD_CRivalMetalSonicLastHit);
    INSTALL_HOOK(CustomHUD_CRivalMetalSonicProcessMessage);
    INSTALL_HOOK(CustomHUD_CBossDeathEggSetMaxHealth);
    INSTALL_HOOK(CustomHUD_CBossDeathEggProcessMessage);
    INSTALL_HOOK(CustomHUD_CBossPerfectChaosProcessMessage);
    INSTALL_HOOK(CustomHUD_CBossPerfectChaosCStateDamageToFinalAttack);
    INSTALL_HOOK(CustomHUD_CBossPerfectChaosCStateDefeated);
    INSTALL_HOOK(CustomHUD_CBossEggDragoonProcessMessage);
    INSTALL_HOOK(CustomHUD_CRivalSilverProcessMessage);
    INSTALL_HOOK(CustomHUD_CBossTimeEaterProcessMessage);
}

void CustomHUD::draw()
{
    if (!m_yesNoWindowText.empty())
    {
        static bool visible = true;
        char const* captionTitle = m_isPamPause ? "Caption0" : (m_cursorPos == 1 ? "Caption1" : "Caption2");
        ImGui::Begin(captionTitle, &visible, UIContext::m_hudFlags);
        {
            ImVec2 textSize = ImGui::CalcTextSize(m_yesNoWindowText.c_str());
            ImGui::SetWindowFocus(); 
            ImGui::SetWindowPos(ImVec2(0, 0));
            ImGui::SetCursorPos(ImVec2(*BACKBUFFER_WIDTH * 0.5f - textSize.x / 2.0f, *BACKBUFFER_HEIGHT * 0.4024f - textSize.y / 2.0f));
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), m_yesNoWindowText.c_str());
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
