#include "SonicMan.h"

#include "Managers/MissionManager.h"
#include "Managers/PathManager.h"
#include "System/Application.h"

class CObjSonicMan : public Sonic::CGameObject
{
	enum SonicManState
	{
		SM_Start,
		SM_Run,
		SM_Won,
		SM_Lost,
	};

	PathDataCollection m_path;
	PathFollowData m_followData;

	Sonic::CGameObject* m_pObject;
	SonicManState m_state;
	float m_startDelay;
	bool m_lost;
	bool m_won;

	std::mutex m_mutex;

public:
	CObjSonicMan(Sonic::CGameObject* pObject)
	{
		if (!PathManager::parsePathXml(m_path, false, (Application::getModDirString() + "Assets\\Stage\\SonicMan.path.xml").c_str()) == tinyxml2::XML_SUCCESS)
		{
			MessageBox(NULL, L"Failed to parse SonicMan.path.xml", NULL, MB_ICONERROR);
		}

		if (!m_path.empty())
		{
			m_followData.m_yawOnly = true;
			m_followData.m_speed = 15.0f;
			m_followData.m_loop = false;
			m_followData.m_pPathData = &m_path[0];

			m_followData.m_rotation = Eigen::Quaternionf::Identity();
			m_followData.m_position = m_path[0].m_knots[0].m_point;
		}

		m_pObject = pObject;
		m_state = SonicManState::SM_Start;
		m_startDelay = 0.5f;

		m_lost = false;
		m_won = false;

		printf("[SonicMan] Sonic Man address 0x%08x\n", (uint32_t)pObject);
	}

	void AddCallback
	(
		const Hedgehog::Base::THolder<Sonic::CWorld>& worldHolder,
		Sonic::CGameDocument* pGameDocument,
		const boost::shared_ptr<Hedgehog::Database::CDatabase>& spDatabase
	) override
	{
		Sonic::CApplicationDocument::GetInstance()->AddMessageActor("GameObject", this);
		pGameDocument->AddUpdateUnit("0", this);
	}

	bool ProcessMessage
	(
		Hedgehog::Universe::Message& message,
		bool flag
	) override
	{
		if (flag)
		{
			if (std::strstr(message.GetType(), "MsgRestartStage") != nullptr)
			{
				Kill();
				return true;
			}
		}

		return Sonic::CGameObject::ProcessMessage(message, flag);
	}

	void UpdateParallel
	(
		const Hedgehog::Universe::SUpdateInfo& updateInfo
	) override
	{
		// Not object physics? Kill
		if (*(uint32_t*)m_pObject != 0x16CF524)
		{
			Kill();
			return;
		}

		std::lock_guard<std::mutex> guard(m_mutex);

		float* frameTime = getFrameTime();
		float frame = *frameTime * 60.0f;
		switch (m_state)
		{
		case SonicManState::SM_Start:
		{
			if (frame >= 244.0f)
			{
				*frameTime = 0.0f;
			}

			m_startDelay -= updateInfo.DeltaTime;
			if (m_startDelay <= 0.0f)
			{
				m_state = SonicManState::SM_Run;
				*frameTime = 254.0f / 60.0f;
			}
			break;
		}
		case SonicManState::SM_Run:
		{
			if (frame >= 297.0f)
			{
				*frameTime = 265.0f / 60.0f;
			}

			if (!m_lost && !m_won && m_followData.m_position.z() <= -159.194)
			{
				m_won = true;
				Common::fEventTrigger(m_pObject, 10);
				MissionManager::setMissionFailed();
			}

			if (m_followData.m_finished)
			{
				m_state = m_won ? SonicManState::SM_Won : SonicManState::SM_Lost;
				*frameTime = (m_won ? 329.0f : 366.0f) / 60.0f;
			}

			break;
		}
		case SonicManState::SM_Won:
		{
			if (frame >= 341.0f || (frame >= 244.0f && frame < 320.0f))
			{
				*frameTime = 0.0f;
			}
			break;
		}
		case SonicManState::SM_Lost:
		{
			if (frame >= 578.0f)
			{
				*frameTime = 377.0f / 60.0f;
			}
			break;
		}
		}

		if (!m_followData.m_finished && m_state != SonicManState::SM_Start)
		{
			PathManager::followAdvance(m_followData, updateInfo.DeltaTime);
			SendMessage(m_pObject->m_ActorID, boost::make_shared<Sonic::Message::MsgSetPosition>(m_followData.m_position));
			SendMessage(m_pObject->m_ActorID, boost::make_shared<Sonic::Message::MsgSetRotation>(m_followData.m_rotation));
		}
	}

	float* getFrameTime()
	{
		if (m_pObject)
		{
			uint32_t frameTimeAddress = Common::GetMultiLevelAddress((uint32_t)m_pObject + 0x110, { 0x0, 0x110, 0x4C, 0x8, 0x8 });
			if (frameTimeAddress)
			{
				return (float*)frameTimeAddress;
			}
		}

		return nullptr;
	}

	void SetLost()
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		m_lost = true;
	}

	void Kill()
	{
		printf("[SonicMan] Killed\n");
		SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
	}
};

std::pair<Sonic::CGameObject*, boost::shared_ptr<CObjSonicMan>> m_spSonicMan;
HOOK(void, __fastcall, SonicMan_MsgNotifyObjectEvent, 0xEA4F50, Sonic::CGameObject* This, void* Edx, Sonic::Message::MsgNotifyObjectEvent& message)
{
	if (Common::GetCurrentStageID() == (SMT_ghz100 | SMT_Mission3))
	{
		if (message.m_Event == 403)
		{
			m_spSonicMan.first = This;
			m_spSonicMan.second = boost::make_shared<CObjSonicMan>(This);
			This->m_pMember->m_pGameDocument->AddGameObject(m_spSonicMan.second);
			return;
		}
		else if (message.m_Event == 404 && m_spSonicMan.first == This)
		{
			// Sonic Man lost
			m_spSonicMan.second->SetLost();
			return;
		}
	}

	originalSonicMan_MsgNotifyObjectEvent(This, Edx, message);
}

HOOK(int, __fastcall, SonicMan_CGameObject3DDestruction, 0xD5D790, Sonic::CGameObject* This)
{
	if (m_spSonicMan.first == This)
	{
		m_spSonicMan.second->Kill();

		m_spSonicMan.first = nullptr;
		m_spSonicMan.second = nullptr;
	}

	return originalSonicMan_CGameObject3DDestruction(This);
}

void SonicMan::applyPatches()
{
	INSTALL_HOOK(SonicMan_MsgNotifyObjectEvent);
	INSTALL_HOOK(SonicMan_CGameObject3DDestruction);
}
