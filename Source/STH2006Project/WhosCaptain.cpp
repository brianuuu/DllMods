#include "WhosCaptain.h"
#include "SubtitleUI.h"
#include "MissionManager.h"

void* WhosCaptain::m_pCaptain = nullptr;
WhosCaptain::CaptainState WhosCaptain::m_state = WhosCaptain::CaptainState::CS_Idle;
HOOK(int, __fastcall, WhosCaptain_MsgRestartStage, 0xE76810, uint32_t* This, void* Edx, void* message)
{
	WhosCaptain::m_pCaptain = nullptr;
	WhosCaptain::m_state = WhosCaptain::CaptainState::CS_Idle;
	return originalWhosCaptain_MsgRestartStage(This, Edx, message);
}

HOOK(void, __fastcall, WhosCaptain_MsgNotifyObjectEvent, 0xEA4F50, Sonic::CGameObject* This, void* Edx, Sonic::Message::MsgNotifyObjectEvent& message)
{
	if (Common::GetCurrentStageID() == (SMT_ghz200 | SMT_Mission5))
	{
		if (message.m_Event == 402)
		{
			WhosCaptain::m_pCaptain = This;
			printf("[WhosCaptain] Captain added with address 0x%08x\n", (uint32_t)This);
			return;
		}
	}

	originalWhosCaptain_MsgNotifyObjectEvent(This, Edx, message);
}

void WhosCaptain::applyPatches()
{
	SubtitleUI::addDialogAcceptCallback(&callbackCaptainAccept);
	SubtitleUI::addDialogFinishCallback(&callbackDialogFinish);

	INSTALL_HOOK(WhosCaptain_MsgRestartStage);
	INSTALL_HOOK(WhosCaptain_MsgNotifyObjectEvent);
}

void WhosCaptain::callbackCaptainAccept(void* pObject, uint32_t dialogID)
{
	// Accepted captain's dialogue
	if (m_pCaptain && m_pCaptain == pObject)
	{
		Common::fEventTrigger(m_pCaptain, 10);
		m_state = CaptainState::CS_Accept;
	}
}

void WhosCaptain::callbackDialogFinish(void* pObject, uint32_t dialogID)
{
	if (!m_pCaptain) return;

	switch (m_state)
	{
	case CaptainState::CS_Accept:
		m_state = CaptainState::CS_FindCaptain;
		break;
	case CaptainState::CS_FindCaptain:
		if (m_pCaptain == pObject)
		{
			Common::fEventTrigger(m_pCaptain, 11);
			m_state = CaptainState::CS_Success;
		}
		else
		{
			MissionManager::setMissionFailed();
			m_state = CaptainState::CS_Fail;
		}
		break;
	}
}
