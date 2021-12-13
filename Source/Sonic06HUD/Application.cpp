#include "Application.h"

std::string Application::m_modDir;
float Application::m_deltaTime = 0.0f;
float Application::m_hudDeltaTime = 0.0f;

HOOK(void*, __fastcall, UpdateApplication, 0xE7BED0, void* This, void* Edx, float elapsedTime, uint8_t a3)
{
    Application::setDeltaTime(elapsedTime);
    return originalUpdateApplication(This, Edx, elapsedTime, a3);
}

HOOK(void, __fastcall, CHudSonicStageUpdate, 0x1098A50, void* This, void* Edx, float* dt)
{
    Application::setHudDeltaTime(*dt);
    originalCHudSonicStageUpdate(This, Edx, dt);
}

void Application::applyPatches()
{
    INSTALL_HOOK(UpdateApplication);
    INSTALL_HOOK(CHudSonicStageUpdate);
}
