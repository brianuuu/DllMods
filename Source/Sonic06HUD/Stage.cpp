#include "Stage.h"
#include "Application.h"
#include "Configuration.h"
#include "UIContext.h"

//---------------------------------------------------
// Fade transitions
//---------------------------------------------------
void __fastcall SetLoadingFadeIn(float startAlpha, float* a2)
{
    uint8_t* color = (uint8_t*)((uint32_t)a2 + 21);
    if (color[0] == 0 && color[1] == 0 && color[2] == 0)
    {
        a2[6] = startAlpha; // start alpha

        if (a2[6] > 0.0f)
        {
            a2[4] = 0.2f; // transition time

            // Unlock player control
            (*(uint32_t**)0x1E66B40)[2] = 0;
        }
    }
}

void __fastcall SetLoadingFadeOut(float* a2)
{
    UIContext::clearDraw();

    uint8_t* color = (uint8_t*)((uint32_t)a2 + 21);
    if (color[0] == 0 && color[1] == 0 && color[2] == 0)
    {
        a2[4] = 0.0f; // transition time
        a2[6] = 1.0f; // start alpha
    }
}

HOOK(float*, __fastcall, MsgFadeInFxp, 0x10CEC90, void* This, void* Edx, float* a2)
{
    SetLoadingFadeIn(((float*)This)[24], a2);
    return originalMsgFadeInFxp(This, Edx, a2);
}

HOOK(float*, __fastcall, MsgFadeOutFxp, 0x10CEDB0, void* This, void* Edx, float* a2)
{
    SetLoadingFadeOut(a2);
    return originalMsgFadeOutFxp(This, Edx, a2);
}

HOOK(float*, __fastcall, MsgFadeInMtfx, 0x57B290, void* This, void* Edx, float* a2)
{
    uint32_t actualThis = (uint32_t)This + 48 * *(uint32_t*)((uint32_t)a2 + 28) + 2064;
    SetLoadingFadeIn(((float*)actualThis)[1], a2);
    return originalMsgFadeInMtfx(This, Edx, a2);
}

HOOK(float*, __fastcall, MsgFadeOutMtfx, 0x57B270, void* This, void* Edx, float* a2)
{
    SetLoadingFadeOut(a2);
    return originalMsgFadeOutMtfx(This, Edx, a2);
}

HOOK(int*, __fastcall, Stage_CGameplayFlowStage_CStateBegin, 0xCFF2B0, void* This)
{
    UIContext::clearDraw();
    return originalStage_CGameplayFlowStage_CStateBegin(This);
}

HOOK(void, __fastcall, Stage_CGameplayFlowStage_CStateTitle, 0xCF8F40, void* This)
{
    UIContext::clearDraw();
    originalStage_CGameplayFlowStage_CStateTitle(This);
}

//---------------------------------------------------
// 3 digit milliseconds correction
//---------------------------------------------------
void __declspec(naked) Stage_GetCurrentMillisecond()
{
    static uint32_t returnAddress = 0x10B3847;
	__asm
	{
		imul    edx, 3E8h
		sub     ecx, edx
		jmp     [returnAddress]
	}
}

void __fastcall SetMissionTime(char* buffer, int totalSeconds)
{
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    sprintf(buffer, "%02d:%02d.", minutes, seconds);
}

void __declspec(naked) Stage_GetMissionCurrentTime()
{
    static uint32_t returnAddress = 0x124F098;
	__asm
	{
        mov     edx, esi
        call    SetMissionTime
        pop     esi
        sub     esp, 0Ch
        jmp     [returnAddress]
	}
}

void __declspec(naked) Stage_GetMissionNextRankMaxTime()
{
    static uint32_t returnAddress = 0x10B16D5;
	__asm
	{
        mov	    esi, 99
        mov     edx, 59
        mov     ecx, 999
        jmp     [returnAddress]
	}
}

uint32_t __fastcall CreateRandomThirdDigit(uint32_t milliseconds)
{
    return milliseconds * 10 + rand() % 10;
}

void __declspec(naked) Stage_GetMissionNextRankRandomDigit()
{
    static uint32_t returnAddress = 0x10B16F9;
	__asm
	{
        push    eax
        push    edx
        push    esi
        call    CreateRandomThirdDigit
        mov     ecx, eax
        pop     esi
        pop     edx
        pop     eax

        push    ecx
        push    edx
        push    esi
        lea     ecx, [esp + 3Ch]
        jmp     [returnAddress]
	}
}

