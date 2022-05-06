#include "Application.h"

float m_deltaTime = 0.0f;

HOOK(void*, __fastcall, UpdateApplication, 0xE7BED0, void* This, void* Edx, float elapsedTime, uint8_t a3)
{
    m_deltaTime = elapsedTime;
    return originalUpdateApplication(This, Edx, elapsedTime, a3);
}

Sonic::EKeyState Application::keyHoldState;
Sonic::EKeyState Application::keyReleasedState;
HOOK(void, __fastcall, CSonicUpdate, 0xE6BF20, void* This, void* Edx, float* dt)
{
    // Key release doesn't work for keyboard, check it here
    Sonic::SPadState const* padState = &Sonic::CInputState::GetInstance()->GetPadState();

    // Get released keys
    Application::keyReleasedState = Sonic::EKeyState((Application::keyHoldState ^ padState->DownState) & Application::keyHoldState);

    // Remember the current state for next frame to check
    Application::keyHoldState = padState->DownState;

    originalCSonicUpdate(This, Edx, dt);
}

void Application::applyPatches()
{
    INSTALL_HOOK(UpdateApplication);
    INSTALL_HOOK(CSonicUpdate);
}

float Application::getDeltaTime()
{ 
	return m_deltaTime;
}

bool Application::getKeyIsReleased(const Sonic::EKeyState key)
{
    return (keyReleasedState & key) == key;
}
