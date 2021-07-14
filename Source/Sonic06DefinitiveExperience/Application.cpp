#include "Application.h"

float m_deltaTime = 0.0f;

HOOK(void*, __fastcall, UpdateApplication, 0xE7BED0, void* This, void* Edx, float elapsedTime, uint8_t a3)
{
    m_deltaTime = elapsedTime;
    return originalUpdateApplication(This, Edx, elapsedTime, a3);
}

void Application::applyPatches()
{
    INSTALL_HOOK(UpdateApplication);
}

float Application::getDeltaTime()
{ 
	return m_deltaTime;
}