//---------------------------------------------------
// Checkpoint
//---------------------------------------------------
std::string Stage::m_lapTimeStr;
float Stage::m_checkpointTimer = 0.0f;
HOOK(void, __fastcall, Stage_MsgNotifyLapTimeHud, 0x1097640, void* This, void* Edx, uint32_t a2)
{
    float lapTime = *(float*)(a2 + 20);
    int minute = (int)(lapTime / 60.0f);
    int seconds = (int)(lapTime - 60.0f * (float)minute);
    int milliseconds = (int)(lapTime * 1000.0f) % 1000;

    char buffer[20];
    sprintf(buffer, "%02d'%02d\"%03d", minute, seconds, milliseconds);

    Stage::m_lapTimeStr = std::string(buffer);
    Stage::m_checkpointTimer = 3.0f;

    return;
}

//---------------------------------------------------
// Boost Particle
//---------------------------------------------------
HOOK(void, __fastcall, Stage_MsgGetHudPosition, 0x1096790, void* This, void* Edx, MsgGetHudPosition* message)
{
    if (message->m_type == 0)
    {
        Eigen::Vector3f sonicPosition;
        Eigen::Quaternionf sonicRotation;
        if (Common::GetPlayerTransform(sonicPosition, sonicRotation))
        {
            sonicPosition += sonicRotation * (Eigen::Vector3f::UnitY() * 0.5f); // half Sonic's height
            message->m_position = sonicPosition;
            return;
        }
    }

    originalStage_MsgGetHudPosition(This, Edx, message);
}

//---------------------------------------------------
// Custom HUD
//---------------------------------------------------
bool Stage::m_scoreEnabled = false;
float Stage::m_missionMaxTime = 0.0f;
boost::shared_ptr<Sonic::CGameObjectCSD> m_spPlayScreen;
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectPlayScreen;
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

boost::shared_ptr<Sonic::CGameObjectCSD> m_spCountdown;
Chao::CSD::RCPtr<Chao::CSD::CProject> m_projectCountdown;
Chao::CSD::RCPtr<Chao::CSD::CScene> m_sceneCountdown;


void Stage::CreateScreen(Sonic::CGameObject* pParentGameObject)
{
    if (m_projectPlayScreen && !m_spPlayScreen)
    {
        m_spPlayScreen = boost::make_shared<Sonic::CGameObjectCSD>(m_projectPlayScreen, 0.5f, "HUD", false);
        pParentGameObject->m_pMember->m_pGameDocument->AddGameObject(m_spPlayScreen, "main", pParentGameObject);
    }

    if (m_projectCountdown && !m_spCountdown)
    {
        m_spCountdown = boost::make_shared<Sonic::CGameObjectCSD>(m_projectCountdown, 0.5f, "HUD", false);
        pParentGameObject->m_pMember->m_pGameDocument->AddGameObject(m_spCountdown, "main", pParentGameObject);
    }
}

