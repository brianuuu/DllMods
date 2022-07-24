#include "SoleannaBoys.h"

PathDataCollection SoleannaBoys::m_paths;
std::deque<void*> SoleannaBoys::m_omochaos;

HOOK(int, __fastcall, SoleannaBoys_OmochaoMsgNotifyObjectEvent, 0x114FB60, void* This, void* Edx, uint32_t a2)
{
	uint32_t* pEvent = (uint32_t*)(a2 + 16);

	if (Common::GetCurrentStageID() == (SMT_ghz200 | SMT_Mission4))
	{
		if (*pEvent >= 421 && *pEvent <= 425)
		{
			if (SoleannaBoys::m_omochaos.size() != 5)
			{
				SoleannaBoys::m_omochaos.resize(5);
			}

			int index = *pEvent - 421;
			SoleannaBoys::m_omochaos[index] = This;
			printf("[SoleannaBoys] Omochao added with address 0x%08x\n", (uint32_t)This);

			return 0;
		}
	}

	return originalSoleannaBoys_OmochaoMsgNotifyObjectEvent(This, Edx, a2);
}

class CObjSoleannaBoy : public Sonic::CGameObject
{
	void* m_pObject;
	bool m_stopped;
	PathFollowData m_followData;

public:
	CObjSoleannaBoy(void* pObject, PathFollowData followData)
	{
		m_pObject = pObject;
		m_stopped = false;
		m_followData = followData;
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

		Eigen::Vector3f sonicPosition;
		Eigen::Quaternionf sonicRotation;
		if (!Common::GetPlayerTransform(sonicPosition, sonicRotation))
		{
			return;
		}

		if (!m_stopped)
		{
			float constexpr hitDist = 0.5f;
			if ((m_followData.m_position - sonicPosition).squaredNorm() <= (hitDist * hitDist))
			{
				m_stopped = true;

				static SharedPtrTypeless soundHandle;
				Common::PlaySoundStatic(soundHandle, 845019953);
				Common::fEventTrigger(m_pObject, 10);

				printf("[SoleannaBoys] Boy captured, %d remaining\n", SoleannaBoys::m_omochaos.size());
				if (!SoleannaBoys::m_omochaos.empty())
				{
					struct MsgNotifyObjectEvent
					{
						INSERT_PADDING(0x10);
						uint32_t m_event;
						bool m_unknown;
					};
					alignas(16) MsgNotifyObjectEvent msgNotifyObjectEvent {};
					msgNotifyObjectEvent.m_event = 6;
					msgNotifyObjectEvent.m_unknown = false;
					originalSoleannaBoys_OmochaoMsgNotifyObjectEvent(SoleannaBoys::m_omochaos.front(), nullptr, (uint32_t)&msgNotifyObjectEvent);
					SoleannaBoys::m_omochaos.pop_front();
				}
			}
			else
			{
				PathManager::followAdvance(m_followData, updateInfo.DeltaTime);
				Common::ApplyObjectPhysicsPosition(m_pObject, m_followData.m_position);
				Common::ApplyObjectPhysicsRotation(m_pObject, m_followData.m_rotation);
			}
		}

		// Handle animation
		float* frameTime = (float*)Common::GetMultiLevelAddress((uint32_t)m_pObject + 0x110, { 0x0, 0x110, 0x4C, 0x8, 0x8 });
		float frame = *frameTime * 60.0f;
		if (m_stopped)
		{
			if (frame <= 36.0f || frame >= 163.0f)
			{
				*frameTime = 38.0f / 60.0f;
			}
		}
		else
		{
			if (frame > 35.0f)
			{
				*frameTime = 0.0f;
			}
		}
	}

	void Kill()
	{
		printf("[SoleannaBoys] Killed\n");
		SendMessage(m_ActorID, boost::make_shared<Sonic::Message::MsgKill>());
	}
};

HOOK(void, __fastcall, SoleannaBoys_MsgNotifyObjectEvent, 0xEA4F50, Sonic::CGameObject* This, void* Edx, uint32_t a2)
{
	uint32_t* pEvent = (uint32_t*)(a2 + 16);
	uint32_t* pObject = (uint32_t*)This;

	if (Common::GetCurrentStageID() == (SMT_ghz200 | SMT_Mission4))
	{
		if (*pEvent >= 411 && *pEvent <= 416)
		{
			if (SoleannaBoys::m_paths.empty())
			{
				if (!PathManager::parsePathXml(SoleannaBoys::m_paths, true, "Assets\\Stage\\SoleannaBoysChallenge.path.xml") == tinyxml2::XML_SUCCESS)
				{
					MessageBox(NULL, L"Failed to parse SoleannaBoysChallenge.path.xml", NULL, MB_ICONERROR);
				}
			}

			int index = *pEvent - 411;
			PathFollowData followData;
			followData.m_yawOnly = true;
			followData.m_speed = 4.0f;
			followData.m_pPathData = &SoleannaBoys::m_paths[index];
			This->m_pMember->m_pGameDocument->AddGameObject(boost::make_shared<CObjSoleannaBoy>(This, followData));

			printf("[SoleannaBoys] Boy added with address 0x%08x\n", (uint32_t)This);
			return;
		}
	}

	originalSoleannaBoys_MsgNotifyObjectEvent(This, Edx, a2);
}

HOOK(int, __fastcall, SoleannaBoys_MsgRestartStage, 0xE76810, uint32_t* This, void* Edx, void* message)
{
	SoleannaBoys::m_omochaos.clear();
	return originalSoleannaBoys_MsgRestartStage(This, Edx, message);
}

void SoleannaBoys::applyPatches()
{
	INSTALL_HOOK(SoleannaBoys_MsgNotifyObjectEvent);
	INSTALL_HOOK(SoleannaBoys_OmochaoMsgNotifyObjectEvent);
	INSTALL_HOOK(SoleannaBoys_MsgRestartStage);
}
