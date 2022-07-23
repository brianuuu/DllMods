#include "WhosCaptain.h"

void* WhosCaptain::m_pCaptain = nullptr;
HOOK(int, __fastcall, WhosCaptain_MsgRestartStage, 0xE76810, uint32_t* This, void* Edx, void* message)
{
	WhosCaptain::m_pCaptain = nullptr;
	return originalWhosCaptain_MsgRestartStage(This, Edx, message);
}

void WhosCaptain::applyPatches()
{
	INSTALL_HOOK(WhosCaptain_MsgRestartStage);
}

void WhosCaptain::callbackCaptainAccept()
{
	if (WhosCaptain::m_pCaptain)
	{
		Common::fEventTrigger(WhosCaptain::m_pCaptain, 10);
	}
}