void Stage::KillScreen()
{
    if (m_spPlayScreen)
    {
        m_spPlayScreen->SendMessage(m_spPlayScreen->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
        m_spPlayScreen = nullptr;
    }

    if (m_spCountdown)
    {
        m_spCountdown->SendMessage(m_spCountdown->m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
        m_spCountdown = nullptr;
    }
}

void Stage::ToggleScreen(const bool visible, Sonic::CGameObject* pParentGameObject)
{
    if (visible)
    {
        CreateScreen(pParentGameObject);
    }
    else
    {
        KillScreen();
    }
}

void __fastcall Stage::CHudSonicStageRemoveCallback(Sonic::CGameObject* This, void*, Sonic::CGameDocument* pGameDocument)
{
    KillScreen();

    Chao::CSD::CProject::DestroyScene(m_projectPlayScreen.Get(), m_sceneLifeCount);
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

HOOK(void, __fastcall, Stage_CHudSonicStageInit, 0x109A8D0, Sonic::CGameObject* This)
{
    originalStage_CHudSonicStageInit(This);
    Stage::CHudSonicStageRemoveCallback(This, nullptr, nullptr);

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

    if (Stage::m_scoreEnabled) // score
    {
        m_sceneScore = m_projectPlayScreen->CreateScene("score");
        m_sceneScore->m_MotionSpeed = 0.0f;
    }

    // Mask to prevent crash when game tries accessing the elements we disabled later on
    flags &= ~(0x1 | 0x2 | 0x4 | 0x200 | 0x800);

    Stage::CreateScreen(This);
}

void __declspec(naked) Stage_GetScoreEnabled()
{
    static uint32_t returnAddress = 0x109C254;
    __asm
    {
        mov		Stage::m_scoreEnabled, 1
        jmp     [returnAddress]
    }
}

void Stage_GetTime(Sonic::CGameDocument* pGameDocument, size_t* minutes, size_t* seconds, size_t* milliseconds)
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

HOOK(void, __fastcall, Stage_CHudSonicStageUpdate, 0x1098A50, Sonic::CGameObject* This, void* Edx, const hh::fnd::SUpdateInfo& in_rUpdateInfo)
{
    originalStage_CHudSonicStageUpdate(This, Edx, in_rUpdateInfo);

    // Always clamp boost to 100
    *Common::GetPlayerBoost() = min(*Common::GetPlayerBoost(), 100.0f);

    Stage::ToggleScreen(*(bool*)0x1A430D8, This); // ms_IsRenderGameMainHud

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
        Stage_GetTime(**This->m_pMember->m_pGameDocument, &minutes, &seconds, &milliseconds);

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
        const float remainingTime = max(0, Stage::m_missionMaxTime - elapsedTime);

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

HOOK(void, __fastcall, Stage_MsgSetPinballHud, 0x1095D40, Sonic::CGameObject* This, void* Edx, MsgSetPinballHud const* message)
{
    // Update score
    if ((message->m_flag & 1) && Stage::m_scoreEnabled)
    {
        char text[16];
        sprintf(text, "%d", message->m_score);
        m_sceneScore->GetNode("score_text")->SetText(text);
    }

    originalStage_MsgSetPinballHud(This, Edx, message);
}

HOOK(void, __fastcall, Stage_MsgNotifySonicHud, 0x1097400, Sonic::CGameObject* This, void* Edx, void* message)
{
    // Ring collect animation
    if (m_sceneRingIcon && m_sceneRingIcon->m_MotionDisableFlag)
    {
        m_sceneRingIcon->SetMotionTime(0.0f);
        m_sceneRingIcon->m_MotionDisableFlag = 0;
        m_sceneRingIcon->Update(0.0f);
    }

    originalStage_MsgNotifySonicHud(This, Edx, message);
}

HOOK(void, __fastcall, Stage_MsgGetMissionLimitTime, 0xD0F0E0, Sonic::CGameObject* This, void* Edx, hh::fnd::Message& in_rMsg)
{
    originalStage_MsgGetMissionLimitTime(This, Edx, in_rMsg);
    Stage::m_missionMaxTime = *(float*)((char*)&in_rMsg + 16);
}

void Stage::applyPatches()
{
    // Install hooks to improve fading transitions.
    INSTALL_HOOK(MsgFadeInFxp);
    INSTALL_HOOK(MsgFadeOutFxp);
    INSTALL_HOOK(MsgFadeInMtfx);
    INSTALL_HOOK(MsgFadeOutMtfx);
    INSTALL_HOOK(Stage_CGameplayFlowStage_CStateBegin);
    INSTALL_HOOK(Stage_CGameplayFlowStage_CStateTitle);

    // Disable loading sfx
    WRITE_MEMORY(0x44A2E8, int, -1);
    WRITE_MEMORY(0x44A4F5, int, -1);

    // Use different xncp for READY GO
    WRITE_STRING(0x168F1EC, "ui_gp_signul");

    // Prevent timer getting reset twice after fade in is completed
    WRITE_MEMORY(0xCFDD8F, uint8_t, 0xEB);

    // Use single digit for life counter.
    WRITE_STRING(0x168E8C4, "%d");

    // Calculate correct 3 digit milliseconds for stage time
    WRITE_MEMORY(0x10B37BF, double*, (double*)0x1703970); // *100 -> *1000
    WRITE_MEMORY(0x10B37F7, uint32_t, 9162597); // 9162597/2^39 ~= 1/60000
    WRITE_MEMORY(0x10B3830, int32_t, -60000); // sec = total_sec + min * (-60000) = total_sec % 60000
    WRITE_MEMORY(0x10B3837, uint32_t, 137438954); // 137438954/2^37 ~= 1/1000
    WRITE_JUMP(0x10B3842, Stage_GetCurrentMillisecond);
    WRITE_MEMORY(0x10B3823, uint32_t, 999); // maximum milliseconds
    WRITE_MEMORY(0x1098D75, uint32_t, 0x168E8E0); // %03d

    // Change mission countdown time to have MM:SS.xxx format
    WRITE_JUMP(0x124F08D, Stage_GetMissionCurrentTime);

    // Set mission count down have 3 digit millisecond
    WRITE_MEMORY(0x124F0B1, double*, (double*)0x1703970); // *100 -> *1000
    WRITE_MEMORY(0x124F0BF, uint32_t, 0x168E8E0); // %03d

    // Display a random 3rd digit for next rank time in mission
    WRITE_STRING(0x1693474, "%02d:%02d.%03d");
    WRITE_JUMP(0x10B16CB, Stage_GetMissionNextRankMaxTime);
    WRITE_JUMP(0x10B16F2, Stage_GetMissionNextRankRandomDigit);

    // Other instances using sub_10B37B0
    WRITE_STRING(0x1689274, "--:--.---");
    WRITE_STRING(0x1689280, "%02d:%02d.%03d");
    WRITE_STRING(0x1689290, "--:--.---");
    WRITE_STRING(0x168929C, "%02d:%02d.%03d");
    WRITE_STRING(0x168CD0C, "99:59.999");
    WRITE_STRING(0x168CD18, "%02d:%02d.%03d");
    WRITE_STRING(0x16941C0, "%02d:%02d.%03d");
    WRITE_MEMORY(0x117BF38, uint32_t, 0x168E8E0); // %03d 
    WRITE_MEMORY(0x11CCCC2, uint32_t, 0x168E8E0); // %03d

    // Set laptime always display current time
    WRITE_MEMORY(0x1032FD1, uint8_t, 0xEB);
    WRITE_MEMORY(0x1033040, uint8_t, 0xEB);
    WRITE_NOP(0x10330A0, 6);
    WRITE_JUMP(0x103312C, (void*)0x103313E);
    WRITE_MEMORY(0x10978E4, double*, (double*)0x1703970); // *100 -> *1000
    INSTALL_HOOK(Stage_MsgNotifyLapTimeHud);

    // Make boost particles goes to Sonic
    INSTALL_HOOK(Stage_MsgGetHudPosition);

    // Remove Life + 1 UI
    WRITE_JUMP(0xE75565, (void*)0xE75630);
    WRITE_NOP(0xE75656, 9);

    // Force 200% and 300% boost challenge to 100%, and allow rings/enemies to refill boost
    WRITE_MEMORY(0x1104F29, uint32_t, 4);
    WRITE_MEMORY(0x1105000, uint32_t, 4);
    WRITE_MEMORY(0x1104DDA, uint32_t, 4);
    WRITE_NOP(0x1125255, 6);

    //---------------------------------------------------
    // Custom HUD
    //---------------------------------------------------
    INSTALL_HOOK(Stage_CHudSonicStageInit);
    INSTALL_HOOK(Stage_CHudSonicStageUpdate);
    INSTALL_HOOK(Stage_MsgSetPinballHud);
    INSTALL_HOOK(Stage_MsgNotifySonicHud);
    INSTALL_HOOK(Stage_MsgGetMissionLimitTime);
    WRITE_MEMORY(0x16A467C, void*, CHudSonicStageRemoveCallback);

    // Disable original HUD
    WRITE_JUMP(0x109B1AA, (void*)0x109B485); // life init
    WRITE_JUMP(0x109B496, (void*)0x109B5A1); // time init
    WRITE_JUMP(0x109B5B3, (void*)0x109B69B); // ring init
    WRITE_JUMP(0x109B8FB, (void*)0x109BA9D); // boost gauge init
    WRITE_JUMP(0x109BC8E, (void*)0x109BDF2); // boost button init
    WRITE_JUMP(0x109C1DC, Stage_GetScoreEnabled); // score init
    //WRITE_JUMP(0x109BEF6, (void*)0x109C05A); // countdown init
}

void Stage::draw()
{
    // At loading screen, clear all
    if (Common::IsAtLoadingScreen())
    {
        clearDraw();
        return;
    }

    if (m_checkpointTimer > 0.0f)
    {
        static bool visible = true;
        ImGui::Begin("Checkpoint", &visible, UIContext::m_hudFlags);
        {
            ImVec2 size = ImGui::CalcTextSize(m_lapTimeStr.c_str());
            ImGui::Text(m_lapTimeStr.c_str());
            ImGui::SetWindowFocus();
            ImGui::SetWindowPos(ImVec2((float)*BACKBUFFER_WIDTH * 0.5f - size.x * 0.5f, (float)*BACKBUFFER_HEIGHT * 0.882f - size.y * 0.5f));
        }
        ImGui::End();

        m_checkpointTimer -= Application::getHudDeltaTime();
    }
}

void Stage::clearDraw()
{
    m_checkpointTimer = 0.0f;
}
