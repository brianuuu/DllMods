#include "NavigationSound.h"
#include "Configuration.h"

float const c_singleButtonDelay = 2.0f;
float NavigationSound::m_singleButtonTimer = 0.0f;

HOOK(void, __fastcall, NavigationSound_CSonicUpdate, 0xE6BF20, void* This, void* Edx, float* dt)
{
    NavigationSound::m_singleButtonTimer = max(0.0f, NavigationSound::m_singleButtonTimer - *dt);
    originalNavigationSound_CSonicUpdate(This, Edx, dt);
}

HOOK(void, __fastcall, NavigationSound_CSonicStateGrindJumpSideBegin, 0x124A1E0, hh::fnd::CStateMachineBase::CStateBase* This)
{
    originalNavigationSound_CSonicStateGrindJumpSideBegin(This);

    // send MsgEndQuickStepSign
    uint32_t context = (uint32_t)This->GetContextBase();
    uint32_t actorID = *(uint32_t*)(context + 4384);
    Hedgehog::Universe::CMessageActor* actor = (Hedgehog::Universe::CMessageActor*)(*(uint32_t*)(context + 272) + 40);
    bool isRight = *(bool*)((uint32_t)This + 0x68);
    actor->SendMessage(actorID, boost::make_shared<Sonic::Message::MsgEndQuickStepSign>(true, isRight));
}

void PlayNavigationHintSound()
{
    if (!Configuration::m_enableNavigationSound)
    {
        return;
    }

    static SharedPtrTypeless soundHandle;
    Common::SonicContextPlaySound(soundHandle, Configuration::m_navigationSoundType == 0 ? 3000812983 : 3000812984, 0);
}

HOOK(void, __fastcall, NavigationSound_MsgStartBoostSign, 0x528740, void* This, void* Edx, void* message)
{
    PlayNavigationHintSound();
    originalNavigationSound_MsgStartBoostSign(This, Edx, message);
}

HOOK(void, __fastcall, NavigationSound_MsgStartQuickStepSign, 0x528870, void* This, void* Edx, int* message)
{
    // QS: a2[4] = 0, a2[8] = 0, a2[12] = type (0,1,2)
    PlayNavigationHintSound();
    originalNavigationSound_MsgStartQuickStepSign(This, Edx, message);
}

HOOK(void, __fastcall, NavigationSound_MsgStartCommonButtonSign, 0x5289A0, void* This, void* Edx, int* message)
{
    // Single Button: buttonType = ? buttonType2 = -1
    // Stick Move: buttonType = -1, buttonType2 = 10, directionType = [2-0] flipped to quick step

    int buttonType = message[4];
    int buttonType2 = message[5];
    int directionType = message[6];

    // Disable Y button prompt?
    if (buttonType == 3 && !Configuration::m_enableLightdashPrompt)
    {
        return;
    }

    if (buttonType2 == -1) // Single button
    {
        if (NavigationSound::m_singleButtonTimer <= 0.0f)
        {
            PlayNavigationHintSound();
            NavigationSound::m_singleButtonTimer = c_singleButtonDelay;
        }
    }
    else
    {
        PlayNavigationHintSound();
        NavigationSound::m_singleButtonTimer = 0.0f;
    }

    originalNavigationSound_MsgStartCommonButtonSign(This, Edx, message);
}

HOOK(void, __fastcall, NavigationSound_MsgStartDirectionSign, 0x528B20, void* This, void* Edx, void* message)
{
    PlayNavigationHintSound();
    originalNavigationSound_MsgStartDirectionSign(This, Edx, message);
}

HOOK(void, __fastcall, NavigationSound_MsgStartSlidingSign, 0x528C60, void* This, void* Edx, void* message)
{
    PlayNavigationHintSound();
    originalNavigationSound_MsgStartSlidingSign(This, Edx, message);
}

HOOK(void, __fastcall, NavigationSound_MsgStartLeftRightSign, 0x528D90, void* This, void* Edx, void* message)
{
    PlayNavigationHintSound();
    originalNavigationSound_MsgStartLeftRightSign(This, Edx, message);
}

void NavigationSound::applyPatches()
{
    INSTALL_HOOK(NavigationSound_CSonicUpdate);

    if (Configuration::m_enableBumperRailSwitch)
    {
        // send MsgEndQuickStepSign
        INSTALL_HOOK(NavigationSound_CSonicStateGrindJumpSideBegin);
    }

    INSTALL_HOOK(NavigationSound_MsgStartBoostSign);
    INSTALL_HOOK(NavigationSound_MsgStartQuickStepSign);
    INSTALL_HOOK(NavigationSound_MsgStartCommonButtonSign);
    INSTALL_HOOK(NavigationSound_MsgStartDirectionSign);
    INSTALL_HOOK(NavigationSound_MsgStartSlidingSign);
    INSTALL_HOOK(NavigationSound_MsgStartLeftRightSign);
}
